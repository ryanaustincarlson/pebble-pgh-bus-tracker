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
  
  parseRoutes : function(routes)
  {
    // console.log('> sorting...');
    routes.sort(getroutes.sortRoutesFcn);
    // console.log('> done sorting!');
    
    var titles = [];
    var subtitles = [];
    for (var i=0; i<routes.length; i++)
    {
      var route = routes[i];
      var routeNum = route.rt;
      var routeName = route.rtnm;

      // console.log(routeNum + ' - ' + routeName);
      titles.push(routeNum);
      subtitles.push(routeName);
    }
    return {titles : titles, subtitles : subtitles};
  },
  savedRoutes : null,
  sendNextRoute : function()
  {
    // console.log('in send next route, savedRoutes: ' + getroutes.savedRoutes);
    var titles = getroutes.savedRoutes.titles;
    var subtitles = getroutes.savedRoutes.subtitles;
    var index = getroutes.savedRoutes.index;
    
    var nextTitle = titles[index];
    var nextSubtitle = subtitles[index];
    getroutes.savedRoutes.index = index+1;
    
    if (!nextTitle || !nextSubtitle)
    {
      nextTitle = "done";
      nextSubtitle = "done";
      getroutes.savedRoutes.index = 0;
    }
    
    console.log('sending title: ', nextTitle);
    console.log('sending subtitle: ', nextSubtitle);
    
    var dictionary = {
      "KEY_TITLES" : nextTitle,
      "KEY_SUBTITLES" : nextSubtitle,
      "KEY_ITEM_INDEX" : index
    };
    
    Pebble.sendAppMessage(dictionary,
      function(e) {
        console.log("Routes sent to pebble successfully!");
      },
      function(e) {
        console.log("Error sending routes to pebble :(");
      }
    );
  },
  get : function(onSuccess)
  {
    var url = URLUtils.constructURL('getroutes', {});
    URLUtils.sendRequest(url, function(responseText){
      // console.log('received http response!');

      // responseText contains a JSON object with weather info
      var data = JSON.parse(responseText);
      var routes = data['bustime-response'].routes;
      var parsed = getroutes.parseRoutes(routes);
      
      // console.log('setting up saved routes');
      getroutes.savedRoutes = {
        titles : parsed.titles,
        subtitles : parsed.subtitles,
        index : 0
      };

      Pebble.sendAppMessage({"KEY_NUM_ENTRIES" : parsed.titles.length},
        function(e) {
          console.log("Num Entries sent to pebble successfully!");
        },
        function(e) {
          console.log("Error sending Num Entries to pebble :(");
        });
    });
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
      if (!getroutes.savedRoutes)
      {
        getroutes.get();
      }
      else
      {
        getroutes.sendNextRoute();
      }
    }
  }                     
);