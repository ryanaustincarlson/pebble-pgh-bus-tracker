var OFFLINE_MODE = false;
var DISPLAY_FEWER_ROUTES = false;

// use to initialize favorites if we want
// PersistentFavoritesManager.setFavorite('P1', 'INBOUND', '8161', 'East Liberty Station Stop C', true)

var Dispatcher = {
  appMessageSize : 500,

  // TODO: rename me!
  sendMenuSetupMessage : function(dataManager, msgType)
  {
    num_entries = DISPLAY_FEWER_ROUTES ? 6 : dataManager.savedData.titles.length;

    var savedData = dataManager.savedData;

    // each character is 1-byte so appMessageSize = NumberOfCharacters in a msg
    if (!savedData.msg)
    {
      var msgTitles = savedData.titles.join('|');
      var msgSubtitles = savedData.subtitles ? savedData.subtitles.join('|') : null;
      var msgSelectors = savedData.selectors.join('|');

      var msg = {};
      msg.titles = msgTitles;
      msg.subtitles = msgSubtitles;
      msg.selectors = msgSelectors;
      msg.totalLength = msgTitles.length +
                        (msgSubtitles ? msgSubtitles.length : 0) +
                        msgSelectors.length;

      // console.log('titles (' + msg.titles.length + ') ~> ' + msg.titles);
      // console.log('subtitles ' + msg.subtitles.length + ') ~> ' + msg.subtitles);
      // console.log('selectors ' + msg.selectors.length + ') ~> ' + msg.selectors);

      savedData.msg = msg;
      savedData.index = 0;
    }

    var numFields = 2 + (savedData.subtitles ? 1 : 0);
    var msgSizePerField = Math.floor(20 / numFields);
    var msgSize = Dispatcher.appMessageSize;
    var start = savedData.index;
    var end = start + msgSize;

    // titles will be empty string if start > titles.length
    var titles = savedData.msg.titles.substring(start, end);
    var subtitleStart = start - savedData.msg.titles.length;
    var subtitleEnd = end - savedData.msg.titles.length;
    var subtitles = '';
    if (savedData.subtitles)
    {
      subtitles = savedData.msg.subtitles.substring(subtitleStart, subtitleEnd);
    }
    var selectorStart = subtitleStart - (savedData.subtitles ? savedData.msg.subtitles.length : 0);
    var selectorEnd = subtitleEnd - (savedData.subtitles ? savedData.msg.subtitles.length : 0);
    var selectors = savedData.msg.selectors.substring(selectorStart, selectorEnd);

    var dictionary = {
      "KEY_MSG_TYPE": msgType
    }
    if (Math.max(titles.length, subtitles.length, selectors.length) == 0)
    {
      dictionary = {
        "KEY_DONE" : true,
        "KEY_NUM_ENTRIES" : num_entries
      };
    }
    else {
      dictionary = {};

      if (titles.length > 0)
      {
        dictionary["KEY_TITLES"] = titles;
      }

      if (subtitles.length > 0)
      {
        dictionary["KEY_SUBTITLES"] = subtitles;
      }

      if (selectors.length > 0)
      {
        dictionary["KEY_SELECTORS"] = selectors;
      }
    }

    // console.log('start: ' + start + ', end: ' + end);
    // console.log('subtitleStart: ' + subtitleStart + ', subtitleEnd: ' + subtitleEnd);
    // console.log('selectorStart: ' + selectorStart + ', selectorEnd: ' + selectorEnd);
    // console.log('titles: ' + titles + ', len: ' + titles.length);
    // console.log('subtitles: ' + subtitles + ', len: ' + subtitles.length);
    // console.log('selectors: ' + selectors + ', len: ' + selectors.length);
    // console.log('');

    //
    // console.log("setup dict: " + JSON.stringify(dictionary));
    //

    Pebble.sendAppMessage(dictionary,
      function(e) {
        if (!dictionary['KEY_DONE'])
        {
          savedData.index = end;
          dataManager.savedData = savedData;
          Dispatcher.sendMenuSetupMessage(dataManager, msgType);
        }

        // console.log("for " + msgType + "... setup msg sent to pebble successfully!");
        // dataManager.handleRequest(false)
      },
      function(e) {
        console.log("Error sending # entries to pebble");
      });
  },
  // sendNextItem : function(dataManager, displayRequestType)
  // {
  //   var savedData = dataManager.savedData;
  //
  //   var titles = savedData.titles;
  //   var subtitles = savedData.subtitles;
  //   var selectors = savedData.selectors;
  //   var index = savedData.index;
  //
  //   var titleString = titles.join('|');
  //   var subtitleString = subtitles.join('|');
  //   var selectorString = selectors.join('|');
  //
  //   var nextTitle = null;
  //   var nextSubtitle = null;
  //   var nextSelector = null;
  //
  //   // if (savedData.index == 0)
  //   // {
  //     nextTitle = titleString;
  //   // }
  //   // else if (savedData.index == 1)
  //   // {
  //     nextSubtitle = subtitleString;
  //   // }
  //   // else if (savedData.index == 2)
  //   // {
  //     nextSelector = selectorString;
  //   // }
  //
  //   var is_done = savedData.index == 1;
  //
  //   savedData.index = index+1;
  //   dataManager.savedData = savedData;
  //
  //   if (is_done)
  //   {
  //     nextTitle = "done";
  //     nextSubtitle = "done";
  //     console.log("DONE!!!");
  //   }
  //
  //   var dictionary = {
  //     "KEY_ITEM_INDEX" : index,
  //     "KEY_MSG_TYPE" : displayRequestType
  //   };
  //   if (nextTitle != null)
  //   {
  //     dictionary["KEY_TITLES"] = nextTitle;
  //   }
  //
  //   if (nextSubtitle != null)
  //   {
  //     dictionary["KEY_SUBTITLES"] = nextSubtitle
  //   }
  //
  //   if (nextSelector != null)
  //   {
  //     dictionary["KEY_SELECTORS"] = nextSelector;
  //   }
  //
  //   Pebble.sendAppMessage(dictionary,
  //     function(e) {
  //       if (!is_done)
  //       {
  //         dataManager.handleRequest(false);
  //       }
  //     },
  //     function(e) {
  //       console.log("Error sending " + displayRequestType + " to pebble :(");
  //     });
  // },
  sendError : function()
  {
    var dictionary = {
      'KEY_ERROR': true
    }
    Pebble.sendAppMessage(dictionary,
      function(e) {

      },
      function(e) {

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
      // FOR TESTING ONLY!
      // responseText = '{"bustime-response": {"prd": [{"tmstmp": "20151126 22:02","typ": "A","stpnm": "Negley Station Stop X","stpid": "8162","vid": "3239","dstp": 27715,"rt": "P1X","rtdd": "P1X","rtdir": "INBOUNDX","des": "DowntownX","prdtm": "20151126 22:24","tablockid": "P1  -165","tatripid": "65856","dly": false,"prdctdn": "25","zone": ""},{"tmstmp": "20151126 22:02","typ": "A","stpnm": "Negley Station Stop C","stpid": "8162","vid": "3239","dstp": 27715,"rt": "P1","rtdd": "P1","rtdir": "INBOUND","des": "Downtown","prdtm": "20151126 22:24","tablockid": "P1  -165","tatripid": "65856","dly": false,"prdctdn": "21","zone": ""}]}}'
      console.log(responseText);
      if (!!responseText)
      {
        try
        {
          var data = JSON.parse(responseText);
          // console.log('data: ' + JSON.stringify(data));
          Dispatcher.organizeAndSaveData(data, dataManager,
                                         extractDataFcn, sortDataFcn,
                                         extractTitleFcn, extractSubtitleFcn, extractSelectorFcn);

          Dispatcher.sendMenuSetupMessage(dataManager, displayRequestType);
        }
        catch(err)
        {
          Dispatcher.sendError();
        }
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

    var app_message_size = payload['150'];
    if (app_message_size)
    {
      Dispatcher.appMessageSize = app_message_size;
    }

    console.log('app msg size: ' + Dispatcher.appMessageSize);

    // console.log("AppMessage received!");
    var requestType = payload['100'];
    // console.log('request type: ' + requestType);
    // console.log('payload: ' + JSON.stringify(payload));

    var route = payload['101'];
    var direction = payload['102'];
    var stopid = payload['103'];
    var stopname = payload['104'];
    var extra = payload['105'];

    if (requestType == 'getroutes')
    {
      getRoutes.handleRequest(true)
    }
    else if (requestType == 'getdirections')
    {
      getDirections.handleRequest(route);
    }
    else if (requestType == 'getstops')
    {
      getStops.handleRequest(route, direction);
    }
    else if (requestType == 'getpredictions')
    {
      if (extra == null)
      {
        console.log("~ handling predictions");
        getPredictions.handleRequest(route, direction, stopid, stopname);
      }
      else
      {
        console.log('~ handling predictions_favorites');
        handlePredictionsRequest_Favorites(extra);
      }
    }
    else if (requestType == 'getfavorites')
    {
      getFavorites.handleRequest()
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
      getNearbyStops.handleRequest();
    }
    else if (requestType == 'getnearbyroutes')
    {
      getNearbyRoutes.handleRequest(stopid);
    }
    else if (requestType == 'getcommute')
    {
      console.log('getting commute...');
      getCommute.handleRequest();
    }
    else if (requestType == 'getsaveddata')
    {
      console.log('get saved data');
      getSavedData.handleRequest(route, direction, stopid, stopname);
    }
  });
