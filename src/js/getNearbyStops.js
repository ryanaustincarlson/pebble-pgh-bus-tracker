var getNearbyStops = {

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

    get : function()
    {
        AllstopsManager.get(function(allstops) {
            distances = [];

            var start = {lat:40.455771, lon:-79.931979};

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

            for (var i=0; i<10; i++)
            {
                console.log(JSON.stringify(distances[i]) + '\n');
            }
        });
    }
};