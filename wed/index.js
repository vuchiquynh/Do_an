
var express = require("express");
var app = express();
app.use(express.static("public"));
app.set("view engine", "ejs");
app.set("views", "./views");

var server = require("http").Server(app);
var io = require("socket.io")(server);
var Chart = require("chart.js");
const { connect } = require("mqtt");
var mysql = require("mysql2");

// su dung mqtt
var mqtt = require("mqtt");
var client = mqtt.connect("mqtt://broker.hivemq.com:1883");


io.on("connection", function (socket) {

  //nhan led
  socket.on("led_status", function (data) {
    console.log(data);
    client.publish("Iot/Led_status", data);
  });
  socket.on("Data_setting", function (data) {
    console.log("[Socket][On]", data.temp_set, data.time_h_set,data.time_p_set );
    client.publish("Iot/Data_setting", 
      JSON.stringify({"temp_set": data.temp_set, "time_h_set": data.time_h_set,"time_p_set": data.time_p_set}));
  });
  socket.on("yeucau", function (data) {
    updateStatus();
  });
});



//ket noi mysql
const connection = mysql.createConnection({
  host: 'localhost',
  user: 'root',
  password: '123456',
  database: 'mcb'
});

// bao ket noi thanh cong
connection.connect(function (err) {

  if (err) throw err;
  console.log("sql connected");

});

var data_js = "";
var data_chart_js = "";
function updateStatus() {
  connection.query("select * from sensor order by id desc limit 1", function (err, results, fields) {
    console.log(results);
    data_js = JSON.stringify(results);
    io.sockets.emit("data_sensors", data_js);
  });
}


server.listen(5000);
app.get("/", function (req, res) {
  res.render("home");
});