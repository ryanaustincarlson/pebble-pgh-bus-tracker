
var getFavorites = {
  separator : "_@_",
  savedData : null,
  sendNextFavorite : function()
  {
    Dispatcher.sendNextItem(getFavorites, "getfavorites");
  },
  get : function()
  {
    if (PersistentFavoritesManager.savedData == null)
    {
      PersistentFavoritesManager.loadFavorites();
    }

    var sep = getFavorites.separator;

    var favs = PersistentFavoritesManager.savedData;
    var titles = [];
    var subtitles = [];
    var selectors = [];
    for (var i=0; i<favs.length; i++)
    {
      var fav = PersistentDataManagerUtils.parseStorageString(favs[i]);

      var title = fav.stopname;
      var subtitle = fav.route + ' - ' + fav.direction;
      var selector = fav.route + sep + fav.direction + sep + fav.stopid + sep + fav.stopname;

      titles.push(title);
      subtitles.push(subtitle);
      selectors.push(selector);
    }

    getFavorites.savedData = {
      titles : titles,
      subtitles : subtitles,
      selectors : selectors,
      index : 0
    };

    Dispatcher.sendMenuSetupMessage(getFavorites, "getfavorites");
  },
  handleRequest : function(should_init)
  {
    if (should_init)
    {
      getFavorites.savedData = null;
      getFavorites.get();
    }
    else
    {
      Dispatcher.sendNextItem(getFavorites, "getfavorites");
    }
  }
}

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
    console.log("requesting favorites....");
    if (dataManager.savedData == null)
    {
      PersistentDataManagerUtils.loadFavorites(dataManager);
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
      PersistentDataManagerUtils.loadFavorites(dataManager);
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
        changedFavorites = true;
      }
    }

    if (changedState)
    {
      PersistentDataManagerUtils.saveData(dataManager);
    }
  }
};

var PersistentFavoritesManager = {
  savedData : null,
  keyword : 'favorites',
  loadFavorites : function()
  {
    PersistentDataManagerUtils.loadData(PersistentFavoritesManager)
  },
  saveFavorites : function()
  {
    PersistentDataManagerUtils.saveData(PersistentFavoritesManager);
  },
  isFavorite : function(route, direction, stopid, stopname)
  {
    console.log("requesting favorites....");
    return PersistentDataManagerUtils.isSaved(PersistentFavoritesManager, route, direction, stopid, stopname)
  },
  setFavorite : function(route, direction, stopid, stopname, isfavorite)
  {
    PersistentDataManagerUtils.setEntry(PersistentFavoritesManager, route, direction, stopid, stopname, isfavorite)
  }
};


// /// old
// var PersistentFavoritesManager = {
//   separator : "_@_",
//   favorites : null,
//   loadFavorites : function()
//   {
//     var jsonfavs = localStorage.getItem('favorites');
//     if (jsonfavs == null)
//     {
//       PersistentFavoritesManager.favorites = [];
//     }
//     else
//     {
//       PersistentFavoritesManager.favorites = JSON.parse(jsonfavs);
//     }
//
//     console.log('persistent loaded... ' + JSON.stringify(PersistentFavoritesManager.favorites));
//   },
//   saveFavorites : function()
//   {
//     if (PersistentFavoritesManager.favorites != null)
//     {
//       var jsonfavs = JSON.stringify(PersistentFavoritesManager.favorites);
//       localStorage.setItem('favorites', jsonfavs);
//       console.log('persistent saved... ' + jsonfavs);
//     }
//   },
//   getStorageString : function(route, direction, stopid, stopname)
//   {
//     var sep = PersistentFavoritesManager.separator;
//     var item = route + sep + direction + sep + stopid + sep + stopname;
//     return item;
//   },
//   parseStorageString : function(storageString)
//   {
//     var sep = PersistentFavoritesManager.separator;
//     var split = storageString.split(sep);
//     var item = {
//       route : split[0],
//       direction : split[1],
//       stopid : split[2],
//       stopname : split[3]
//     };
//     return item;
//   },
//   isFavorite : function(route, direction, stopid, stopname)
//   {
//     console.log("requesting favorites....");
//     if (PersistentFavoritesManager.favorites == null)
//     {
//       PersistentFavoritesManager.loadFavorites();
//     }
//
//     var item = PersistentFavoritesManager.getStorageString(route, direction, stopid, stopname);
//     var isfavorite = PersistentFavoritesManager.favorites.indexOf(item) >= 0;
//     console.log(item + ' is favorite? ' + isfavorite);
//     return isfavorite;
//   },
//   setFavorite : function(route, direction, stopid, stopname, isfavorite)
//   {
//     console.log('received set fav request rt: ' +
//     route + ', direction: ' + direction + ', stopid: ' +
//     stopid + ', stopname: ' + stopname + ', isfav: ' + isfavorite);
//
//     if (PersistentFavoritesManager.favorites == null)
//     {
//       PersistentFavoritesManager.loadFavorites();
//     }
//
//     // use localStorage
//     var item = PersistentFavoritesManager.getStorageString(route, direction, stopid, stopname);
//     console.log('new item: ' + item);
//
//     var changedFavorites = false;
//     if (isfavorite)
//     {
//       /* only add if item's not already there */
//       if (PersistentFavoritesManager.favorites.indexOf(item) == -1)
//       {
//         PersistentFavoritesManager.favorites.push(item);
//         changedFavorites = true;
//       }
//     }
//     else
//     {
//       var index = PersistentFavoritesManager.favorites.indexOf(item);
//       if (index >= 0)
//       {
//         PersistentFavoritesManager.favorites.splice(index, 1);
//         changedFavorites = true;
//       }
//     }
//
//     if (changedFavorites)
//     {
//       PersistentFavoritesManager.saveFavorites();
//     }
//   }
// };
