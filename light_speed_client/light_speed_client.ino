/*
  Upload Data to IoT Server ThingSpeak (https://thingspeak.com/):
  Support Devices: LoRa Shield + Arduino 
  
  Example sketch showing how to read Temperature and Humidity from DHT11 sensor,  
  Then send the value to LoRa Server, the LoRa Server will send the value to the 
  IoT server

  It is designed to work with the other sketch dht11_server. 

  modified 24 11 2016
  by Edwin Chen <support@dragino.com>
  Dragino Technology Co., Limited
*/

#include <SPI.h>
#include <RH_RF95.h>
#include <String.h>

RH_RF95 rf95;

#define dht_dpin A0 // Use A0 pin as Data pin for DHT11. 
#define light_pin A1 // Use A1 pin as Data pin for Light
#define speed_pin A2 // Use A2 pin as Data pin for Speed.
byte bGlobalErr;
char dht_dat[5]; // Store Sensor Data
String stringOne;
float frequency = 923.2;
long nowtime = 0;
long lastime = 0;
int flag = 0;
int up = 0;
int pulse = 0;
int counter = 0;

unsigned char sendBuf1[10]={0};
unsigned char sendBuf2[10]={0};
char data[50] = {0} ;

void setup()
{
    InitDHT();
    Serial.begin(9600);
    if (!rf95.init())
        Serial.println("init failed");
    // Setup ISM frequency
    rf95.setFrequency(frequency);
    // Setup Power,dBm
    rf95.setTxPower(13);
    lastime = millis();

    Serial.println("Humidity and temperature\n\n"); 
}

void InitDHT()
{
    pinMode(dht_dpin,OUTPUT);//Set A0 to output
    pinMode(speed_pin,INPUT);//Set A2 to input
    pinMode(light_pin,INPUT);//Set A1 to input
    digitalWrite(dht_dpin,HIGH);//Pull high A0
}

//Get Sensor Data
void ReadDHT()
{
    bGlobalErr=0;
    byte dht_in;
    byte i;
        
    //pinMode(dht_dpin,OUTPUT);
    digitalWrite(dht_dpin,LOW);//Pull Low A0 and send signal
    delay(30);//Delay > 18ms so DHT11 can get the start signal
        
    digitalWrite(dht_dpin,HIGH);
    delayMicroseconds(40);//Check the high level time to see if the data is 0 or 1
    pinMode(dht_dpin,INPUT);
    // delayMicroseconds(40);
    dht_in=digitalRead(dht_dpin);//Get A0 Status
    //Serial.println(dht_in,DEC);
    if(dht_in){
        bGlobalErr=1;
        return;
    }
    delayMicroseconds(80);//DHT11 send response, pull low A0 80us
    dht_in=digitalRead(dht_dpin);
    
    if(!dht_in){
        bGlobalErr=2;
        return;
    }
    delayMicroseconds(80);//DHT11 send response, pull low A0 80us
    for (i=0; i<5; i++)//Get sensor data
    dht_dat[i] = read_dht_dat();
    pinMode(dht_dpin,OUTPUT);
    digitalWrite(dht_dpin,HIGH);//release signal and wait for next signal
    byte dht_check_sum = dht_dat[0]+dht_dat[1]+dht_dat[2]+dht_dat[3];//calculate check sum
    if(dht_dat[4]!= dht_check_sum)//check sum mismatch
        {bGlobalErr=3;}
};

int ReadSpeed(){

  if(digitalRead(speed_pin) == LOW)
    flag = 1;
  if((digitalRead(speed_pin) == HIGH) && (flag == 1))
  {
    up = 1;
    flag = 0;
  }

  if(up)
  {
    up = 0;
    return 1; 
  }
  else
  {
    return 0;
  }
}

int ReadLight(){

  if(digitalRead(light_pin) == LOW)
    flag = 1;
  if((digitalRead(light_pin) == HIGH) && (flag == 1))
  {
    up = 1;
    flag = 0;
    Serial.println("HIGH!");
  }

  if(up)
  {
    up = 0;
    return 1; 
  }
  else
  {
    return 0;
  }
}

int ReadCycle(){
  int cycletemp = 0;

  while(1){
    if(ReadLight()){
     cycletemp++;
    }

    if( (cycletemp == 3) && (digitalRead(light_pin) == HIGH) ){
      cycletemp = 0;
      return 1;
    }
  }
}

byte read_dht_dat(){
    byte i = 0;
    byte result=0;
    for(i=0; i< 8; i++)
    {
        while(digitalRead(dht_dpin)==LOW);//wait 50us
        delayMicroseconds(30);//Check the high level time to see if the data is 0 or 1
        if (digitalRead(dht_dpin)==HIGH)
        result |=(1<<(7-i));//
        while (digitalRead(dht_dpin)==HIGH);//Get High, Wait for next data sampleing. 
    }
    return result;
}
uint16_t calcByte(uint16_t crc, uint8_t b)
{
    uint32_t i;
    crc = crc ^ (uint32_t)b << 8;
    
    for ( i = 0; i < 8; i++)
    {
        if ((crc & 0x8000) == 0x8000)
            crc = crc << 1 ^ 0x1021;
        else
            crc = crc << 1;
    }
    return crc & 0xffff;
}

uint16_t CRC16(uint8_t *pBuffer,uint32_t length)
{
    uint16_t wCRC16=0;
    uint32_t i;
    if (( pBuffer==0 )||( length==0 ))
    {
      return 0;
    }
    for ( i = 0; i < length; i++)
    { 
      wCRC16 = calcByte(wCRC16, pBuffer[i]);
    }
    return wCRC16;
}

void loop()
{
    if(ReadCycle()){
      
      nowtime = millis();
      long rooptime = nowtime - lastime;
      Serial.println(rooptime);
      if (rooptime > 5000){
        ReadDHT();
        data[1] = dht_dat[0];//Get Humidity
        data[2] = dht_dat[1];//Get Humidity      
        data[3] = dht_dat[2];//Get Temperature
        data[4] = dht_dat[3];//Get Temperature
      }

      data[0] = 60000 / (nowtime - lastime);
      lastime = nowtime;

      switch (bGlobalErr)
      {
        case 0:
            Serial.print("Speed = ");
            Serial.print(data[0], DEC);//Show Speed
            Serial.print("/min ");
            Serial.print("Current humdity = ");
            Serial.print(data[1], DEC);//Show humidity
            Serial.print(".");
            Serial.print(data[2], DEC);//Show humidity
            Serial.print("%  ");
            Serial.print("temperature = ");
            Serial.print(data[3], DEC);//Show temperature
            Serial.print(".");
            Serial.print(data[4], DEC);//Show temperature
            Serial.println("C  ");
            break;
         case 1:
            Serial.println("Error 1: DHT start condition 1 not met.");
            break;
         case 2:
            Serial.println("Error 2: DHT start condition 2 not met.");
            break;
         case 3:
            Serial.println("Error 3: DHT checksum error.");
            break;
         default:
            Serial.println("Error: Unrecognized code encountered.");
            break;
    }

    int i;
    for(i = 0;i < 5;i++)
    {
        sendBuf2[i] = data[i];
    }
    rf95.send(sendBuf2, 5);//Send LoRa Data
  }
}


