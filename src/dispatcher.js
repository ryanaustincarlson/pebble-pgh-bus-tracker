var OFFLINE_MODE = false;

var URLUtils = {
  constructURL : function(requestType, data)
  {
    var url = 'http://truetime.portauthority.org/bustime/api/v2/';
    url += requestType + '?';
    data.key = 'myAm3A47DLjS4wuSvvHCrgs42';
    data.format = 'json';
    var params = [];
    for (var key in data)
    {
      params.push(encodeURIComponent(key) + '=' + encodeURIComponent(data[key]));
    }
    url += params.join('&');
    return url;
  },
  
  sendRequest : function(url, callback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
      callback(this.responseText);
    };
    xhr.open('GET', url);
    xhr.send();
  }
};

var sendMenuSetupMessage = function(num_entries, msgType)
{
  /*if (num_entries > 6)
  {
    num_entries = 6;
  }*/
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
      // console.log("Error sending # entries to pebble :(");
    });
}

var sendMenuEntryMessage = function(title, subtitle, selector, index, msgType)
{
  /*
  console.log('sending for ' + msgType + 
              '... title: ' + title + 
              ', subtitle: ' + subtitle + 
              ', selector: ' + selector +
              ', idx: ' + index);*/

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
      // console.log("Error sending " + msgType + " to pebble :(");
    }
    );
}

var Dispatcher = {
  sendNextItem : function(savedDataContainer, requestType, 
                          resetSavedDataOnFinish)
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

    if (!nextTitle ) /* || index > 5) */
    {
      nextTitle = "done";
      nextSubtitle = "done";

      if (resetSavedDataOnFinish)
      {
        savedData = null;
      }
      else
      {
        savedData.index = -1;
      }
    }

    savedDataContainer.savedData = savedData;

    sendMenuEntryMessage(nextTitle, nextSubtitle, nextSelector, index, requestType);
  },
  sendRequest : function(savedDataContainer, requestType, requestData, 
                         extractDataFcn, sortDataFcn, extractTitleFcn, 
                         extractSubtitleFcn, extractSelectorFcn)
  {
    var url = URLUtils.constructURL(requestType, requestData);
    // console.log('url: ' + url);
    URLUtils.sendRequest(url, function(responseText) {
      if (!!responseText)
      {
        var data = JSON.parse(responseText);
        var items = extractDataFcn(data);
        if (!items)
        {
          items = []
        }

        // custom sort?
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

        sendMenuSetupMessage(titles.length, requestType);
      }
    });
  }
}


var getroutes = {
  sortRoutesFcn : function(a, b)
  {
    a = a.rt;
    b = b.rt;

    var retValue = null;

    // console.log(a,b);
    var aIsNum = !isNaN(a);
    var bIsNum = !isNaN(b);
    if (aIsNum && bIsNum)
    {
      retValue = Number(a) - Number(b);
    }
    else
    {
        // one of two formats if NaN
        // letterFirst: <letter><number>
        // numberFirst: <number><letter>
        // 
        // letters are only ever one character long
        // numbers can be multiple chars
        var aIsLetterFirst = isNaN(a.substr(0,1));
        var bIsLetterFirst = isNaN(b.substr(0,1));

        var aLetter = '-', bLetter = '-';
        var aNumber = Number(a), bNumber = Number(b);

        if (!aIsNum)
        {
          aNumber = Number(aIsLetterFirst ? a.substr(1) : a.substr(0, a.length-1));
          aLetter = aIsLetterFirst ? a.substr(0,1) : a.substr(a.length-1);
        }

        if (!bIsNum)
        {
          bNumber = Number(bIsLetterFirst ? b.substr(1) : b.substr(0, b.length-1));
          bLetter = bIsLetterFirst ? b.substr(0,1) : b.substr(b.length-1);
        }

        if (!aIsLetterFirst && !bIsLetterFirst) /* numbers first */
        {
          if (aNumber == bNumber)
          {
            retValue = aLetter < bLetter ? -1 : 1;
          }
          else
          {
            retValue = aNumber - bNumber;
          }
        }
        else if (!aIsLetterFirst && bIsLetterFirst)
        {
          retValue = -1;
        }
        else if (aIsLetterFirst && !bIsLetterFirst)
        {
          retValue = 1;
        }
        else /* both letters first */
        {
          if (aLetter == bLetter)
          {
            retValue = aNumber - bNumber;
          }
          else
          {
            retValue = aLetter < bLetter ? -1 : 1;
          }
        }
      }
    // console.log(a + ' <-> ' + b + ' = ' + retValue);
    return retValue;
  },
  savedData : null,
  sendNextRoute : function()
  {
    // console.log('.sendNextRoute -> dispatcher.savedData: ' + getroutes.savedData);
    Dispatcher.sendNextItem(getroutes, 'getroutes', false);
  },
  get : function()
  {
    // console.log('my saved data: ' + getroutes.savedData);

    Dispatcher.sendRequest(getroutes, 'getroutes', {}, function(data){
      return data['bustime-response'].routes;
    }, getroutes.sortRoutesFcn, function(route) {
      return route.rt;
    }, function(route) {
      return route.rtnm;
    }, function(route) {
      return route.rt;
    });
  }
};

