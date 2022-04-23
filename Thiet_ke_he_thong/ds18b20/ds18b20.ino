
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <ESP8266WebServer.h>
ESP8266WebServer webServer(80);
#include "Web_server.h"

#define ONE_WIRE_BUS 0         // BS18B20 Pin D1
#define LED 2 // Led MCU
#define UP 12 // D6
#define DOWN 13 // D7
#define EXIT 14 // D5
#define SET 16 // D0
#define RELAY 15 // D8

char* ssid = "QuynhVC";
char* pass = "quynh199@";

const int lcdColumns = 16;
const int lcdRows = 2;

int addr = 0;
float Temp_Real;
unsigned int Time_Temp_h =0 , Time_Temp_p =0,t=0,dem =0;
unsigned int Val_time =0,  Tmax_N =0 , Tmax_Tp =0 , Time_h=0, Time_m = 0 , Time_Down = 0 ;
float Tmax =0.0 ;
String DataJson;

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
  Serial.begin(9600);
  Serial.print("Connecting");
  while(WiFi.status()!=WL_CONNECTED){
   delay(500);
    Serial.print("...");
  }
  Serial.println(WiFi.localIP());
}
//-------Ham Gưi web lên PC------------
void Send_Webpage(){
  webServer.send(200,"text/html",Webs);
}
//-------Ham gui data lêm web----------
void Send_Data(){
  String d1;
  if(digitalRead(RELAY)==0){
    d1 = "OFF";
  }else{
    d1 = "ON";
  }
  DataJson = "{\"RELAY\": \""+ String(d1) +"\","+
              "\"Temp_Real\": \""+ String(Temp_Real) +"\","+
              "\"Temp_Setup\": \""+  String(Tmax) +"\","+
              "\"Time_Setup_h\": \""+  String(Time_h) +"\","+
              "\"Time_Setup_m\": \""+  String(Time_m) +"\","+
              "\"Time_Real_h\": \""+  String(Time_Temp_h) +"\","+
              "\"Time_Real_m\": \""+String(Time_Temp_p) +"\"}";  
  webServer.send(200,"application/json",DataJson);
  webServer.on("/",Send_Webpage);
}
//---------Ham nhan Nhiet do cai dat tu web-----------
void Recev_Value_Temp(){
  // Ham nhan du lieu tu web
  Tmax= webServer.arg("namex").toFloat();
  webServer.on("/",Send_Webpage);
  
   
}
//------Ham nhan thoi gian giờ --------------
void Recev_Value_Time_H(){
  Time_h= webServer.arg("namex1").toInt();
  EEPROM.write(addr+2, Time_h);     
  Time_Down = (Time_h*60 + Time_m)*60;
  webServer.on("/",Send_Webpage);
}
//-------Ham nhan thoi gian phut-----------
void Recev_Value_Time_M(){ 
  Time_m= webServer.arg("namex2").toInt();
  EEPROM.write(addr+3, Time_m);
  Time_Down = (Time_h*60 + Time_m)*60;
  webServer.on("/",Send_Webpage);
}
//-------------Ham Setup------------------
void setup(void) {
  EEPROM.begin(512);  
  pinMode(LED,OUTPUT);
  pinMode(UP,INPUT);
  pinMode(SET,INPUT);
  pinMode(DOWN,INPUT);
  pinMode(EXIT,INPUT);
  pinMode(RELAY,OUTPUT);
  Serial.println(digitalRead(SET));
  blinker.attach(1, changeState);
  lcd.init();
  lcd.backlight();
  lcd.createChar(1, degree);
  lcd.createChar(2, degree1);
  sensors.begin(); // Bắt đầu đọc cảm biến
  Tmax_N = EEPROM.read(addr);
  Tmax_Tp = EEPROM.read(addr+1);
  Time_h = EEPROM.read(addr+2);
  Time_m = EEPROM.read(addr+3);
  Tmax = ((Tmax_N*100)+Tmax_Tp )/100.0;// Cai dat Tmax
  Time_Down = (Time_h*60 + Time_m)*60;
  Connect_Wifi();
  webServer.on("/",Send_Webpage);
  webServer.on("/Load_Data",Send_Data);
  webServer.on("/x",Recev_Value_Temp);
  webServer.on("/x1",Recev_Value_Time_H);
  webServer.on("/x2",Recev_Value_Time_M);
  webServer.begin();
}
 
void loop(void) {
  
  webServer.handleClient();
  if(millis()-t >100) {
    Setting();
    dem++;
    if(dem=10){
      dem =0;
      Read_Ds18B20();
      if(Temp_Real<Tmax)
      {
        digitalWrite(RELAY,HIGH);
      }
      else
      {
        digitalWrite(RELAY,LOW);
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
    webServer.handleClient();
    lcd.setCursor(x, y);
    lcd.print("  ");
    delay(100);
    //------Up ----------
    if(digitalRead(UP)== LOW){
       delay(10);
       if (digitalRead(UP)== LOW){
        value++;
       }
    }
    if(value > value_max)
    {
      value = value_min;
    }
    //----DOWN-----------
    if(digitalRead(DOWN)== LOW){
       delay(10);
       if (digitalRead(DOWN)== LOW){
        value--;
       }
    }
    if(value < value_min)
    {
      value = 99;
    }
    Show_Vale(x,y,value);
    delay(150);
    if(digitalRead(EXIT)== LOW){
       delay(10);
       if(digitalRead(EXIT)== LOW){
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
    if(digitalRead(SET)== LOW){
      delay(10);
       if(digitalRead(SET)== LOW){
        //-------Cai dat Tmax-----------
        lcd.clear();
        lcd.setCursor(2,0);
        lcd.print("CAI DAT Tmax");
        Show_Vale(4,1,Tmax_N);
        lcd.write(2);
        Show_Vale(7,1,Tmax_Tp); 
        lcd.write(1);
        lcd.print("C");      
        Tmax_N=change_value(4,1,Tmax_N,0,99);
        Tmax_Tp=change_value(7,1,Tmax_Tp,0,99);
        EEPROM.write(addr, Tmax_N);
        EEPROM.write(addr+1, Tmax_Tp);
        EEPROM.commit();
        Tmax = ((Tmax_N*100)+Tmax_Tp )/100.0;// Cai dat Tmax
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
      lcd.setCursor(9, 1);
      lcd.print(Time_h);
      lcd.write('h');
      lcd.print(Time_m);
      lcd.write('P'); 
}
void Read_Ds18B20(){
      sensors.requestTemperatures(); 
      Temp_Real= sensors.getTempCByIndex(0);
}
//------Ham ngat timer--------------
void changeState()
{
  digitalWrite(LED, !(digitalRead(LED)));  //Invert Current State of LED 
  if(Time_Down > 0 )
  {
      Time_Down--;
  }   
}
