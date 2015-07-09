
var getStops = {
  savedData : null,
  sendNextStop : function()
  {
    Dispatcher.sendNextItem(getStops, 'getstops');
  },
  get : function(route, direction)
  {
    var params = {
      'rt' : route,
      'dir' : direction
    };

    Dispatcher.sendRequest(getStops, 'getstops', 'getstops', params, function(data){
      return data['bustime-response'].stops;
    }, null, function(stop) {
      return stop.stpnm;
    }, function(stop) {
      return null;
    }, function(stop) {
      return stop.stpid;
    });
  }
};

var handleStopsRequest = function(should_init, route, direction)
{
  if (should_init)
  {
    getStops.savedData = null;
    getStops.get(route, direction);
  }
  else
  {
    getStops.sendNextStop();
  }
};