var getdirections = {
  savedData : null,
  sendNextDirection : function()
  {
    Dispatcher.sendNextItem(getdirections, 'getdirections', true);
  },
  get : function(route)
  {
    Dispatcher.sendRequest(getdirections, 'getdirections', {'rt':route}, function(data){
      return data['bustime-response'].directions;
    }, null, function(direction) {
      return direction.dir;
    }, function(direction) {
      return null;
    }, function(direction) {
      return direction.dir;
    });
  }
};

var getstops = {
  savedData : null,
  sendNextStop : function()
  {
    Dispatcher.sendNextItem(getstops, 'getstops', true);
  },
  get : function(route, direction)
  {
    var params = {
      'rt' : route,
      'dir' : direction
    };

    Dispatcher.sendRequest(getstops, 'getstops', params, function(data){
      return data['bustime-response'].stops;
    }, null, function(stop) {
      return stop.stpnm;
    }, function(stop) {
      return null;
    }, function(stop) {
      return stop.stpid;
    });
  }
}

var getpredictions = {
  savedData : null,
  sendNextPrediction : function()
  {
    Dispatcher.sendNextItem(getpredictions, 'getpredictions', true);
  },
  get : function(route, direction, stopid)
  {
    var params = {
      'rt' : route,
      'dir' : direction,
      'stpid' : stopid
    };

    Dispatcher.sendRequest(getpredictions, 'getpredictions', params, function(data) {
      return data['bustime-response'].prd;
    }, null, function(prediction) {
      var route = prediction.rt;
      var destination = prediction.des;
      var title = '#' + route + ' to ' + destination;
      return title;
    }, function(prediction) {
      var timeEstimate = prediction.prdctdn;
      if (!isNaN(timeEstimate))
      {
        timeEstimate += ' min';
      }
      return timeEstimate;
    }, function(prediction) {
      return 'foo'; // dunno what to do here...
    });
  }
}

var handleRoutesRequest = function()
{
  if (getroutes.savedData)
  {
    if (getroutes.savedData.index >= 0)
    {
      getroutes.sendNextRoute();
    }
    /* else, this is not the first time we're requesting routes
     * so there's really no need to make a network request, just
     * reset the index and re-send the data!
     */ 
     else
     {
      getroutes.savedData.index = 0;
      sendMenuSetupMessage(getroutes.savedData.titles.length, "getroutes");
    }
  }
  else
  {
    getroutes.get(); 
  }
}

var handleDirectionsRequest = function(payload)
{
  if (getdirections.savedData)
  {
    getdirections.sendNextDirection();
  }
  else
  {
    var route = payload['1'];
    // console.log("get directions with route: " + route);

    getdirections.get(route);
  }
}

var handleStopsRequest = function(payload)
{
  if (getstops.savedData)
  {
    getstops.sendNextStop();
  }
  else
  {
    var route = payload['1'];
    var direction = payload['2'];

    getstops.get(route, direction);
  }
}

var handlePredictionsRequest = function(payload)
{
  if (getpredictions.savedData)
  {
    getpredictions.sendNextPrediction();
  }
  else
  {
    var route = payload['1'];
    var direction = payload['2'];
    var stopid = payload['3'];

    console.log('rt: ' + route + ', dir: ' + direction + ', stopid: ' + stopid);

    getpredictions.get(route, direction, stopid);
  }
}

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    // console.log("AppMessage received!");
    var requestType = e.payload["0"];
    // console.log('request type: ' + requestType);
    
    if (requestType == 'getroutes')
    {
      handleRoutesRequest();
    }
    else if (requestType == 'getdirections')
    {
      // handleRoutesRequest();
      handleDirectionsRequest(e.payload);
    }
    else if (requestType == 'getstops')
    {
      handleStopsRequest(e.payload);
    }
    else if (requestType == 'getpredictions')
    {
      handlePredictionsRequest(e.payload);
    }
  });