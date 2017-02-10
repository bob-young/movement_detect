#include"Wire.h"
#include <Adafruit_NFCShield_I2C.h>
#define IRQ   (2)
Adafruit_NFCShield_I2C nfc(IRQ);

volatile uint8_t uids[7];
volatile boolean flag;

void setup() {
  // put your setup code here, to run once:
  //Serial.begin(115200);
  Wire.begin(31);
  //Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  //Serial.println("nfc");
  nfc.begin();
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();
  flag=true;
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.print("sleeping");
  if(flag){
  boolean success;
  uint8_t uidLength;
   uint8_t uid[7]={0,0,0,0,0,0,0};
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
  if(!success){
  //for(int i=0;i<7;i++){
  //Serial.print(uids[i],DEC);
  //}  
  uids[1]=0;uids[2]=0;uids[3]=0;uids[4]=0;uids[5]=0;uids[6]=0;uids[0]=0;
  //for(int i=0;i<7;i++){
  //Serial.print(uids[i],DEC);
  //}
  }else{
  //for(int i=0;i<7;i++){
  //Serial.print(uids[i],DEC);
  //}
  uids[0]=uid[0];uids[1]=uid[1];uids[2]=uid[2];uids[3]=uid[3];
  uids[4]=uid[4];uids[5]=uid[5];uids[6]=uid[6];
  //for(int i=0;i<7;i++){
  //Serial.print(uids[i],DEC);
  //}
  }
  flag=false;
  }else{
  delay(2000);
  }
} 
//---
/*
void receiveEvent(int howMany)
{
  Serial.println("receive");
  while(Wire.available()>1){
    char s=Wire.read();
    Serial.print(s);
    }
  byte c=Wire.read();
  if(c==0){
    flag=true;
  }
}*/
//---
void requestEvent()
{
  uint8_t uid[7]={0,0,0,0,0,0,0};
  uid[0]=uids[0];uid[1]=uids[1];uid[2]=uids[2];uid[3]=uids[3];
  uid[5]=uids[5];uid[6]=uids[6];uid[4]=uids[4];
  Wire.write(uid,7);
  for(int i=0;i<7;i++){
  uids[i]=0;
  }
  flag=true;
}
