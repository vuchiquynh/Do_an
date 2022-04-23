  
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <DHT.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h >
ESP8266WebServer webServer(80);
#define Topic1 "Iot/Led_status"
#define Topic2 "Iot/Data_setting"

#define ONE_WIRE_BUS 2         // BS18B20 Pin D4
#define UP 12 // D6
#define DOWN 13 // D7
#define EXIT 14 // D5
#define SET 15 // D8
#define RELAY 16 // D0
#define QUAT 10 //SD3
#define DHTPIN 0 //D3
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

char* ssid = "FPT Telecom-B506";
char* pass = "12346789";
#define mqtt_server "broker.hivemq.com"

const int lcdColumns = 16;
const int lcdRows = 2;

int addr = 0;
float Temp_Real;
unsigned int Time_Temp_h =0 ,Time_Temp_p =0,t=0,dem =0,Humi =10;
unsigned int Val_time =0,Time_h=0, Time_m = 0 , Time_Down = 0 ;
int Tmax =0 ;
String Data= "";
const uint16_t mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

byte degree[8] = {
  0B01110,
  0B01010,
  0B01110,
  0B00000,
  0B00000,
  0B00000,
  0B00000,
  0B00000
};
byte degree1[8] = {
  0B00000,
  0B00000,
  0B00000,
  0B00000,
  0B00000,
  0B01100,
  0B01100,
  0B00000
}; 

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  
Ticker blinker;

//-------------Hàm Connect Wifi------------
void Connect_Wifi(){
  WiFi.begin(ssid,pass);
  Serial.print("Connecting");
  while(WiFi.status()!=WL_CONNECTED){
   delay(500);
    Serial.print("...");
  }
  Serial.println(WiFi.localIP());
}
void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Da Nhan Duoc Du Lieu Topic:");
  Serial.println(topic);
  for (int i = 0; i < length; i++){
    Data += (char)payload[i];
  }
  Serial.println(Data);
  if(strstr(topic,Topic1)!= NULL){
    if(Data.toInt() == 1){
      Serial.println("Led On");
      digitalWrite(QUAT,0);
    }
    else{
      Serial.println("Led Off");
      digitalWrite(QUAT,1);      
    }
  }
  if(strstr(topic,Topic2)!= NULL){
    Serial.println("---------OK--------- ");
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, Data);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }
    Tmax = doc["temp_set"];
    Time_h = doc["time_h_set"];
    Time_m = doc["time_p_set"];
    Time_Down = (Time_h*60 + Time_m)*60;
    EEPROM.write(addr, Tmax);
    EEPROM.write(addr+2, Time_h);
    EEPROM.write(addr+3, Time_m); 
    EEPROM.commit(); 
    Serial.println("Gtri cai dat la: ");
    Serial.println(Tmax);
    Serial.println(Time_h);
    Serial.println(Time_m);
    Serial.println(Time_Down);
  }
  Data = "";
}

