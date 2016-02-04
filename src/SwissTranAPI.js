var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function getNextDepartures(id){
  var maxprediction = 10;
  var nextDepartures = '';
  var url = 'http://transport.opendata.ch/v1/stationboard?id=' + id + '&limit=' + maxprediction;
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      //console.log(JSON.stringify(json));
      
      if(json.station !== null && json.stationboard !==null){
        var stationName = json.station.name.split(" ")[1];
        //console.log('Station ' + stationName);
        nextDepartures = stationName + ';';//line:direction:time;
        for (var i = 0; i < maxprediction ; i++){
          //console.log(json.stationboard[i].number + ':' + json.stationboard[i].passList[0].arrival);
          //json.stationboard[i].number
          //json.stationboard[i].passList[0].arrivalTimestamp
          //json.stationboard[i].to.split(" ")
          nextDepartures = nextDepartures + json.stationboard[i].number +':';
          nextDepartures = nextDepartures + json.stationboard[i].to.split(',')[1] + ':';//do not split the spaces to avoid the beginning space. made in departure_view
          nextDepartures = nextDepartures + json.stationboard[i].passList[0].arrivalTimestamp;
          nextDepartures = nextDepartures + ';';
        }
        nextDepartures = nextDepartures + '%';
        console.log(nextDepartures);
      }
      
      var dictionary = {
        'KEY_DEPARTURES': nextDepartures,
      };
      
      Pebble.sendAppMessage(dictionary,
      function(e) {
        console.log('Info sent to pebble');
      },
      function(e) {
        console.log('Error sending info to Pebble!');
      }
);
      
    }          
  );
}

function locationSuccess(pos) {
  var station = '';
  var id = '';
  //var url = 'http://transport.opendata.ch/v1/locations?x=46.202988&y=6.174194&type=station';
  var url = 'http://transport.opendata.ch/v1/locations?x=' + pos.coords.latitude + '&y='+ pos.coords.longitude +'&type=station';
  console.log('url: '+ url);
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      //console.log(json);
      
      if(json.stations !== null){
        for(var i =0 ; i < json.stations.length; i++){
          if(json.stations[i].name.split(",")[1]!==undefined){
            station = station + json.stations[i].name.split(",")[1];
            station = station + ':';
            id = id + json.stations[i].id;
            id = id + ':';
          }
        }
        console.log('Station ' + id);
      }
      var dictionary = {
        'KEY_STATIONS': station,
        'KEY_IDS': id,
      };
      
      Pebble.sendAppMessage(dictionary,
      function(e) {
        console.log('Info sent to pebble');
      },
      function(e) {
        console.log('Error sending info to Pebble!');
      }
);
      
    }          
  );
  
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getStations() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}



// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    //console.log('PebbleKit JS ready!');
    getStations();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    //console.log('AppMessage received: ' + JSON.stringify(e.payload));
    var recieved = e.payload;
    var key = Object.keys(recieved)[0];
    console.log(key);
    if(key == 'KEY_SEND_ID' || key == 3){
      getNextDepartures(recieved.KEY_SEND_ID);
    }
    //getStations();
  }                     
);