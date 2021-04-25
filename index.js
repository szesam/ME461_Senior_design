//Declare fs, http, express and socket io for graphing
var app = require('express')();
var http = require('http').Server(app);
var fs = require('fs')

function toogleDataSeries(e){
    if (typeof(e.dataSeries.visible) === "undefined" || e.dataSeries.visible) {
        e.dataSeries.visible = false;
    } else{
        e.dataSeries.visible = true;
    }
    chart.render();
}
// initialize data structures to store sensor data
var weight1=[], weight2 = [], weight3 = [], pressure = [];
//including serial port module 
const SerialPort = require('serialport');
const Readline = require('@serialport/parser-readline');
//create a new serial port connection to my port 
const port = new SerialPort('COM5',{baudRate: 115200});
//create a parser to read in the data 
const parser = port.pipe(new Readline({ delimiter: '\n'}));

var fs = require("fs");
var stream = fs.createWriteStream("out_with_40g.csv");
stream.write("FSR_weight1 (g), FSR_weight2 (g), FSR_weight3 (g), Pressure (PSI) \n")
//read each line of data from the open port
parser.on('data', data => {
    push_sensor_data(data);
    //all_data = data;
    console.log(data);
    stream.write(String(data));
});
// i is x-axis.
var i = 0;
// push_sensor_data is function in parse data. 
function push_sensor_data(data)
{
    data = data.split(",");
    // console.log(data[3]);
    i++
	weight1.push({
		x: parseInt(i),
		y: parseFloat(data[0])
	});
	weight2.push({
		x: parseInt(i),
		y: parseFloat(data[1])
	});
	weight3.push({
		x: parseInt(i),
		y: parseFloat(data[2])
	});
	pressure = data[3];
    if (weight1.length > 30) //number of data poitns visible at any time
    {
        weight1.shift();
        weight2.shift();
        weight3.shift();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// socket io script connect server to client (web browser) to display canvasjs charts.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// declare socket io for http
var io = require('socket.io')(http);
//interface with index.html directory to plot chart
app.get('/', function(req, res){
    res.sendFile(__dirname + '/index.html');
});
// chart format
  var chartOptions1 = {
	title:{
		text: "FSR weight in grams"
	},
	axisX: {
		Prefix: "Seconds",
		title: "Time(sec)",
		interval: 1
	},
	axisY: {
		title: "Weight",
		lineColor: "#7F6084",
		tickColor: "#7F6084",
		labelFontColor: "#7F6084",
		titleFontColor: "#7F6084",
		includeZero: true,
		suffix: "g",
	},
	legend: {
		cursor: "pointer",
		verticalAlign: "bottom",
		horizontalAlign: "center",
		dockInsidePlotArea: true,
	},
	toolTip: {
		shared: true
	},
	data: [
	{
		name: "FSR1",
		type: "spline",
		color: "#7F6084",
		yValueFormatString: "0.##m",
		showInLegend: true,
		dataPoints: weight1,
	},
	{
		name: "FSR2",
		type: "spline",
		markerType: "cross",
		color: "#ff1a1a",
		yValueFormatString: "0.##m",
		showInLegend: true,
		dataPoints: weight2,
	},
    {
		name: "FSR3",
		type: "spline",
		markerType: "triangle",
		color: "#369EAD",
		yValueFormatString: "0.##m",
		showInLegend: true,
		dataPoints: weight3,
	}]

};
// emit data every 1second
setInterval(function(){
	io.emit('dataMsg1', chartOptions1);
	io.emit('dataMsg3', pressure);
 }, 1000);

//socket io creation. 
io.on('connection', function(socket){
	console.log('a user connected');
	io.emit('dataMsg1', chartOptions1);
	io.emit('dataMsg3', pressure);
});

// keep channel open.
http.listen(3001, function(){
});
