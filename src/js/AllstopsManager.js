
var AllstopsManager = {
    allstops : null,

    /* TODO: check cache instead of just downloaded */
    get : function(callback) 
    {
        /* check if instance var is loaded */
        if (AllstopsManager.allstops != null)
        {
            callback(AllstopsManager.allstops);
        }
        else
        {
            /* check for local storage */
            var storageAllstops = localStorage.getItem('allstops');
            if (storageAllstops != null)
            {
                console.log("found data locally!");
                AllstopsManager.allstops = JSON.parse(storageAllstops);
                callback(AllstopsManager.allstops);
            }
            else
            {
                /* grab from the web */
                console.log("about to call dropbox url...");
                var ALLSTOPS_URL = "https://www.dropbox.com/s/6kcvn4fseqb4agf/allstops.json?dl=1";
                URLUtils.sendRequest(ALLSTOPS_URL, function(response) 
                {
                    if (!!response)
                    {
                        console.log('dropbox response found!! ');
                        console.log('response: ' + response);
                        localStorage.setItem('allstops', response);
                        AllstopsManager.allstops = JSON.parse(response);
                        callback(AllstopsManager.allstops);
                    }
                    else
                    {
                        console.log("~ no data found!");
                        callback(null);
                    }
                });
            }
        }
    }
}
