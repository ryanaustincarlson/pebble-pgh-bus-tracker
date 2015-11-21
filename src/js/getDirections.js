
var getDirections = {
  savedData : null,
  sendNextDirection : function()
  {
    
  },
  get : function(route)
  {
    Dispatcher.sendRequest(getDirections, 'getdirections', 'getdirections', {'rt':route}, function(data){
      return data['bustime-response'].directions;
    }, null, function(direction) {
      return direction.dir;
    }, function(direction) {
      return null;
    }, function(direction) {
      return direction.dir;
    });
  },
  handleRequest : function(should_init, route)
  {
    if (should_init)
    {
      getDirections.savedData = null;
      getDirections.get(route);
    }
    else
    {
      Dispatcher.sendNextItem(getDirections, 'getdirections');
    }
  }
};
