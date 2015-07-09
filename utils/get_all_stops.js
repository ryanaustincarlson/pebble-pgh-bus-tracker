var request = require('request')

var URLUtils = {
  constructURL : function(requestType, data)
  {
    var url = 'http://truetime.portauthority.org/bustime/api/v2/';
    url += requestType + '?';
    data.key = 'myAm3A47DLjS4wuSvvHCrgs42';
    data.format = 'json';
    var params = [];
    for (var key in data)
    {
      params.push(encodeURIComponent(key) + '=' + encodeURIComponent(data[key]));
    }
    url += params.join('&');
    return url;
  },
  
  sendRequest : function(url, callback) {
    request({
        url:url, 
        method:'GET'
      },
      function(error, response, body) {
        if (!error && response.statusCode == 200) {
            callback(body)
        }
    });
  }
};

var stops_map = {} // key = stopid, value = details
var expected_getstops_calls = 0;
var num_getstops_calls = 0;

var getstops = function(route, routename, direction)
{
    // console.log(route + ' _ ' + routename + ' _ ' + direction);
    var stops_url = URLUtils.constructURL('getstops', {'rt':route, 'dir':direction})
    URLUtils.sendRequest(stops_url, function(stops_responseText) {
        var stops = JSON.parse(stops_responseText)['bustime-response'].stops;
        // console.log(JSON.stringify(stops));
        for (var i=0; i<stops.length; i++)
        {
            var stopid = stops[i].stpid;
            var stopname = stops[i].stpnm;
            var latitude = stops[i].lat;
            var longitude = stops[i].lon;

            if (!(stopid in stops_map))
            {
                stops_map[stopid] = {
                    stpnm : stopname, 
                    lat : latitude, 
                    lon : longitude,
                    rt : []
                }
            }
            stops_map[stopid]['rt'].push(route + '_' + routename + '_' + direction);
        }
        num_getstops_calls += 1;
        if (num_getstops_calls == expected_getstops_calls)
        {
            // console.log(Object.keys(stops_map).length);
            console.log(stops_map);
            // console.log(JSON.stringify(stops_map));
        }
    });
}

var getdirections = function(route, routename)
{
    var directions_url = URLUtils.constructURL('getdirections', {'rt':route});
    URLUtils.sendRequest(directions_url, function(dirs_responseText) {
        var directions = JSON.parse(dirs_responseText)['bustime-response'].directions;
        for (var i=0; i<directions.length; i++)
        {
            var direction = directions[i]['dir'];
            getstops(route, routename, direction);
        }
    });
}

var getroutes = function()
{
    var routes_url = URLUtils.constructURL('getroutes', {});
    URLUtils.sendRequest(routes_url, function(routes_responseText) {
        var routes = JSON.parse(routes_responseText)['bustime-response'].routes;
        expected_getstops_calls = 2 * routes.length;
        for (var i=0; i<routes.length; i++)
        {
            var route = routes[i].rt;
            var routename = routes[i].rtnm;
            getdirections(route, routename);
        }
    });
}
getroutes();

