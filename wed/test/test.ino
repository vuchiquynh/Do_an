#include <Sensor_DHT.h>
#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define LED_BUILTIN 14
#define DHTTYPE DHT11 // DHT 11
const int DHTPin = 3;

DHT dht(DHTPin, DHTTYPE);


const char* ssid = "phong210000";
const char* password = "phong210";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient mangcb;
PubSubClient client(mangcb);

unsigned long interval = 1000;
unsigned long previousMillis = 0;


void setup_wifi() {

  delay(100);
  
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // loop toi khi ket noi toi mqtt
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    String clientId = "mangcb-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(2000);
    }
  }
}


// test json 
//char temp = 'on';
//void callback(char* topic, byte* payload, unsigned int length) {
//
// if(length == 2){
//  digitalWrite(LED_BUILTIN, LOW);
// }else if(length == 3){
//    digitalWrite(LED_BUILTIN, HIGH);
// }
// 
//}
char temp = 'on';
void callback(char* topic, byte* payload, unsigned int length) {
 Serial.print("nhan: ");
 Serial.println(topic);
 for(int i=0;i<length;i++){
  Serial.print((char) payload[i]);
 }
 if(char(payload[0]) == '1'){
  digitalWrite(LED_BUILTIN, 1);
  Serial.print("bat");
 }else {
    digitalWrite(LED_BUILTIN, 0);
    Serial.print("tat");
 }
 
}


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  reconnect();

  client.subscribe("quynhvc/led_status");

  dht.begin();
  
}

void loop() {
  // tést json
  StaticJsonDocument<200> doc;

  int nhietdo = dht.readTemperature();
  int doam = dht.readHumidity();
  
  doc["temp"] = nhietdo;
  doc["humi"] = doam;

  char  buffer[256];
  serializeJson(doc, buffer);

  
  
  //if (!client.connected()) {
    //reconnect();
  //}
  //client.subscribe("mangcb/led_status");
  //client.publish("mangcb/data", buffer);
  //delay(5000);
  //client.loop();

  if ((unsigned long)(millis() - previousMillis) >= interval) {
    previousMillis = millis();
    client.publish("haianh/data", buffer);
  }
  client.loop();

}
