#include "ESP8266.h"
#include "string.h"
#include "Wire.h"
#include "I2Cdev.h"
#include "MPU6050.h"

#define move_count 3
#define move_up 1
#define move_down -1

MPU6050 accelgyro;

int16_t ax, ay, az;
int16_t gx, gy, gz;

int16_t last_move=0;
int counter=0;
int16_t movement[100]={0};
int tmp=0;
int sleep=0;
//==test data
//char *User_ID= "0000000000000001";

#define SSID        "yyb"
#define PASSWORD    "12345678"
#define Device_ID    "00000000000000000001"

ESP8266 wifi(Serial1,115200);
char *join_ap="AT+CWJAP=\"yyb\",\"12345678\"\r\n";
char *create_tcp="AT+CIPSTART=\"TCP\",\"192.168.137.1\",6666\r\n";
char *stop_tcp="AT+CWQAP\r\n";
char* ends="\r\n\0";
int powpin = A3;
char powstr[4];

void setup(void)
{
    Wire.begin();
    //Serial.begin(115200);
    accelgyro.initialize();
    if(wifi.setOprToStation()){
    //Serial.print("set station ok: ");
    }else{
      setup();
    //Serial.print("set station err: ");
    }
    pinMode(powpin,INPUT);
    delay(2000);
}

void loop() {
 uint8_t NFC_input[7];
 long powers;
 int ni=0;
l: delay(1000);
 ni=0;
 //Serial.print("ask for 7 byte: ");
 Wire.requestFrom(31,7);
 //
 while(Wire.available()>0&&ni<7){
   NFC_input[ni++]=Wire.read(); 
 }
 if(NFC_input[0]==0&&NFC_input[1]==0){
 //Serial.println("rnone:");
 goto l;
 }
 if(ni==7){
  /*
  Serial.print("rev:");
  for(int i=0;i<7;i++){
  Serial.print(NFC_input[i]);
  //Serial.print("\t");
  }
  Serial.print("\n");
  */
  //-------------------------------------------------------
 l2: avgpow();
 powers=avgpow();
 //Serial.println(powers);
 if(powers<=0){
 goto l2; 
 }
 if(powers>9999){
  powers=9999;
 }
 powstr[0]='0'+powers/1000;
 powstr[1]='0'+(powers/100)%10;
 powstr[2]='0'+(powers/10)%10;
 powstr[3]='\0';
 //send_to_ap(powstr);

  //--------------------------
  char *User_ID_tmp=to_string((unsigned char*)NFC_input,4);
  strcat(User_ID_tmp,"000000000000");
  char *User_ID=(char*)malloc(sizeof(char)*50);
  User_ID[20]='\0';
  strncpy(User_ID,User_ID_tmp,20);
  char *User_ID_in=(char*)malloc(sizeof(char)*21);
  strcpy(User_ID_in,User_ID);
  //Serial.println(User_ID);
  strcat(User_ID,Device_ID);
  strcat(User_ID,"000");
  strcat(User_ID,powstr);
  strcat(User_ID,ends);
 // Serial.println(User_ID);
  send_to_ap(User_ID);
  //Serial.println("login;");
  main_p(User_ID_in);
  }else{
  /*Serial.println("REV ERR");  
  for(int i=0;i<7;i++){
  //Serial.print(NFC_input[i]);
  
  }*/
  delay(1000);
  }
}

void main_p(char *User_ID)
{
  init_mov();
  int mv=1;
  int avg=100;
  
   while(1){
    if(sleep==50){ break;}//none use
    int te=detect_mov();
    //Serial.println(te);
    if(te==0){
      sleep++;
    }else if(te==1||te==-1){
      sleep=0;
      mv++;
    }else {
      sleep=0;  
    }
    }
 
 char mov_num[4];
 iota(mv,mov_num);
 char data_sends[46]="";
 strcat(data_sends,User_ID);
 strcat(data_sends,Device_ID);
 strcat(data_sends,mov_num);
 strcat(data_sends,powstr);
 strcat(data_sends,ends);
 send_to_ap(data_sends);
 delay(2000);
}

void init_mov()
{
   last_move=0;
   counter=0;
   sleep=0;
   memset(movement,0,100);
   tmp=0;
   return ;
}
int detect_mov() {
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    if(gz<=800&&gz>=-1000){
        delay(100);
    return 0;
    } else {
    tmp=check_move(gz);
    if(counter==0){
      movement[counter]=tmp;
      counter++;
            delay(100);
      return 2;
      }else{
        if(tmp==movement[counter-1]){
          movement[counter]=tmp;
          counter++;
              delay(100);
          return 3;
          }else{
          if(counter>move_count){
            last_move=movement[counter-1];
            counter=0;
                delay(100);
            return last_move;
            }else{
              counter=0;
                  delay(100);
              return 4;
            }  
          }
        }
    }
    delay(100);
}

int check_move(int16_t gz)
{
  if(gz<-1000){
    return move_down;
    }
  if(gz>800){
    return move_up;
    }
}



//--input AT_command (to ESP8266 through Serial1) debug mode
//--display msg from ESP8266
//no retval
void AT_write(char *jap) {
  char c;
  int i=0;
  for(;;i++) {
    c = jap[i];
    Serial1.write(c);
    if(c=='\n'){
      break;
      }
  }
  delay(1000);//--AT+CWJAP will take a while
}

//--input int 0~999
//--return char[]
void iota(int n,char ret[4])
{
  ret[0]=(char)(n/100+48);
  ret[1]=(char)((n%100)/10+48);
  ret[2]=(char)(n%10+48);
  ret[3]='\0';
}

void send_to_ap(char* data_sends)
{
    AT_write(join_ap);
    delay(1500); 
    //Serial.println(wifi.joinAP(SSID,PASSWORD));
    wifi.joinAP(SSID,PASSWORD);
    delay(1000); 
    
    if(wifi.createTCP("192.168.137.1",6666)){
      //Serial.println("tcp ok");
    }else{
      //Serial.println("tcp err");
      }
    //AT_write(create_tcp);
    //delay(2000); 
    uint8_t* he=(uint8_t*)data_sends;
    if(wifi.send(he,strlen(data_sends))){
      //Serial.println("send ok");
      }else{
       // Serial.println("send err");
      }
    if(wifi.releaseTCP()){
      //Serial.println("tcp released");
    }else{
      //Serial.println("tcp released fail");
    }
    if(wifi.leaveAP()){
      //Serial.println("leave AP");  
    }else{
      //Serial.println("leave AP err");
    }
}

char* to_string(unsigned char *s,int len)
{
  char *re=(char*)malloc(sizeof(char)*20);
  memset(re,'\0',20);
  char c[4];
  for(int i=0;i<len;i++){
    c[0]=(int)s[i]/100+48;
    c[1]=(int)s[i]/10%10+48;
    c[2]=(int)s[i]%10+48;
    c[3]='\0';
    strcat(re,c);
  }
  return re;
}
int avgpow()
{
  int val[100];
  int cnt;
  l3: cnt=0;
  for(int i=0;i<100;i++){
  val[i]=analogRead(powpin);
  delay(5);
  }
  sort(val,100);
  for(int i=30;i<70;i++){
    cnt+=val[i];
  }
  if(cnt<=0){
  goto l3;
  }
  return cnt;
}

void sort(int *a,int len)
{
  int i=0;
  int j;
  int t;
  for(i=0;i<len-1;i++)
  {
    for(j=0;j<len-i-1;j++)
    {
      if(a[j]>a[j+1]){
      t=a[j];
      a[j]=a[j+1];
      a[j+1]=t;
    }
   }
  }
}
