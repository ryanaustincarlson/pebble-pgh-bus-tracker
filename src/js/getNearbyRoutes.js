
var getNearbyRoutes = {

    savedData : null,

    sendNextRoute : function()
    {
        Dispatcher.sendNextItem(getNearbyRoutes, 'getnearbyroutes');
    },

    get : function(stopid)
    {
        var allstops = AllstopsManager.allstops;
        var ourstop = allstops[stopid];

        var routeStrings = ourstop.rt;
        var routes = []
        for (var i=0; i<routeStrings.length; i++)
        {
            var routeFields = routeStrings[i].split('_');
            console.log(JSON.stringify(routeFields));
            routes.push({
                rt : routeFields[0],
                name : routeFields[1],
                direction : routeFields[2]
            });
        }

        Dispatcher.organizeAndSaveData(routes, getNearbyRoutes, function(data) {
            return data;
        }, getRoutes.sortRoutesFcn, function(item) {
            // title
            return item.rt;
        }, function(item) {
            // subtitle
            return item.name + ' - ' + item.direction;
        }, function(item) {
            // selector
            return item.direction;
        });

        sendMenuSetupMessage(getNearbyRoutes.savedData.titles.length, "getnearbyroutes");

    }
};

var handleNearbyRoutesRequest = function(should_init, stopid)
{
    if (should_init)
    {
        getNearbyRoutes.savedData = null;
        getNearbyRoutes.get(stopid);
    }
    else
    {
        getNearbyRoutes.sendNextRoute();
    }
}
