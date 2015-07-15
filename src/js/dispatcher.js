var OFFLINE_MODE = false;
var DISPLAY_FEWER_ROUTES = false;

// use to initialize favorites if we want
// PersistentFavoritesManager.setFavorite('P1', 'INBOUND', '8161', 'East Liberty Station Stop C', true)

var sendMenuSetupMessage = function(num_entries, msgType)
{
  // console.log('for ' + msgType + '... sending menu setup message w/ ' + num_entries + ' entries');

  var dictionary = {
    "KEY_NUM_ENTRIES" : num_entries,
    "KEY_MSG_TYPE" : msgType    
  }

  Pebble.sendAppMessage(dictionary,
    function(e) {
      // console.log("for " + msgType + "... # entries sent to pebble successfully!");
    },
    function(e) {
      console.log("Error sending # entries to pebble :(");
    });
}

var sendMenuEntryMessage = function(title, subtitle, selector, index, msgType)
{
  
  // console.log('sending for ' + msgType + 
  //             '... title: ' + title + 
  //             ', subtitle: ' + subtitle + 
  //             ', selector: ' + selector +
  //             ', idx: ' + index);

  var dictionary = {
    "KEY_ITEM_INDEX" : index,
    "KEY_MSG_TYPE" : msgType
  };
  if (title != null)
  {
    dictionary["KEY_TITLES"] = title;
  }

  if (subtitle != null)
  {
    dictionary["KEY_SUBTITLES"] = subtitle
  }

  if (selector != null)
  {
    dictionary["KEY_SELECTORS"] = selector;
  }

  Pebble.sendAppMessage(dictionary,
    function(e) {
      // console.log(msgType + " sent to pebble successfully!");
    },
    function(e) {
      console.log("Error sending " + msgType + " to pebble :(");
    }
    );
}

var Dispatcher = {
  sendNextItem : function(savedDataContainer, displayRequestType)
  {
    var savedData = savedDataContainer.savedData;

    var titles = savedData.titles;
    var subtitles = savedData.subtitles;
    var selectors = savedData.selectors;
    var index = savedData.index;

    var nextTitle = titles[index];
    var nextSubtitle = null;
    if (!!subtitles)
    {
      nextSubtitle = subtitles[index];
    }
    var nextSelector = selectors[index];

    savedData.index = index+1;
    savedDataContainer.savedData = savedData;

    var cutoff_early = DISPLAY_FEWER_ROUTES && index > 5;

    if (!nextTitle || cutoff_early) 
    {
      nextTitle = "done";
      nextSubtitle = "done";
    }

    sendMenuEntryMessage(nextTitle, nextSubtitle, nextSelector, index, displayRequestType);   
  },   

  organizeAndSaveData : function(data, savedDataContainer, extractDataFcn, sortDataFcn, extractTitleFcn, extractSubtitleFcn, extractSelectorFcn)   
  {
    var items = extractDataFcn(data);
    if (!items)
    {
      items = []
    }

    /* custom sort */
    if (!!sortDataFcn)
    {
      items.sort(sortDataFcn);
    }

    should_use_subtitles = true;

    var titles = [];
    var subtitles = [];
    var selectors = [];
    for (var i=0; i<items.length; i++)
    {
      var item = items[i];
      var title = extractTitleFcn(item);
      titles.push(title);

      var subtitle = extractSubtitleFcn(item);
      if (!subtitle)
      {
        should_use_subtitles = false;
        subtitles = null;
      }

      if (should_use_subtitles)
      {
        subtitles.push(subtitle);
      }

      var selector = extractSelectorFcn(item);
      selectors.push(selector);
    }

    savedDataContainer.savedData = {
      titles : titles,
      subtitles : subtitles,
      selectors : selectors,
      index : 0
    };
  },
  sendRequest : function(savedDataContainer, requestType, displayRequestType, requestData, 
                         extractDataFcn, sortDataFcn, extractTitleFcn, 
                         extractSubtitleFcn, extractSelectorFcn)
  {
    var url = URLUtils.constructURL(requestType, requestData);
    // console.log('url: ' + url);
    URLUtils.sendRequest(url, function(responseText) {
      if (!!responseText)
      {
        var data = JSON.parse(responseText);
        Dispatcher.organizeAndSaveData(data, savedDataContainer, 
                                       extractDataFcn, sortDataFcn,
                                       extractTitleFcn, extractSubtitleFcn, extractSelectorFcn);

        var titlesLength = DISPLAY_FEWER_ROUTES ? 6 : savedDataContainer.savedData.titles.length;
        sendMenuSetupMessage(titlesLength, displayRequestType);
      }
    });
  }
};

var SWITCH = true;

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {

    // TODO: remove this!
    if (SWITCH)
    {
      // console.log("calling getNearbyStops.get()");
      // getNearbyStops.get();

      // AllstopsManager.get(function(allstops) {
      //   // console.log(JSON.stringify(allstops));
      //   console.log(allstops);
      //   var length = 0;
      //   for (var stopid in allstops)
      //   {
      //     length++;
      //   }
      //   console.log(length);
      // });
      SWITCH = false;
    }

    var payload = e.payload
    // console.log("AppMessage received!");
    var requestType = payload['100'];
    // console.log('request type: ' + requestType);
    // console.log('payload: ' + JSON.stringify(payload));

    var route = payload['101'];
    var direction = payload['102'];
    var stopid = payload['103'];
    var stopname = payload['104'];
    var extra = payload['105'];
    var should_init = payload['106'] == 1;

    if (requestType == 'getroutes')
    {
      handleRoutesRequest(should_init);
    }
    else if (requestType == 'getdirections')
    {
      // handleRoutesRequest();
      handleDirectionsRequest(should_init, route);
    }
    else if (requestType == 'getstops')
    {
      handleStopsRequest(should_init, route, direction);
    }
    else if (requestType == 'getpredictions')
    {
      if (extra == null)
      {
        console.log("~ handling predictions");
        handlePredictionsRequest(should_init, route, direction, stopid, stopname, 'getpredictions');
      }
      else
      {
        console.log('~ handling predictions_favorites');
        handlePredictionsRequest_Favorites(should_init, extra);
      } 
    }
    else if (requestType == 'getfavorites')
    {
      handleFavoritesRequest(should_init);
    }
    else if (requestType == 'setfavorite')
    {
      var isfavorite = e.payload['106'] == 1;

      // we need to extra the data first
      if (extra != null)
      {
        var fav = PersistentFavoritesManager.parseStorageString(extra);
        route = fav.route;
        direction = fav.direction;
        stopid = fav.stopid;
        stopname = fav.stopname;
      }

      PersistentFavoritesManager.setFavorite(route, 
        direction, stopid, stopname, isfavorite);
    }
    else if (requestType == 'getnearbystops')
    {
      handleNearbyStopsRequest(should_init);
    }
    else if (requestType == 'getnearbyroutes')
    {
      handleNearbyRoutesRequest(should_init, stopid);
    }
  });
