var getNearbyStops = {

    savedData : null,

    haversine : function(start, end) {
        var R = 6371; /* in kilometers */

        var toRad = function(angle) {
            return angle * Math.PI / 180;
        }

        var dLat = toRad(end.lat - start.lat);
        var dLon = toRad(end.lon - start.lon);
        var lat1 = toRad(start.lat);
        var lat2 = toRad(end.lat);

        var a = Math.sin(dLat/2) * Math.sin(dLat/2) +
        Math.sin(dLon/2) * Math.sin(dLon/2) * Math.cos(lat1) * Math.cos(lat2);
        var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));

        return R * c;
    },

    sendNextStop : function()
    {
        Dispatcher.sendNextItem(getNearbyStops, 'getnearbystops');
    },

    get : function()
    {
        AllstopsManager.get(function(allstops) {
            distances = [];

            var locationOptions = {
              enableHighAccuracy: true, 
              maximumAge: 10000, 
              timeout: 10000
            };

            navigator.geolocation.getCurrentPosition(function(position) {
                console.log('success!: ', JSON.stringify(position));
                console.log('lat= ' + position.coords.latitude + ' lon= ' + position.coords.longitude);


                // TODO: get from device...
                var start = {lat:40.455771, lon:-79.931979}; // negley
                // var start = {lat:40.441669, lon:-79.9140807}; // dalzell
                // var start = {lat:position.coords.latitude, lon:position.coords.longitude};

                for (var stopid in allstops)
                {
                    stop = allstops[stopid];
                    distance = getNearbyStops.haversine(start, stop);

                    distances.push({
                        dist  : distance,
                        stpid : stopid,
                        stpnm : stop.stpnm,
                        rt    : stop.rt
                    });
                }

                /* sort by distance */
                distances.sort(function(a, b) {
                    return a.dist - b.dist;
                });

                distances = distances.slice(0, 10);

                Dispatcher.organizeAndSaveData(distances, getNearbyStops, function(data) {
                    return data;
                }, null, function(item) {
                    /* title */
                    return item.stpnm;
                }, function(item) {
                    /* subtitle */
                    return 'Stop ' + item.stpid;
                }, function(item) {
                    /* selector */
                    return item.stpid;
                });

                /* console.log(JSON.stringify(getNearbyStops.savedData)); */
                Dispatcher.sendMenuSetupMessage(getNearbyStops, "getnearbystops");
            }, function(error) {
                console.log("error!");
            }, locationOptions);
        });
    },

    handleRequest : function(should_init)
    {
        if (should_init)
        {
            getNearbyStops.savedData = null;
            getNearbyStops.get();
        }
        else
        {
            Dispatcher.sendNextItem(getNearbyStops, 'getnearbystops');
        }
    }
};

// var handleNearbyStopsRequest = function(should_init)
// {
//   if (should_init)
//   {
//     getNearbyStops.savedData = null;
//     getNearbyStops.get();
//   }
//   else
//   {
//     getNearbyStops.sendNextStop();
//   }
// };

