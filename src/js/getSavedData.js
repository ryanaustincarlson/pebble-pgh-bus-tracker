var getSavedData = {
  handleRequest : function(route, direction, stopid, stopname)
  {
    var isFavorite = PersistentFavoritesManager.isFavorite(route, direction, stopid, stopname);
    var isAMCommute = PersistentMorningCommuteManager.isMorningCommute(route, direction, stopid, stopname);
    var isPMCommute = PersistentEveningCommuteManager.isEveningCommute(route, direction, stopid, stopname);

    var dictionary = {
      "KEY_IS_FAVORITE" : isFavorite ? 1 : 0,
      "KEY_IS_MORNING_COMMUTE" : isAMCommute ? 1 : 0,
      "KEY_IS_EVENING_COMMUTE" : isPMCommute ? 1 : 0
    };

    Pebble.sendAppMessage(dictionary,
      function(e) {
        console.log('getSavedData success!');
      },
      function(e) {
        console.log("Error sending saved data to pebble :(");
      }
    );

    console.log('route: ' + route + ', dir: ' + direction + ', stopid: ' + stopid + ', stopname: ' + stopname);
    console.log('fav? ' + isFavorite + ', AM? ' + isAMCommute + ', PM? ' + isPMCommute);
  }
}
