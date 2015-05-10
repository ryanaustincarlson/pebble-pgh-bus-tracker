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
  console.log('for ' + msgType + '... sending menu setup message w/ ' + num_entries + ' entries');

  var dictionary = {
    "KEY_NUM_ENTRIES" : num_entries,
    "KEY_MSG_TYPE" : msgType    
  }

  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log("for " + msgType + "... # entries sent to pebble successfully!");
    },
    function(e) {
      console.log("Error sending # entries to pebble :(");
    });
}

var sendMenuEntryMessage = function(title, subtitle, index, msgType)
{
  console.log('sending for ' + msgType + '... title: ' + title + ', subtitle: ' + subtitle + ', idx: ' + index);

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

  Pebble.sendAppMessage(dictionary,
    function(e) {
      console.log(msgType + " sent to pebble successfully!");
    },
    function(e) {
      console.log("Error sending " + msgType + " to pebble :(");
    }
    );
}

var Dispatcher = {
  sendNextItem : function(savedDataContainer, requestType, resetSavedDataOnFinish)
  {
    var savedData = savedDataContainer.savedData;

    var titles = savedData.titles;
    var subtitles = savedData.subtitles;
    var index = savedData.index;

    var nextTitle = titles[index];
    var nextSubtitle = null;
    if (!!subtitles)
    {
      nextSubtitle = subtitles[index];
    }

    savedData.index = index+1;

    if (!nextTitle)
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

    sendMenuEntryMessage(nextTitle, nextSubtitle, index, requestType);
  },
  sendRequest : function(savedDataContainer, requestType, requestData, extractDataFcn, sortDataFcn, extractTitleFcn, extractSubtitleFcn)
  {
    var url = URLUtils.constructURL(requestType, requestData);
    URLUtils.sendRequest(url, function(responseText) {
      if (!!responseText)
      {
        var data = JSON.parse(responseText);
        items = extractDataFcn(data);

        // custom sort?
        if (!!sortDataFcn)
        {
          items.sort(sortDataFcn);
        }

        should_use_subtitles = true;

        var titles = [];
        var subtitles = [];
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
        }

        savedDataContainer.savedData = {
          titles : titles,
          subtitles : subtitles,
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
    }, null, function(route) {
      return route.rt;
    }, function(route) {
      return route.rtnm;
    });
  }
};

// console.log('creating new routes');
// var getroutes = new RoutesDispatcher();
// getroutes.dispatcher = new Dispatcher(false);
  
  // parseRoutes : function(routes)
  // {
  //   // console.log('> sorting...');
  //   routes.sort(getroutes.sortRoutesFcn);
  //   // console.log('> done sorting!');

  //   var titles = [];
  //   var subtitles = [];
  //   for (var i=0; i<routes.length; i++)
  //   {
  //     var route = routes[i];
  //     var routeNum = route.rt;
  //     var routeName = route.rtnm;

  //     // console.log(routeNum + ' - ' + routeName);
  //     titles.push(routeNum);
  //     subtitles.push(routeName);
  //   }
  //   return {titles : titles, subtitles : subtitles};
  // },
  // savedRoutes : null,
  // sendNextRoute : function()
  // {
  //   // console.log('in send next route, savedRoutes: ' + getroutes.savedRoutes);
  //   var titles = getroutes.savedRoutes.titles;
  //   var subtitles = getroutes.savedRoutes.subtitles;
  //   var index = getroutes.savedRoutes.index;

  //   var nextTitle = titles[index];
  //   var nextSubtitle = subtitles[index];
  //   getroutes.savedRoutes.index = index+1;

  //   if (!nextTitle || !nextSubtitle || index > 2)
  //   {
  //     nextTitle = "done";
  //     nextSubtitle = "done";
  //     getroutes.savedRoutes.index = -1;
  //   }

  //   // console.log('sending title: ', nextTitle);
  //   // console.log('sending subtitle: ', nextSubtitle);
  //   sendMenuEntryMessage(nextTitle, nextSubtitle, index, "routes");    
  // },
  // get : function(onSuccess)
  // {
  //   if (!OFFLINE_MODE)
  //   {
  //     var url = URLUtils.constructURL('getroutes', {});
  //     URLUtils.sendRequest(url, function(responseText){
  //       if (!!responseText)
  //       {
  //         /* responseText contains a JSON object */
  //         var data = JSON.parse(responseText);
  //         var routes = data['bustime-response'].routes;
  //         var parsed = getroutes.parseRoutes(routes);

  //         getroutes.savedRoutes = {
  //           titles : parsed.titles,
  //           subtitles : parsed.subtitles,
  //           index : 0
  //         };

  //         sendMenuSetupMessage(parsed.titles.length, "routes");
  //       }
  //     });
  //   }
  //   else
  //   {
  //     getroutes.savedRoutes = {
  //       titles: ['1', '2', '61A', '61B', 'P1', 'P2', 'P10'],
  //       subtitles: ['a', 'b', 'c', 'd', 'e', 'f', 'g'],
  //       index: 0
  //     };

  //     sendMenuSetupMessage(getroutes.savedRoutes.titles.length, "routes");
  //   }
  // }

var getdirections = {
  parseDirections : function(directions)
  {
    var items = [];
    for (var i=0; i<directions.length; i++)
    {
      var direction = directions[i].dir;

      // console.log('dir: ' + direction);
      items.push(direction);
    }
    return items;
  },
  savedDirections : null,
  sendNextDirection : function()
  {
    var titles = getdirections.savedDirections.titles;
    var index = getdirections.savedDirections.index;

    /*
    console.log('(2) ~~ titles count: ' + titles.length);
    for (var i=0; i<titles.length; i++)
    {
      console.log('(2) title[' + i + '] = ' + titles[i]);
    }*/
    
    var nextTitle = titles[index];

    getdirections.savedDirections.index = index+1;
    
    if (!nextTitle)
    {
      nextTitle = "done";
      getdirections.savedDirections = null;
    }

    sendMenuEntryMessage(nextTitle, null, index, "directions");
  },
  get : function(route)
  {
    if (!OFFLINE_MODE)
    {
      var url = URLUtils.constructURL('getdirections', {'rt' : route});
      console.log('url: ' + url);
      URLUtils.sendRequest(url, function(responseText){
        var data = JSON.parse(responseText);

        console.log('success!');
        console.log('data: ' + data);
        console.log('response: ' + data['bustime-response']);
        var directions = data['bustime-response'].directions;
        console.log('dirs: ' + directions);
        var items = getdirections.parseDirections(directions);

        getdirections.savedDirections = {
          titles : items,
          index : 0
        };

        sendMenuSetupMessage(items.length, "directions");
      });
    }
    else
    {
      getdirections.savedDirections = {
        titles: ['INBOUND', 'OUTBOUND'],
        index: 0
      };

      sendMenuSetupMessage(getdirections.savedDirections.titles.length, "directions");
    }
  }
};

// var getdirections = {
//   savedDirections : null,
//   sendNextDirection : function()
//   {
//     var titles = getdirections.savedDirections.titles;
//     var index = getdirections.savedDirections.index;

//     /*
//     console.log('(2) ~~ titles count: ' + titles.length);
//     for (var i=0; i<titles.length; i++)
//     {
//       console.log('(2) title[' + i + '] = ' + titles[i]);
//     }*/

//     var nextTitle = titles[index];
//     if (nextTitle)
//     {
//       nextTitle += ' '
//     }

//     getdirections.savedDirections.index = index+1;

//     if (!nextTitle)
//     {
//       nextTitle = "done";
//       getdirections.savedDirections = null;
//     }

//     sendMenuEntryMessage(nextTitle, null, index, "directions");
//   },
//   get : function(route, direction)
//   {
//     // if (!OFFLINE_MODE)
//     // {

//     // }
//     // else
//     // {
//       getdirections.savedDirections = {
//         titles: ['inbound', 'outbound'],
//         index: 0
//       };

//       sendMenuSetupMessage(getdirections.savedDirections.titles.length, "directions");
//     // }
//   }
// }

var getstops = {
  savedStops : null,
  sendNextStop : function()
  {
    var titles = getstops.savedStops.titles;
    var index = getstops.savedStops.index;

    /*
    console.log('(2) ~~ titles count: ' + titles.length);
    for (var i=0; i<titles.length; i++)
    {
      console.log('(2) title[' + i + '] = ' + titles[i]);
    }*/
    
    var nextTitle = titles[index];

    getstops.savedStops.index = index+1;
    
    if (!nextTitle)
    {
      nextTitle = "done";
      getstops.savedStops = null;
    }

    sendMenuEntryMessage(nextTitle, null, index, "stops");
  },
  get : function(route, direction)
  {
    // if (!OFFLINE_MODE)
    // {

    // }
    // else
    // {
      getstops.savedStops = {
        titles: ['Negley Station', 'Negley @ Ellesworth'],
        index: 0
      };

      // sendMenuSetupMessage(getstops.savedStops.titles.length, "stops");
    // }
  }
}

var handleRoutesRequest = function()
{
  // console.log('getroutes: ' + getroutes);
  // console.log('dispatcher: ' + getroutes.dispatcher);
  // console.log('savedData: ' + getroutes.dispatcher.savedData);
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
      sendMenuSetupMessage(getroutes.dispatcher.savedData.titles.length, "getroutes");
    }
  }
  else
  {
    getroutes.get(); 
  }
}

// getroutes.get();
// setTimeout(getroutes.get, 3000);
// handleRoutesRequest();
// setTimeout(handleRoutesRequest, 3000);

var handleDirectionsRequest = function(payload)
{
  if (getdirections.savedDirections)
  {
    getdirections.sendNextDirection();
    // getroutes.sendNextRoute();
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
  if (getstops.savedStops)
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
  });