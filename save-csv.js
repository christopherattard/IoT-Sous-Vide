var sys = require('util');
var fs = require('fs');
var mqtt = require('mqtt');

var g_filepath = './../mqtt-logs/';
        
//Connect to MQTT broker
var client = mqtt.connect({ host: 'localhost', port: 1883, keepalive: 3000});
 
client.on('connect', function() {
    console.log('Connected to mqtt broker')
    client.subscribe('data')    
    console.log('Client subscribed to topic: #data')
    
    startNewLogFile()
    
    //To test 
    handleData("24,5,6,30")    
})

client.on('message', function (topic, message) {
    console.log('Received message: #%s %s ', topic, message);
    switch (topic) {
    case 'data':
        return handleData(message)            
    }
})

function handleData (message) {  
  console.log('handleData')
  
  //Time, Temperature, Setpoint, Power  
  fs.appendFile(g_filepath, message+"\n", function(err) {
    if (err) throw err;
    });  
}

function startNewLogFile() {  
  var now = new Date()  
  var date = [now.getFullYear(), now.getMonth()+1, now.getDate(), now.getHours(), now.getMinutes(), now.getSeconds()]
  var suffix = date.join("")
  g_filepath = g_filepath + "log_" + suffix + ".csv"
  console.log('Starting new log file: '+g_filepath)   
}



/**
 * Want to notify controller that garage is disconnected before shutting down
 */
function handleAppExit (options, err) {  
  if (err) {
    console.log(err.stack)
  }

  if (options.cleanup) {
    console.log('cleanup')    
  }

  if (options.exit) {
    process.exit()
  }
}

/**
 * Handle the different ways an application can shutdown
 */
process.on('exit', handleAppExit.bind(null, {  
  cleanup: true
}))
process.on('SIGINT', handleAppExit.bind(null, {  
  exit: true
}))
process.on('uncaughtException', handleAppExit.bind(null, {  
  exit: true
}))


/*process.on('uncaughtException', function (error) {
    console.log(error.stack);
});

client.on('connack', function(packet) {
        
    });

client.end();*/

    /*client.connect({keepalive: 3000});*/

    /*client.on('connack', function(packet) {
        client.subscribe({topic: 'data', qos: 0})
        client.subscribe({topic: 'stdout', qos: 0})
    });

    client.on('publish', function(packet) {
        sys.puts(packet.topic+" : "+packet.payload);
        if (packet.topic == 'data') {
            fs.appendFile('/mqtt-logs/log.csv', packet.payload+"\n", function(err) {
                if (err) throw err;
            });
        };
        if (packet.topic == 'stdout' && packet.payload == 'restart') {
            sys.puts('Deleting /mqtt-logs/log.csv');
            fs.unlink('/mqtt-logs/log.csv', function(err) {
                if (err) throw err;
            });
        };
    });*/
