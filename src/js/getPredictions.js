
var getPredictions = {
  savedData : null,
  sortPredictionsFcn : function(primaryRoute)
  {
    var innerSort = function(a, b)
    {
      var aRoute = a.rt;
      var bRoute = b.rt;

      var aEst = Number(a.prdctdn);
      var bEst = Number(b.prdctdn);

      if (aRoute == primaryRoute && bRoute != primaryRoute)
      {
        return -1;
      }
      else if (aRoute != primaryRoute && bRoute == primaryRoute)
      {
        return 1;
      }
      else
      {
        return aEst - bEst;
      }
    };
    return innerSort;
  },
  sendNextPrediction : function()
  {
    Dispatcher.sendNextItem(getPredictions, 'getpredictions');
  },
  get : function(route, direction, stopid)
  {
    var params = {
      // 'rt' : route,
      // 'dir' : direction,
      'stpid' : stopid
    };

    displayRequestType = 'getpredictions'

    var sortingFcn = getPredictions.sortPredictionsFcn(route);

    Dispatcher.sendRequest(getPredictions, 'getpredictions', displayRequestType, params, function(data) {
      return data['bustime-response'].prd;
    }, sortingFcn, function(prediction) {
      var timeEstimate = prediction.prdctdn;
      if (!isNaN(timeEstimate))
      {
        timeEstimate += ' min';
      }
      return timeEstimate;
    }, function(prediction) {
      var route = prediction.rt;
      var destination = prediction.des;
      var title = '#' + route + ' to ' + destination;
      return title;
    }, function(prediction) {
      return 'foo'; // dunno what to do here...
    });
  },
  handleRequest : function(should_init, route, direction, stopid, stopname)
  {
    if (should_init)
    {
      getPredictions.savedData = null;

      isfavorite = PersistentFavoritesManager.isFavorite(route, direction, stopid, stopname);
      ismorningcommute = PersistentMorningCommuteManager.isMorningCommute(route, direction, stopid, stopname);
      iseveningcommute = PersistentEveningCommuteManager.isEveningCommute(route, direction, stopid, stopname);
      var dictionary = {
        "KEY_IS_FAVORITE" : isfavorite ? 1 : 0,
        "KEY_IS_MORNING_COMMUTE" : ismorningcommute ? 1 : 0,
        "KEY_IS_EVENING_COMMUTE" : iseveningcommute ? 1 : 0,
        "KEY_MSG_TYPE" : 'getpredictions'
      };

      Pebble.sendAppMessage(dictionary,
        function(e) {
          // success
          getPredictions.handleRequest(false, route, direction, stopid, stopname)
        },
        function(e) {
          // failure - do nothing?
        });
    }
    else if (getPredictions.savedData == null)
    {
      getPredictions.get(route, direction, stopid);
    }
    else
    {
      Dispatcher.sendNextItem(getPredictions, 'getpredictions');
    }
  }
};

// var handlePredictionsRequest = function(should_init, route, direction, stopid, stopname)
// {
//   if (should_init)
//   {
//     getPredictions.savedData = null;

//     isfavorite = PersistentFavoritesManager.isFavorite(route, direction, stopid, stopname);
//     var dictionary = {
//       "KEY_IS_FAVORITE" : isfavorite ? 1 : 0,
//       "KEY_MSG_TYPE" : requestType
//     };

//     Pebble.sendAppMessage(dictionary,
//       function(e) {
//         // success
//         getPredictions.handleRequest(false, route, direction, stopid, stopname)
//       },
//       function(e) {
//         // failure - do nothing?
//       });
//   }
//   else if (getPredictions.savedData == null)
//   {
//     getPredictions.get(route, direction, stopid, requestType);
//   }
//   else
//   {
//     Dispatcher.sendNextItem(getPredictions, 'getpredictions');
//   }
// };

/* predictions coming from the *favorites* menu */
var handlePredictionsRequest_Favorites = function(should_init, selector)
{
  var fields = selector.split(getFavorites.separator);
  var route = fields[0];
  var direction = fields[1];
  var stopid = fields[2];
  var stopname = fields[3];

  console.log('predictions request') // TODO: log what i'm sending to predictions handler

  getPredictions.handleRequest(should_init, route, direction, stopid, stopname);
};
