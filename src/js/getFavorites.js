
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
