
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
  },
  handleRequest : function(route, direction)
  {
    getStops.savedData = null;
    getStops.get(route, direction);
  }
};