void reconnect()
{
  while (!client.connected()) // Chờ tới khi kết nối
  {
    // Thực hiện kết nối với mqtt user và pass
    //kết nối vào broker
    if (client.connect("ESP8266_id1", "ESP_offline", 0, 0, "ESP8266_id1_offline"))
    {
      Serial.println("Đã kết nối:");
      //đăng kí nhận dữ liệu từ topic Iot/Led
      client.subscribe(Topic1);
      client.subscribe(Topic2);
    }
    else
    {
      Serial.print("Lỗi:, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Đợi 5s
      delay(5000);
    }
  }
}

//-------------Ham Setup------------------
void setup(void) {
  Serial.begin(115200);
  Serial.println("Le quang phu");
  EEPROM.begin(512);  
  //pinMode(LED,OUTPUT);
  pinMode(UP,INPUT);
  pinMode(SET,INPUT);
  pinMode(DOWN,INPUT);
  pinMode(EXIT,INPUT);
  pinMode(RELAY,OUTPUT);
  pinMode(QUAT,OUTPUT);
  digitalWrite(QUAT,LOW);
  digitalWrite(RELAY,HIGH);
  blinker.attach(1, changeState);
  lcd.init();
  lcd.backlight();
  lcd.createChar(1, degree);
  lcd.createChar(2, degree1);
  sensors.begin(); // Bắt đầu đọc cảm biến
  dht.begin();
  
  Tmax = EEPROM.read(addr);
  Time_h = EEPROM.read(addr+2);
  Time_m = EEPROM.read(addr+3);
  Time_Down = (Time_h*60 + Time_m)*60;
  Connect_Wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
}
 
void loop(void) {
  
  if (!client.connected()){
    reconnect();
  }  
  client.loop();
  
  if(millis()-t >10) {
    Setting();
    dem++;
    if(dem == 200){
      dem =0;
      char  buffer[256];
      StaticJsonDocument<200> doc;
      Read_Ds18B20();
      Humi = dht.readHumidity();
      doc["temp"] = Temp_Real;
      doc["temp_set"] = Tmax;
      doc["Time_h"] = Time_Temp_h;
      doc["Time_p"] = Time_Temp_p;
      doc["humi"] = Humi;
      serializeJson(doc, buffer);
      client.publish("Iot/Data", buffer);
      
      Serial.println(Humi);
      Serial.println(Temp_Real);
      if(Time_Down ==0)
      {
        digitalWrite(RELAY,HIGH);
        digitalWrite(QUAT,HIGH);
        Serial.println("Tat Realy");
      }
      else
      {
         if(Temp_Real>Tmax)
          {
            digitalWrite(RELAY,HIGH);
            Serial.println("Tat Realy");
          }
         else if(Temp_Real<(Tmax-5))
         {
            Serial.println("Bat Realy");
            digitalWrite(RELAY,LOW);
         }        
      }

      Hien_Thi();
      }
     t=millis();
  }
}
//--------Ham hien thi 2 so-------------
void Show_Vale(unsigned char x, unsigned char y,unsigned int value)
{
  lcd.setCursor(x,y);
  lcd.print(value/10);
  lcd.print(value%10);
}

//----Ham thay doi gia tri x: cot, y: hang-----------
unsigned int change_value(unsigned char x, unsigned char y,  int value,  int value_min,  int value_max)
{
  while(1)
  {
    lcd.setCursor(x, y);
    lcd.print("  ");
    delay(50);
    //------Up ----------
    if(digitalRead(UP)== HIGH){
       delay(50);
       if (digitalRead(UP)== HIGH){
        value++;
       }
    }
    if(value > value_max)
    {
      value = value_min;
    }
    //----DOWN-----------
    if(digitalRead(DOWN)== HIGH){
       delay(50);
       if (digitalRead(DOWN)== HIGH){
        value--;
       }
    }
    if(value < value_min)
    {
      value = 99;
    }
    Show_Vale(x,y,value);
    delay(100);
    if(digitalRead(EXIT)== HIGH){
       delay(50);
       if(digitalRead(EXIT)== HIGH){
        delay(100);
        return value;
        break;
       }
    }
  }
}

//------------Ham Cai Dat Gtri-----------------
void Setting()
{
    if(digitalRead(SET)== HIGH){
      delay(50);
       if(digitalRead(SET)== HIGH){
        //-------Cai dat Tmax-----------
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("CAI DAT Tmax");
        Show_Vale(4,1,Tmax);
        lcd.write(1);
        lcd.print("C");    
        Tmax=change_value(4,1,Tmax,0,99);
        EEPROM.write(addr, Tmax);
        EEPROM.commit();
        Serial.println(Tmax);
        //------ Cai Dat Time--------------
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("CAI DAT TIME");
        Show_Vale(4,1,Time_h);
        lcd.print('h');
        Show_Vale(7,1,Time_m); 
        lcd.print("Phut");      
        Time_h=change_value(4,1,Time_h,0,12);// Cai dat gio
        Time_m=change_value(7,1,Time_m,0,60);// Cai dat phut
        EEPROM.write(addr+2, Time_h);
        EEPROM.write(addr+3, Time_m); 
        EEPROM.commit();       
        Time_Down = (Time_h*60 + Time_m)*60;
       }        
   }  
}
void Hien_Thi(){
      lcd.clear();
      //--- hien thi nhiet do cam bien-------
      lcd.setCursor(0, 0);
      lcd.print(Temp_Real);
      lcd.write(1);
      lcd.print("C");
      //----Hien thi thoi gian dem lui----------------------
      lcd.setCursor(9, 0);
      Time_Temp_h = Time_Down/3600;
      lcd.print(Time_Temp_h);
      lcd.write('h');
      Time_Temp_p = (Time_Down%3600)/60;
      lcd.print(Time_Temp_p);
      lcd.write('P');
      //--- hien thi nhiet do max------
      lcd.setCursor(0, 1);
      lcd.print(Tmax);
      lcd.write(1);
      lcd.print("C");
     //----Hien thi thoi gian cai dat----------------------
      lcd.setCursor(5, 1);
      lcd.print(Time_h);
      lcd.write('h');
      lcd.print(Time_m);
      lcd.write('P'); 
      //--- hien thi nhiet do max------
      lcd.setCursor(12, 1);
      lcd.print(Humi);
      lcd.print("%");
}
void Read_Ds18B20(){
      sensors.requestTemperatures(); 
      Temp_Real= sensors.getTempCByIndex(0);
}
//------Ham ngat timer--------------
void changeState()
{
  if(Time_Down > 0 )
  {
      Time_Down--;
      Serial.println(Time_Down);
  }   
}
