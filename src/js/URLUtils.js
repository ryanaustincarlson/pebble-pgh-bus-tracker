
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
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
      callback(this.responseText);
    };
    xhr.open('GET', url);
    xhr.send();
  }
};
