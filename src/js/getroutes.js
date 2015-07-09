
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
    Dispatcher.sendNextItem(getroutes, 'getroutes');
  },
  get : function()
  {
    // console.log('my saved data: ' + getroutes.savedData);

    Dispatcher.sendRequest(getroutes, 'getroutes', 'getroutes', {}, function(data){
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

var handleRoutesRequest = function(should_init)
{
  if (should_init)
  {
    if (getroutes.savedData == null)
    {
      getroutes.get();
    }
    else
    {
      /* else, this is not the first time we're requesting routes
      * so there's really no need to make a network request, just
      * reset the index and re-send the data!
      */       
      getroutes.savedData.index = 0;
      sendMenuSetupMessage(getroutes.savedData.titles.length, "getroutes");
    }
  }
  else
  {
    getroutes.sendNextRoute();
  }
};
