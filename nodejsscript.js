var sys = require('util');
var fs = require('fs');
var mqtt = require('mqtt');

var filepath = 'C:/JavaScript/NodeJS/vince/mqtt-logs/log.csv'
        
//Connect to MQTT broker
var client = mqtt.connect({ host: 'localhost', port: 1883, keepalive: 3000});
 
client.on('connect', function() {
    console.log('Connected to mqtt broker')
    client.subscribe('data')
    client.subscribe('stdout')
    console.log('Client subscribed to topics: #data #stdout')
    
    //Time, Temperature, Setpoint, Power
    var message = "9.5,23.0,30,22"
    handleData(message)
})

client.on('message', function (topic, message) {
    console.log('Received message: #%s %s ', topic, message);
    switch (topic) {
    case 'data':
        return handleData(message)
    case 'stdout':
        return handleStdout(message)        
    }
})

function handleData (message) {  
  console.log('handleData')
  
  //var array = message.split(',');
  
  //var jsonmsg = JSON.stringify(message);
  
  
  /*message.pipe(csv2json({
    // Defaults to comma. 
    separator: ';'
  }))
  .pipe(jsonmsg);*/
  
  //console.log(jsonmsg)
  fs.appendFile(filepath, message+"\n", function(err) {
    if (err) throw err;
    });  
}

function handleStdout (message) {  
  console.log('handleStdout')
  fs.unlink(filepath, function(err) {
    if (err) throw err;
    });  
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
    /*client.publish('garage/connected', 'false')*/
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
