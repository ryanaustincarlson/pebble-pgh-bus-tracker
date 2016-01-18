
var getRoutes = {
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
        //
        // `substr` takes two args, a start index and a length
        // note that this is NOT like `substring` which is start idx and finish idx
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
    // console.log('.sendNextRoute -> dispatcher.savedData: ' + getRoutes.savedData);
    Dispatcher.sendNextItem(getRoutes, 'getroutes', getRoutes.handleRoutesRequest);
  },
  get : function()
  {
    // console.log('my saved data: ' + getRoutes.savedData);

    Dispatcher.sendRequest(getRoutes, 'getroutes', 'getroutes', {}, function(data){
      return data['bustime-response'].routes;
    }, getRoutes.sortRoutesFcn, function(route) {
      return route.rt;
    }, function(route) {
      return route.rtnm;
    }, function(route) {
      return route.rt;
    });
  },
  handleRequest : function()
  {
    if (getRoutes.savedData == null)
    {
      getRoutes.get();
    }
    else
    {
      /* else, this is not the first time we're requesting routes
      * so there's really no need to make a network request, just
      * reset the index and re-send the data!
      */
      getRoutes.savedData.index = 0;
      Dispatcher.sendMenuSetupMessage(getRoutes, "getroutes");
    }
  }
};
