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
  if (num_entries > 6)
  {
    num_entries = 6;
  }
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
                          resetSavedDataFcn)
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

    if (!nextTitle || index > 5) 
    {
      nextTitle = "done";
      nextSubtitle = "done";

      resetSavedDataFcn();

      // if (resetSavedDataOnFinish)
      // {
      //   savedData = null;
      // }
      // else
      // {
      //   savedData.index = -1;
      // }
    }

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
    Dispatcher.sendNextItem(getroutes, 'getroutes', function() {
      getroutes.savedData.index = -1;
    });
  },
  get : function()
  {
    // console.log('my saved data: ' + getroutes.savedData);

    Dispatcher.sendRequest(getroutes, 'getroutes', {}, function(data){
      return data['bustime-response'].routes;
    }, getroutes.sortRoutesFcn, function(route) {
      return 'P1';
      // return route.rt;
    }, function(route) {
      return route.rtnm;
    }, function(route) {
      return 'P1';
      // return route.rt;
    });
  }
};

var getdirections = {
  savedData : null,
  sendNextDirection : function()
  {
    Dispatcher.sendNextItem(getdirections, 'getdirections', function() {
      getdirections.savedData = null;
    });
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
    Dispatcher.sendNextItem(getstops, 'getstops', function() {
      getstops.savedData = null;
    });
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
  checkedFavorite : false,
  sendNextPrediction : function()
  {
    Dispatcher.sendNextItem(getpredictions, 'getpredictions', function() {
      getpredictions.savedData = null;
      getpredictions.checkedFavorite = false;
    });
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
  var route = payload['1'];
  var direction = payload['2'];
  var stopid = payload['3'];
  var stopname = payload['4'];

  if (getpredictions.checkedFavorite == false)
  {
    // TODO: check if we're actually a favorite
    isfavorite = PersistentFavoritesManager.isFavorite(route, direction, stopid, stopname);
    var dictionary = {
      "KEY_IS_FAVORITE" : isfavorite ? 1 : 0,
    };

    Pebble.sendAppMessage(dictionary, 
      function(e) {}, 
      function(e) {});
    getpredictions.checkedFavorite = true;
  }
  else if (getpredictions.savedData)
  {
    getpredictions.sendNextPrediction();
  }
  else
  {
    console.log('rt: ' + route + ', dir: ' + direction + ', stopid: ' + stopid);
    getpredictions.get(route, direction, stopid);
  }
}

var PersistentFavoritesManager = {
  separator : "_@_",
  favorites : null,
  loadFavorites : function()
  {
    var jsonfavs = localStorage.getItem('favorites');
    if (jsonfavs == null)
    {
      PersistentFavoritesManager.favorites = [];
    }
    else
    {
      PersistentFavoritesManager.favorites = JSON.parse(jsonfavs);
    }

    console.log('persistent loaded... ' + JSON.stringify(PersistentFavoritesManager.favorites));
  },
  saveFavorites : function()
  {
    if (PersistentFavoritesManager.favorites != null)
    {
      var jsonfavs = JSON.stringify(PersistentFavoritesManager.favorites);
      localStorage.setItem('favorites', jsonfavs);
      console.log('persistent saved... ' + jsonfavs);
    }
  },
  getStorageString : function(route, direction, stopid, stopname)
  {
    var sep = PersistentFavoritesManager.separator;
    var item = route + sep + direction + sep + stopid + sep + stopname;
    return item;
  },
  parseStorageString : function(storageString)
  {
    var sep = PersistentFavoritesManager.separator;
    var split = storageString.split(sep);
    var item = {
      route : split[0],
      direciton : split[1],
      stopid : split[2],
      stopname : split[3]
    };
    return item;
  },
  isFavorite : function(route, direction, stopid, stopname)
  {
    console.log("requesting favorites....");
    if (PersistentFavoritesManager.favorites == null)
    {
      PersistentFavoritesManager.loadFavorites();
    }

    var item = PersistentFavoritesManager.getStorageString(route, direction, stopid, stopname);
    var isfavorite = PersistentFavoritesManager.favorites.indexOf(item) >= 0;
    console.log(item + ' is favorite? ' + isfavorite);
    return isfavorite;
  },
  setFavorite : function(route, direction, stopid, stopname, isfavorite)
  {
    console.log('received set fav request rt: ' + 
    route + ', direction: ' + direction + ', stopid: ' +
    stopid + ', stopname: ' + stopname + ', isfav: ' + isfavorite); 

    if (PersistentFavoritesManager.favorites == null)
    {
      PersistentFavoritesManager.loadFavorites();
    }
    
    // use localStorage
    var item = PersistentFavoritesManager.getStorageString(route, direction, stopid, stopname);
    console.log('new item: ' + item);

    var changedFavorites = false;
    if (isfavorite)
    {
      /* only add if item's not already there */
      if (PersistentFavoritesManager.favorites.indexOf(item) == -1)
      {
        PersistentFavoritesManager.favorites.push(item);
        changedFavorites = true;
      }
    }
    else
    {
      var index = PersistentFavoritesManager.favorites.indexOf(item);
      if (index >= 0)
      {
        PersistentFavoritesManager.favorites.splice(index, 1);
        changedFavorites = true;
      }
    }

    if (changedFavorites)
    {
      PersistentFavoritesManager.saveFavorites();
    }
  }
};

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
    else if (requestType == 'setfavorite')
    {
      var route = e.payload['1'];
      var direction = e.payload['2'];
      var stopid = e.payload['3'];
      var stopname = e.payload['4'];
      var isfavorite = e.payload['5'] == 1;

      PersistentFavoritesManager.setFavorite(route, 
        direction, stopid, stopname, isfavorite);
    }
  });