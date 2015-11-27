var OFFLINE_MODE = false;
var DISPLAY_FEWER_ROUTES = false;

// use to initialize favorites if we want
// PersistentFavoritesManager.setFavorite('P1', 'INBOUND', '8161', 'East Liberty Station Stop C', true)

var Dispatcher = {
  sendMenuSetupMessage : function(dataManager, msgType)
  {
    num_entries = DISPLAY_FEWER_ROUTES ? 6 : dataManager.savedData.titles.length;

    var dictionary = {
      "KEY_NUM_ENTRIES" : num_entries,
      "KEY_MSG_TYPE" : msgType
    }

    console.log("setup dict: " + JSON.stringify(dictionary));

    Pebble.sendAppMessage(dictionary,
      function(e) {
        // console.log("for " + msgType + "... setup msg sent to pebble successfully!");
        dataManager.handleRequest(false)
      },
      function(e) {
        console.log("Error sending # entries to pebble :(");
      });
  },
  sendNextItem : function(dataManager, displayRequestType)
  {
    var savedData = dataManager.savedData;

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
    dataManager.savedData = savedData;

    var cutoff_early = DISPLAY_FEWER_ROUTES && index > 5;
    var is_done = !nextTitle || cutoff_early;

    if (is_done)
    {
      nextTitle = "done";
      nextSubtitle = "done";
      console.log("DONE!!!");
    }

    var dictionary = {
      "KEY_ITEM_INDEX" : index,
      "KEY_MSG_TYPE" : displayRequestType
    };
    if (nextTitle != null)
    {
      dictionary["KEY_TITLES"] = nextTitle;
    }

    if (nextSubtitle != null)
    {
      dictionary["KEY_SUBTITLES"] = nextSubtitle
    }

    if (nextSelector != null)
    {
      dictionary["KEY_SELECTORS"] = nextSelector;
    }

    console.log("send next dict: " + JSON.stringify(dictionary));

    Pebble.sendAppMessage(dictionary,
      function(e) {
        // console.log(displayRequestType + " sent to pebble successfully!");
        if (!is_done)
        {
          dataManager.handleRequest(false);
        }
      },
      function(e) {
        console.log("Error sending " + displayRequestType + " to pebble :(");
      });
  },

  organizeAndSaveData : function(data, dataManager, extractDataFcn, sortDataFcn, extractTitleFcn, extractSubtitleFcn, extractSelectorFcn)
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

    dataManager.savedData = {
      titles : titles,
      subtitles : subtitles,
      selectors : selectors,
      index : 0
    };
  },
  sendRequest : function(dataManager, requestType, displayRequestType, requestData, extractDataFcn, sortDataFcn, extractTitleFcn, extractSubtitleFcn, extractSelectorFcn)
  {
    var url = URLUtils.constructURL(requestType, requestData);
    console.log('request URL: ' + url);
    URLUtils.sendRequest(url, function(responseText) {
      // TODO: remove!
      // responseText = '{"bustime-response": {"prd": [{"tmstmp": "20151126 22:02","typ": "A","stpnm": "Negley Station Stop C","stpid": "8162","vid": "3239","dstp": 27715,"rt": "P1","rtdd": "P1","rtdir": "INBOUND","des": "Downtown","prdtm": "20151126 22:24","tablockid": "P1  -165","tatripid": "65856","dly": false,"prdctdn": "21","zone": ""}]}}'
      if (!!responseText)
      {
        var data = JSON.parse(responseText);
        console.log('data: ' + JSON.stringify(data));
        Dispatcher.organizeAndSaveData(data, dataManager,
                                       extractDataFcn, sortDataFcn,
                                       extractTitleFcn, extractSubtitleFcn, extractSelectorFcn);

        Dispatcher.sendMenuSetupMessage(dataManager, displayRequestType);
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
      getRoutes.handleRequest(true)
    }
    else if (requestType == 'getdirections')
    {
      getDirections.handleRequest(should_init, route);
      // handleDirectionsRequest(should_init, route);
    }
    else if (requestType == 'getstops')
    {
      getStops.handleRequest(should_init, route, direction);
      // handleStopsRequest(should_init, route, direction);
    }
    else if (requestType == 'getpredictions')
    {
      if (extra == null)
      {
        console.log("~ handling predictions");
        getPredictions.handleRequest(should_init, route, direction, stopid, stopname);
        // handlePredictionsRequest(should_init, route, direction, stopid, stopname, 'getpredictions');
      }
      else
      {
        console.log('~ handling predictions_favorites');
        handlePredictionsRequest_Favorites(should_init, extra);
      }
    }
    else if (requestType == 'getfavorites')
    {
      getFavorites.handleRequest(should_init)
      // handleFavoritesRequest(should_init);
    }
    else if (requestType == 'setfavorite' ||
             requestType == 'setmorningcommute' ||
             requestType == 'seteveningcommute')
    {
      var shouldAdd = e.payload['106'] == 1;

      // we need to extract the data first
      if (extra != null)
      {
        var parsed = PersistentDataManagerUtils.parseStorageString(extra);
        route = parsed.route;
        direction = parsed.direction;
        stopid = parsed.stopid;
        stopname = parsed.stopname;
      }

      if (requestType == 'setfavorite')
      {
        PersistentFavoritesManager.setFavorite(route, direction, stopid, stopname, shouldAdd);
      }
      else if (requestType == 'setmorningcommute')
      {
        PersistentMorningCommuteManager.setMorningCommute(route, direction, stopid, stopname, shouldAdd);
      }
      else if (requestType == 'seteveningcommute')
      {
        PersistentEveningCommuteManager.setEveningCommute(route, direction, stopid, stopname, shouldAdd);
      }
    }
    else if (requestType == 'getnearbystops')
    {
      getNearbyStops.handleRequest(should_init);
      // handleNearbyStopsRequest(should_init);
    }
    else if (requestType == 'getnearbyroutes')
    {
      getNearbyRoutes.handleRequest(should_init, stopid);
      // handleNearbyRoutesRequest(should_init, stopid);
    }
    else if (requestType == 'getcommute')
    {
      console.log('getting commute...');
      getCommute.handleRequest(should_init);
    }
  });
