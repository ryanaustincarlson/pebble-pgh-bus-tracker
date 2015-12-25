
var getCommute = {
  savedData : null,
  entries : null,
  sortPredictionsFcn : function(primaryRoute)
  {
    var innerSort = function(a, b)
    {
      var aRoute = a.rt;
      var bRoute = b.rt;

      var aEst = Number(a.prdctdn);
      var bEst = Number(b.prdctdn);

      if (aRoute == primaryRoute && bRoute != primaryRoute)
      {
        return -1;
      }
      else if (aRoute != primaryRoute && bRoute == primaryRoute)
      {
        return 1;
      }
      else
      {
        return aEst - bEst;
      }
    };
    return innerSort;
  },
  sendNextPrediction : function()
  {
    Dispatcher.sendNextItem(getCommute, 'getcommute');
  },
  get : function(entries)
  {
    var routes = [];
    var stopids = [];
    for (var i=0; i<entries.length; i++)
    {
      var route = entries[i].route;
      var stopid = entries[i].stopid;
      if (routes.indexOf(route) == -1)
      {
        routes.push(route);
      }

      if (stopids.indexOf(stopid) == -1)
      {
        stopids.push(stopid);
      }
    }
    var params = {
      'rt' : routes.join(','),
      // 'dir' : direction,
      'stpid' : stopids.join(',')
    };

    displayRequestType = 'getcommute'

    var sortingFcn = getCommute.sortPredictionsFcn(null);

    Dispatcher.sendRequest(getCommute, 'getpredictions', displayRequestType, params, function(data) {
      return data['bustime-response'].prd;
    }, sortingFcn, function(prediction) {
      var timeEstimate = prediction.prdctdn;
      if (!isNaN(timeEstimate))
      {
        timeEstimate += ' min';
      }
      return timeEstimate;
    }, function(prediction) {
      var route = prediction.rt;
      var destination = prediction.des;
      var title = '#' + route + ' to ' + destination + ' @ ' + prediction.stpnm;
      return title;
    }, function(prediction) {
      return [prediction.rt, prediction.rtdir, prediction.stpid, prediction.stpnm].join('_');
      // return 'foo'; // dunno what to do here...
    });
  },
  handleRequest : function(should_init, route, direction, stopid, stopname)
  {
    if (should_init)
    {
      getCommute.savedData = null;

      var currentHours = new Date().getHours();
      var isMorning = currentHours <= 12;

      var persistentDataManager = isMorning ? PersistentMorningCommuteManager : PersistentEveningCommuteManager;
      if (persistentDataManager.savedData == null)
      {
        persistentDataManager.loadCommute();
      }
      var persistentData = persistentDataManager.savedData;

      var sep = PersistentDataManagerUtils.separator;
      var entries = [];
      for (var i=0; i<persistentData.length; i++)
      {
        var commuteEntry = PersistentDataManagerUtils.parseStorageString(persistentData[i]);
        console.log(JSON.stringify(commuteEntry));
        entries.push(commuteEntry);
      }

      getCommute.get(entries);
    }
    else
    {
      Dispatcher.sendNextItem(getCommute, 'getcommute');
    }
  }
};

var PersistentMorningCommuteManager = {
  savedData : null,
  keyword : 'am_commute',
  loadCommute : function()
  {
    PersistentDataManagerUtils.loadData(PersistentMorningCommuteManager)
  },
  saveCommute : function()
  {
    PersistentDataManagerUtils.saveData(PersistentMorningCommuteManager);
  },
  isMorningCommute : function(route, direction, stopid, stopname)
  {
    return PersistentDataManagerUtils.isSaved(PersistentMorningCommuteManager, route, direction, stopid, stopname)
  },
  setMorningCommute : function(route, direction, stopid, stopname, isMorningCommute)
  {
    PersistentDataManagerUtils.setEntry(PersistentMorningCommuteManager, route, direction, stopid, stopname, isMorningCommute)
  }
};

var PersistentEveningCommuteManager = {
  savedData : null,
  keyword : 'pm_commute',
  loadCommute : function()
  {
    PersistentDataManagerUtils.loadData(PersistentEveningCommuteManager)
  },
  saveCommute : function()
  {
    PersistentDataManagerUtils.saveData(PersistentEveningCommuteManager);
  },
  isEveningCommute : function(route, direction, stopid, stopname)
  {
    return PersistentDataManagerUtils.isSaved(PersistentEveningCommuteManager, route, direction, stopid, stopname)
  },
  setEveningCommute : function(route, direction, stopid, stopname, isEveningCommute)
  {
    PersistentDataManagerUtils.setEntry(PersistentEveningCommuteManager, route, direction, stopid, stopname, isEveningCommute)
  }
};
