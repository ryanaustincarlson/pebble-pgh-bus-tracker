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
