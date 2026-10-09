#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_INA219.h>
#include "../config.h"
#include "../saver.h"
WiFiClient wifi;PubSubClient mqtt(wifi);Adafruit_INA219 currentSensor(0x40);Saver saver;
bool sensorReady=false,lastSafe=false;uint32_t sampleAt=0,reportAt=0,retryAt=0;
void command(char*,byte* b,unsigned n){if(n==5&&!memcmp(b,"RESET",5))saver.reset(lastSafe);if(n==3&&!memcmp(b,"OFF",3))saver.stop();}
void setup(){
 pinMode(25,OUTPUT);digitalWrite(25,LOW);Serial.begin(115200);
 analogReadResolution(12);analogSetPinAttenuation(34,ADC_11db);Wire.begin(21,22);sensorReady=currentSensor.begin();
 mqtt.setServer(MQTT_HOST,MQTT_PORT);mqtt.setCallback(command);mqtt.setSocketTimeout(1);
 if(strlen(WIFI_SSID))WiFi.begin(WIFI_SSID,WIFI_PASSWORD);
}
void loop(){
 uint32_t now=millis();
 static char line[16];static uint8_t count=0;static bool overflow=false;
 while(Serial.available()){char c=Serial.read();if(c=='\n'){if(!overflow)command(nullptr,(byte*)line,count);count=0;overflow=false;}else if(c!='\r'){if(count<sizeof(line))line[count++]=c;else overflow=true;}}
 if(strlen(MQTT_HOST)&&WiFi.status()==WL_CONNECTED&&!mqtt.connected()&&uint32_t(now-retryAt)>=10000){
  retryAt=now;String cid="energy14-"+String((uint32_t)ESP.getEfuseMac(),HEX);
  bool ok=strlen(MQTT_USER)?mqtt.connect(cid.c_str(),MQTT_USER,MQTT_PASSWORD):mqtt.connect(cid.c_str());
  if(ok)mqtt.subscribe("energy14/command");
 }
 mqtt.loop();
 if(uint32_t(now-sampleAt)<100){delay(1);return;}sampleAt=now;
 int lo=4095,hi=0;bool micValid=true;
 for(int i=0;i<64;i++){int v=analogRead(34);lo=min(lo,v);hi=max(hi,v);if(v<2||v>4093)micValid=false;delayMicroseconds(100);}
 Wire.beginTransmission(0x40);bool ack=Wire.endTransmission()==0;
 if(!sensorReady&&ack)sensorReady=currentSensor.begin();
 float ma=sensorReady&&ack?currentSensor.getCurrent_mA():NAN;
 float volts=sensorReady&&ack?currentSensor.getBusVoltage_V():NAN;
 bool valid=micValid&&ack&&isfinite(ma)&&isfinite(volts)&&ma>=0&&ma<=1000&&volts>=0&&volts<=6;
 lastSafe=valid&&ma<500;saver.tick(now,valid,ma,hi-lo>=500);digitalWrite(25,saver.on?HIGH:LOW);
 if(uint32_t(now-reportAt)>=1000){reportAt=now;char out[180];snprintf(out,sizeof(out),"{\"id\":14,\"valid\":%s,\"current_ma\":%.2f,\"bus_v\":%.2f,\"sound_pp\":%d,\"relay\":%s,\"latched\":%s}",valid?"true":"false",valid?ma:0,valid?volts:0,hi-lo,saver.on?"true":"false",saver.latched?"true":"false");Serial.println(out);if(mqtt.connected())mqtt.publish("energy14/state",out,true);}
}
