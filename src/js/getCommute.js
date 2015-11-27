
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
    var stopids = [];
    for (var i=0; i<entries.length; i++)
    {
      stopids.push(entries.stopid);
    }
    var params = {
      // 'rt' : route,
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
      var title = '#' + route + ' to ' + destination;
      return title;
    }, function(prediction) {
      return 'foo'; // dunno what to do here...
    });
  },
  handleRequest : function(should_init, route, direction, stopid, stopname)
  {
    if (should_init)
    {
      getCommute.savedData = null;

      // TODO: check the time and choose the commute type
      // let's just do evening now...
      if (PersistentEveningCommuteManager.savedData == null)
      {
        PersistentEveningCommuteManager.loadEveningCommute();
      }

      var sep = PersistentDataManagerUtils.separator;
      var persistentData = PersistentEveningCommuteManager.savedData;
      var entries = [];
      for (var i=0; i<persistentData.length; i++)
      {
        var commuteEntry = PersistentDataManagerUtils.parseStorageString(persistentData[i]);
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
  loadMorningCommute : function()
  {
    PersistentDataManagerUtils.loadData(PersistentMorningCommuteManager)
  },
  saveMorningCommute : function()
  {
    PersistentDataManagerUtils.saveData(PersistentMorningCommuteManager);
  },
  isMorningCommute : function(route, direction, stopid, stopname)
  {
    console.log("requesting AM commute....");
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
  loadEveningCommute : function()
  {
    PersistentDataManagerUtils.loadData(PersistentEveningCommuteManager)
  },
  saveEveningCommute : function()
  {
    PersistentDataManagerUtils.saveData(PersistentEveningCommuteManager);
  },
  isEveningCommute : function(route, direction, stopid, stopname)
  {
    console.log("requesting PM commute....");
    return PersistentDataManagerUtils.isSaved(PersistentEveningCommuteManager, route, direction, stopid, stopname)
  },
  setEveningCommute : function(route, direction, stopid, stopname, isEveningCommute)
  {
    PersistentDataManagerUtils.setEntry(PersistentEveningCommuteManager, route, direction, stopid, stopname, isEveningCommute)
  }
};
