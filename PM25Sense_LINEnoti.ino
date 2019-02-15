 //******************************
 //*Abstract: Read value of PM1,PM2.5 and PM10 of air quality
 //
 //*Version：V3.1
 //*Author：Zuyang @ HUST
 //*Modified by Cain for Arduino Hardware Serial port compatibility
 //*Date：March.25.2016
 //******************************
#include <Arduino.h>
#include <pm25senses.h>
#include "LineNotify.h"
#define  LINE_TOKEN "your-token" //MY ANDBORN Cabinet Noti Wemos D1
LINENOTIFY line;
#define LENG 31   //0x42 + 31 bytes equal to 32 bytes
unsigned char buf[LENG];
int PM01Value=0;          //define PM1.0 value of the air detector module
int PM2_5Value=0;         //define PM2.5 value of the air detector module
int PM10Value=0;         //define PM10 value of the air detector module
int PreviousPM2_5Value=0; //tracking change each cycle

const char *ssid = "your wifi ssid";
const char *passw = "wifi password";

String sendername;
float lat, lng;
float pm25, pm10;

String response; 
pm25senses mydevice;


void setup()
{
  Serial.begin(9600);   //use serial0
  Serial.setTimeout(1500);    //set the Timeout to 1500ms, longer than the data transmission periodic time of the sensor
  mydevice.begin(ssid, passw);
  response = mydevice.checkServerReady();
  Serial.println("PM25Senses response :" + String(response)); 
  line.sendSticker(LINE_TOKEN,"เปิดเครื่อง",2,34);
}

void loop()
{
  if(Serial.find(0x42)){    //start to read when detect 0x42
    Serial.readBytes(buf,LENG);

    if(buf[0] == 0x4d){
      if(checkValue(buf,LENG)){
        PM01Value=transmitPM01(buf); //count PM1.0 value of the air detector module
        PM2_5Value=transmitPM2_5(buf);//count PM2.5 value of the air detector module
        PM10Value=transmitPM10(buf); //count PM10 value of the air detector module 
      }           
    } 
  }

  static unsigned long UploadTimer=millis();  
    if (millis() - UploadTimer >=5000) // upload every 5 seconds
    {
      UploadTimer=millis(); 
      //Upload to http://pm25senses.iottweet.com/      
      sendername = "HeerPong SUTTHISAN";
      lat = 13.788506;  
      lng = 100.560444;
      pm25 = PM2_5Value;
      pm10 = PM10Value;
      response = mydevice.reportPM25senses(sendername, lat, lng, pm25, pm10);
      Serial.println(response);  
      if(abs(PM2_5Value - PreviousPM2_5Value) >5){  //PM2.5 change more than 5ug/m3 in last 5 seconds
        PreviousPM2_5Value = PM2_5Value;      //save new value  
        line.sendSticker(LINE_TOKEN,"อัพเดท",2,179); 
        line.sendMessage(LINE_TOKEN,"ตอนนี้ PM1.0="+String(PM01Value)+"ug/m3");       
        line.sendMessage(LINE_TOKEN,"ตอนนี้ PM2.5="+String(PM2_5Value)+"ug/m3");   
        line.sendMessage(LINE_TOKEN,"ตอนนี้ PM10="+String(PM10Value)+"ug/m3"); 
      }
    }
  
}
char checkValue(unsigned char *thebuf, char leng)
{  
  char receiveflag=0;
  int receiveSum=0;

  for(int i=0; i<(leng-2); i++){
  receiveSum=receiveSum+thebuf[i];
  }
  receiveSum=receiveSum + 0x42;
 
  if(receiveSum == ((thebuf[leng-2]<<8)+thebuf[leng-1]))  //check the serial data 
  {
    receiveSum = 0;
    receiveflag = 1;
  }
  return receiveflag;
}

int transmitPM01(unsigned char *thebuf)
{
  int PM01Val;
  PM01Val=((thebuf[3]<<8) + thebuf[4]); //count PM1.0 value of the air detector module
  return PM01Val;
}

//transmit PM Value to PC
int transmitPM2_5(unsigned char *thebuf)
{
  int PM2_5Val;
  PM2_5Val=((thebuf[5]<<8) + thebuf[6]);//count PM2.5 value of the air detector module
  return PM2_5Val;
  }

//transmit PM Value to PC
int transmitPM10(unsigned char *thebuf)
{
  int PM10Val;
  PM10Val=((thebuf[7]<<8) + thebuf[8]); //count PM10 value of the air detector module  
  return PM10Val;
}
