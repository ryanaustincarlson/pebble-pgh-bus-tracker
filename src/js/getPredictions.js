
var getPredictions = {
  savedData : null,
  checkedFavorite : false,
  sendNextPrediction : function(requestType)
  {
    if (requestType == null)
    {
      requestType = 'getpredictions'
    }
    Dispatcher.sendNextItem(getPredictions, requestType);
  },
  get : function(route, direction, stopid, displayRequestType)
  {
    var params = {
      'rt' : route,
      'dir' : direction,
      'stpid' : stopid
    };

    if (displayRequestType == null)
    {
      displayRequestType = 'getpredictions'
    }

    Dispatcher.sendRequest(getPredictions, 'getpredictions', displayRequestType, params, function(data) {
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
};

var handlePredictionsRequest = function(should_init, route, direction, stopid, stopname, requestType)
{
  if (should_init)
  {
    getPredictions.checkedFavorite = false; // FIXME: prob don't need this now
    getPredictions.savedData = null;

    isfavorite = PersistentFavoritesManager.isFavorite(route, direction, stopid, stopname);
    var dictionary = {
      "KEY_IS_FAVORITE" : isfavorite ? 1 : 0,
      "KEY_MSG_TYPE" : requestType
    };

    Pebble.sendAppMessage(dictionary, 
      function(e) {}, 
      function(e) {});
    getPredictions.checkedFavorite = true;
  }
  else if (getPredictions.savedData == null)
  {
    getPredictions.get(route, direction, stopid, requestType);
  }
  else
  {
    getPredictions.sendNextPrediction(requestType);
  }
};

/* predictions coming from the *favorites* menu */
var handlePredictionsRequest_Favorites = function(should_init, selector)
{
  var fields = selector.split(getFavorites.separator);
  var route = fields[0];
  var direction = fields[1];
  var stopid = fields[2];
  var stopname = fields[3];

  console.log('predictions request') // TODO: log what i'm sending to predictions handler

  handlePredictionsRequest(should_init, route, direction, stopid, stopname, 'getpredictions');
};
