var PersistentDataManagerUtils = {
  separator : "_@_",
  loadData : function(dataManager)
  {
    var jsonData = localStorage.getItem(dataManager.keyword);
    console.log(jsonData);
    if (jsonData == null)
    {
      dataManager.savedData = [];
    }
    else
    {
      dataManager.savedData = JSON.parse(jsonData);
    }

    console.log('persistent loaded... ' + JSON.stringify(dataManager.savedData));
  },
  saveData : function(dataManager)
  {
    if (dataManager.savedData != null)
    {
      var jsonData = JSON.stringify(dataManager.savedData);
      localStorage.setItem(dataManager.keyword, jsonData);
      console.log('persistent saved... ' + jsonData);
    }
  },
  getStorageString : function(route, direction, stopid, stopname)
  {
    var sep = PersistentDataManagerUtils.separator;
    var item = route + sep + direction + sep + stopid + sep + stopname;
    return item;
  },
  parseStorageString : function(storageString)
  {
    var sep = PersistentDataManagerUtils.separator;
    var split = storageString.split(sep);
    var item = {
      route : split[0],
      direction : split[1],
      stopid : split[2],
      stopname : split[3]
    };
    return item;
  },
  isSaved : function(dataManager, route, direction, stopid, stopname)
  {
    console.log("requesting " + dataManager.keyword + "....");
    if (dataManager.savedData == null)
    {
      PersistentDataManagerUtils.loadData(dataManager);
    }

    var item = PersistentDataManagerUtils.getStorageString(route, direction, stopid, stopname);
    var issaved = dataManager.savedData.indexOf(item) >= 0;
    console.log(item + ' is saved as ' + dataManager.keyword + '? ' + issaved);
    return issaved;
  },
  setEntry : function(dataManager, route, direction, stopid, stopname, shouldAdd)
  {
    /* shouldAdd = True if item should be added, False if item should be removed */
    console.log('received set ' + dataManager.keyword + ' request rt: ' +
    route + ', direction: ' + direction + ', stopid: ' +
    stopid + ', stopname: ' + stopname + ', shouldAdd: ' + shouldAdd);

    if (dataManager.savedData == null)
    {
      PersistentDataManagerUtils.loadData(dataManager);
    }

    // use localStorage
    var item = PersistentDataManagerUtils.getStorageString(route, direction, stopid, stopname);
    console.log('new item: ' + item);

    var changedState = false;
    if (shouldAdd)
    {
      /* only add if item's not already there */
      if (dataManager.savedData.indexOf(item) == -1)
      {
        dataManager.savedData.push(item);
        changedState = true;
      }
    }
    else
    {
      var index = dataManager.savedData.indexOf(item);
      if (index >= 0)
      {
        dataManager.savedData.splice(index, 1);
        changedState = true;
      }
    }

    if (changedState)
    {
      PersistentDataManagerUtils.saveData(dataManager);
    }
  }
};
