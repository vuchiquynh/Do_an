#include <DHT.h>
#include <ArduinoJson.h>
#include "SSD1306Wire.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

SSD1306Wire display(0x3C, D1, D2);
#define DHTTYPE DHT11 // DHT 11
#define LED_BUILTIN D0
const int DHTPin = D4;

DHT dht(DHTPin, DHTTYPE);


//const char* ssid = "VCC";
//const char* password = "1234554321";
const char* ssid = "QuynhVC";
const char* password = "quynh199@";

const char* mqtt_server = "broker.hivemq.com";

WiFiClient quynhvc;
PubSubClient client(quynhvc);

unsigned long interval = 5000;
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
    
    String clientId = "quynhvc-";
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
char temp = 'on';
void callback(char* topic, byte* payload, unsigned int length) {
 Serial.print("nhan: ");
 Serial.println(topic);
 for(int i=0;i<length;i++){
  Serial.print((char) payload[i]);
 }
 if(char(payload[0]) == '1'){
  digitalWrite(LED_BUILTIN, 0);
  Serial.print("bat");
 }else {
    digitalWrite(LED_BUILTIN, 1);
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
  display.init();
  display.flipScreenVertically();// do nguoc theo chieu doc
  display.setContrast(255);
  
}

void loop() {
  
  StaticJsonDocument<200> doc;

  float nhietdo = dht.readTemperature();
  int doam = dht.readHumidity();
  
  doc["tem"] = nhietdo;
  doc["humi"] = doam;

  char  buffer[256];
  serializeJson(doc, buffer);

  
    int temperature = dht.readTemperature();
    int humidity = dht.readHumidity();
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_CENTER); /*Can le giua*/
    display.drawString(64,0 ,"~ Vu Chi Quynh ~");/*64, 0, <=> x=64, y=0: giua dong dau tien*/
  
    display.setLogBuffer(4, 30);/*Cap phat bo nho de hien thi 4 hang text va 30 ky tu moi hang.*/
    
    display.print("Temp: ");
    display.print(temperature);
    display.println(" °C");
    display.print("Hum: ");
    display.print(humidity);
    display.println(" %");
    display.drawLogBuffer(30,15);/*In ra tai vi tri x tinh tu trai sang,y tinh tu tren xuong*/
    display.display();

  if ((unsigned long)(millis() - previousMillis) >= interval) {
    previousMillis = millis();
    client.publish("quynhvc/data", buffer);
  }
  client.loop();

}
