var g_jsonData="";

//Check for the various File API support
function checkFileAPI() 
{
    if (window.File && window.FileReader && window.FileList && window.Blob) 
    {             
        return true;
    } 
    else 
    {
        alert('The File APIs are not fully supported by your browser. Fallback required.');
        return false;
    }
}      

//read text input         
/*function readText(filePath) 
{            
    var file = filePath.files[0];
    var reader = new FileReader();
    var output = ""; //placeholder for text output
    if (filePath.files && filePath.files[0]) 
    {
        reader.onload = function (e) 
        {
            // By lines
            var lines = e.target.result.split('\n');
            for(var line = 0; line < lines.length; line++)
            {
                alert(lines[line]);
            };
        };//end onload()
        reader.readAsText(file);
    }//end if html5 filelist support
    else 
    { //this is where you could fallback to Java Applet, Flash or similar
        //alert('ERROR');
        return false;
    }
    return true;
}*/

function drawChart() 
{
    var jsonPrefix = "{ \"cols\": [{\"label\":\"Time\",\"type\":\"number\"},{\"label\":\"Temperature\",\"type\":\"number\"},{\"label\":\"Setpoint\",\"type\":\"number\"},";
    jsonPrefix = jsonPrefix + "{\"label\":\"Power\",\"type\":\"number\"}],\"rows\": [";
    g_jsonData = jsonPrefix + g_jsonData;
    //var jsonData = "{\"c\":[{\"v\":\"7\"},{\"v\":60},{\"v\":134},{\"v\":100}]}]}";
    //var jsonData = "{\"c\":[{\"v\":\"7\"},{\"v\":60},{\"v\":134},{\"v\":100}]},{\"c\":[{\"v\":\"8\"},{\"v\":60},{\"v\":134},{\"v\":100}]},{\"c\":[{\"v\":\"9\"},{\"v\":65},{\"v\":134},{\"v\":100}]}]}";                        
                
    //jsonData = jsonPrefix + jsonData;
    //alert(jsonData);
    var data = new google.visualization.DataTable(g_jsonData);

    // Create a dashboard.
    var dashboard = new google.visualization.Dashboard(
    document.getElementById('dashboard_div'));

    var today = new Date();

    var control = new google.visualization.ControlWrapper({
        'controlType': 'ChartRangeFilter',
        'containerId': 'control_div',
        'options': {
                // Filter by the date axis.
                'filterColumnIndex': 0,
                'ui': {
                        'chartType': 'LineChart',
                        'chartOptions': {
                                'chartArea': {'width': '80%'},
                                'hAxis': {'baselineColor': 'none'}
                        },
                        // Display a single series that shows the total bug count
                        // Thus, this view has two columns: the date (axis) and the count (line series).
                        'chartView': {'columns': [0, 1]},
                        // 1 day in milliseconds = 24 * 60 * 60 * 1000 = 86,400,000
                        // 1 minute in milliseconds = 1000 * 60
                        'minRangeSize': 60000
                }
        }
        // Initial range: half hour ago
        //'state': {'range': {'start': new Date(today.getTime()-1000*60*60*24), 'end': today}}
    });

    var chart = new google.visualization.ChartWrapper({
            'chartType': 'ComboChart',
            'containerId': 'chart_div',
            'options': {
                    // Use the same chart area width as the control for axis alignment.
                    'chartArea': {'height': '80%', 'width': '80%'},
                    //'hAxis': {'slantedText': false,'maxAlternation':1},
                    //'vAxis': {'viewWindow': {'min': 0}},
                    'legend': {'position': 'in'},
                    'title':'Chuck\'s Sous-Vide Temperature Log',
                    'hAxis':{'title': 'Minutes'},
                    'vAxis':{'title': 'Fahrenheit/PWM duty cycle'}
            },
    });

    dashboard.bind(control, chart);
    dashboard.draw(data);
}

function handleFileSelect(event) 
{
    if (checkFileAPI() == false)
    {
        return;
    }
    
    var file = event.target.files[0];
    
    var reader = new FileReader();
    
    reader.onload = function(event) {
        var contents = event.target.result;
        // By lines
        var lines = contents.split('\n');
        for(var i = 0; i < lines.length; i++)
        {
            //Check if there's any value
            if (lines[i])
            {
                if (lines[i].length > 0)
                {                    
                    if (i > 0) g_jsonData = g_jsonData + ",";
                    var data = lines[i].split(',');  
                    g_jsonData = g_jsonData + "{\"c\":[{\"v\":\"" +data[0] +"\"},{\"v\":" +data[1] +"},{\"v\":" +data[2] +"},{\"v\":" +data[3] +"}]}";
                }
            }
        }
        
        g_jsonData = g_jsonData + "]}";
        
        //alert(g_jsonData);
    };

    reader.onerror = function(event) {
        alert("File could not be read! Code " + event.target.error.code);
    };

    reader.readAsText(file);
        
    // Load the Visualization API and the piechart package.
    //Set a callback to run when the Google Visualization API is loaded.
    google.load('visualization', '1.0', {'packages':['controls'], 'callback': drawChart});
    
    /* getData calls an external url to populate and return a json object
    suitable for google charts as a dataTable.  Validate the incoming json
    text with a site like http://jsonlint.com/.

    The first column should be the Time.
    The second column should be the total number of bugs
    Subsequent columns can be whatever you want (optional)

    The format should look like:
            {
            "cols": [
                    {"label":"Time","type":"number"},
                    {"label":"Temperature","type":"number"},
                    {"label":"Setpoint","type":"number"},
                    {"label":"Power","type":"number"}
                    ],
            "rows": [
                    {"c":[{"v":"7"},{"v":60},{"v":134},{"v":100}]},
                    {"c":[{"v":"8"},{"v":60},{"v":134},{"v":100}]},
                    {"c":[{"v":"9"},{"v":65},{"v":134},{"v":100}]}
                    ]
            }*/
}