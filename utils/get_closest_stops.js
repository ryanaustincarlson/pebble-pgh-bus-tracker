// load up from file
var fs = require('fs');
var allstops = JSON.parse(fs.readFileSync('allstops.json', 'utf8'));

var toRad = function(num) {
    return num * Math.PI / 180
}

var haversine = function(start, end) {
    var R = 6371; // in kilometers

    var dLat = toRad(end.lat - start.lat);
    var dLon = toRad(end.lon - start.lon);
    var lat1 = toRad(start.lat);
    var lat2 = toRad(end.lat);

    var a = Math.sin(dLat/2) * Math.sin(dLat/2) +
            Math.sin(dLon/2) * Math.sin(dLon/2) * Math.cos(lat1) * Math.cos(lat2);
    var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1-a));

    return R * c;
}

start = {lat:40.455771, lon:-79.931979}
console.log(start)

end_close = {lat:40.456504766836,lon:-79.93290716666797}
end_far = {lat:40.444059594464,lon:-79.925951757275}

haversine_close = haversine(start, end_close)
haversine_far = haversine(start, end_far)

console.log('close: ' + haversine_close)
console.log('far: ' + haversine_far)

distances = []
for (var stopid in allstops)
{
    stop = allstops[stopid]
    distance = haversine(start, stop)

    distances.push({
        dist  : distance,
        stpid : stopid,
        stpnm : stop.stpnm,
        rt    : stop.rt
    });
}

// sort by distance
distances.sort(function(a, b) {
    return a.dist - b.dist;
});

for (var i=0; i<10; i++)
{
    console.log(distances[i]);
}
