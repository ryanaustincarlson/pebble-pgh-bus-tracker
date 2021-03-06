
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
      // selector
      return [prediction.rt, prediction.rtdir, prediction.stpid, prediction.stpnm].join('_');
    });
  },
  handleRequest : function(route, direction, stopid, stopname)
  {
    getPredictions.get(route, direction, stopid);
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
var handlePredictionsRequest_Favorites = function(selector)
{
  var fields = selector.split(PersistentDataManagerUtils.separator);
  var route = fields[0];
  var direction = fields[1];
  var stopid = fields[2];
  var stopname = fields[3];

  console.log('predictions request') // TODO: log what i'm sending to predictions handler

  getPredictions.handleRequest(route, direction, stopid, stopname);
};
