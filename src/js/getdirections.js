
var getdirections = {
  savedData : null,
  sendNextDirection : function()
  {
    Dispatcher.sendNextItem(getdirections, 'getdirections');
  },
  get : function(route)
  {
    Dispatcher.sendRequest(getdirections, 'getdirections', 'getdirections', {'rt':route}, function(data){
      return data['bustime-response'].directions;
    }, null, function(direction) {
      return direction.dir;
    }, function(direction) {
      return null;
    }, function(direction) {
      return direction.dir;
    });
  }
};

var handleDirectionsRequest = function(should_init, route)
{
  if (should_init)
  {
    getdirections.savedData = null;
    getdirections.get(route);
  }
  else
  {
    getdirections.sendNextDirection();
  }
};
