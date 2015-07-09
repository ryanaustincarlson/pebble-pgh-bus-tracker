
var getstops = {
  savedData : null,
  sendNextStop : function()
  {
    Dispatcher.sendNextItem(getstops, 'getstops');
  },
  get : function(route, direction)
  {
    var params = {
      'rt' : route,
      'dir' : direction
    };

    Dispatcher.sendRequest(getstops, 'getstops', 'getstops', params, function(data){
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
    getstops.savedData = null;
    getstops.get(route, direction);
  }
  else
  {
    getstops.sendNextStop();
  }
};
