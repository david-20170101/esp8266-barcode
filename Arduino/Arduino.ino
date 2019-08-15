
#include <qrcode.h>
#include "Lcd12864.h"
#include "Buzzer.h"
#include "74HC595.h"
#include "TouchKey.h"
#include "RC522.h"

#define MQTTSerial Serial
//#define DebugSerial Serial1
#define USART_RX_BUF_SIZE 128
#define USART_TX_BUF_SIZE 128

#define pin_PC13  PC13     //系统自带LED灯

QRCode    qrcode;
lcd12864  Lcd12864;
Buzzer    Buzzer;
IC74HC595 HC595;
TouchKey  TouchKey;
RC522     RC522;

unsigned char qrcodemap[288];
char qrcode_str[74]={
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,    //xxxxxxxxxx
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,    //xxxxxxxxxx
0xFF,0xFF,0xFF,0xFF,0x20,0xFF,0xFF,0xFF,0xFF,0xFF,    //xxxxxxxxxx
0xFF,0xFF,0xFF,0xFF,0xFF,0x20,0x00,0x00,0x00,0x00,    //xxxx
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,
};
unsigned char sn_uid[8]={
0xC9,0xE8,0xB1,0xB8,0xC2,0xEB,0xA3,0xBA,
};
unsigned char logo[30]={
0xC6,0xF3,0xD2,0xB5,0xD6,0xC7,0xC4,0xDC,0xBB,0xAF,    //xxxxxxxxxx
0xCE,0xEF,0xC1,0xAA,0xCD,0xF8,0xB2,0xD6,0xBF,0xE2,    //xxxxxxxxxx
0xC8,0xCB,0xD0,0xD4,0xBB,0xAF,0xB9,0xDC,0xC0,0xED,    //xxxxxxxxxx
};
unsigned char mqtt_temp[16]={              //临时数据存储
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
};
unsigned char mqtt_init[16]={              //系统初始化
0xCF,0xB5,0xCD,0xB3,0xB3,0xF5,0xCA,0xBC,
0xBB,0xAF,0x20,0x20,0x20,0x20,0x20,0x20,
};
unsigned char mqtt_fail[16]={              //数据接收错误
0xCA,0xFD,0xBE,0xDD,0xBD,0xD3,0xCA,0xD5,
0xB4,0xED,0xCE,0xF3,0x20,0x20,0x20,0x20,
};
unsigned char mqtt_good[16]={              //数据接收正常
0xCA,0xFD,0xBE,0xDD,0xBD,0xD3,0xCA,0xD5,
0xD5,0xFD,0xB3,0xA3,0x20,0x20,0x20,0x20,
};
unsigned char mqtt_00[16]={                 //归仓模式
0xB9,0xE9,0xB2,0xD6,0xC4,0xA3,0xCA,0xBD,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
};
unsigned char mqtt_fc[16]={                 //配货模式
0xC5,0xE4,0xBB,0xF5,0xC4,0xA3,0xCA,0xBD,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
};
unsigned char mqtt_fd[16]={                 //移仓模式
0xD2,0xC6,0xB2,0xD6,0xC4,0xA3,0xCA,0xBD,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
};
unsigned char mqtt_fe[16]={                 //盘点模式
0xC5,0xCC,0xB5,0xE3,0xC4,0xA3,0xCA,0xBD,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
};
/****************************************************************/
const int bufferLength = 128;
char RxBuffer[bufferLength];
unsigned int ii = 0;
const char comma[] = ",";
const char init_cmd[] = "app.serial_number()";
const char mqtt_start[] = "app.mqtt_start()";
struct
{
  unsigned char CMD;       //命令码
  unsigned char ISOK;      //是否正确
  unsigned char LengthA;
  unsigned char LengthB;
  unsigned char LengthC;
  unsigned char LengthD;
  unsigned char LengthE;
  unsigned char LengthF;
  unsigned char LengthG;
  unsigned char LengthH;
  unsigned char LengthI;
  unsigned char LengthJ;
  unsigned char LengthK;
  char id[30];             //esp8266芯片ID
  int streamA;             //单据编号
  char streamB[30];        //卡号
  char streamC[30];        //条码
  char streamD[30];        //StyleNo
  char streamE[30];        //ColorNo
  char streamF[30];        //SizeNo
  char streamG[30];        //StyleName
  char streamH[30];        //ColorName
  char streamI[30];        //SizeName
  char streamJ[30];        //QTY
  char streamK[30];        //Unit
  char MQTT_Buffer[bufferLength];
} MQTT_Data;

void setup()
{
    MQTTSerial.begin(115200);
    while (!MQTTSerial){
      ;    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    }
    RC522.initial_RC522();
    Lcd12864.initial_lcd();
    Lcd12864.display_GB2312_string(1,1,1,mqtt_temp);
    makeQRCODE();
    display_qrcode_logo();
    
    pinMode(pin_PC13,OUTPUT);
    digitalWrite(pin_PC13,LOW);
    MQTT_Data.CMD = 0;                                         //清空命令字
    MQTT_Data.ISOK = 0;                                        //清空是否正确
    memset(MQTT_Data.id, 0, sizeof MQTT_Data.id);              //清空esp8266芯片ID
    MQTT_Data.LengthA = 0;                                     //清空单据编号
    MQTT_Data.streamA = 0;                                     //清空单据编号
    MQTT_Data.LengthB = 0;                                     //清空卡号
    memset(MQTT_Data.streamB, 0, sizeof MQTT_Data.streamB);    //清空卡号
    clrData_Buffer();                                          //清空
    HC595.HC595(MQTT_Data.streamA);
    Lcd12864.display_GB2312_string(7,1,16,mqtt_init);
    delay(15000);
    MQTTSerial.println(mqtt_start);
}

void loop()
{
  //MQTTSerial.println("loop start!");
  while (MQTT_Data.CMD == 0 ){
    if (MQTT_Data.id[0] == 0) {
      delay(13000);
      MQTTSerial.println(init_cmd); 
      delay(2000);
      getMQTTData();
      if (MQTT_Data.ISOK) {
          makeSnUid();
          makeQRCODE();
          Lcd12864.display_QRCODE_string(1,1,6,qrcodemap);
          HC595.HC595(MQTT_Data.streamA);
      }
    } else {
      delay(2000);
      getMQTTData();
      if (MQTT_Data.ISOK && MQTT_Data.CMD != 0) {
          make3line();
          if (MQTT_Data.streamA==0xfe) {                     //盘点模式
              Lcd12864.display_GB2312_string(7,1,16,mqtt_fe);
          } else if (MQTT_Data.streamA==0xfd) {              //移仓模式
              Lcd12864.display_GB2312_string(7,1,16,mqtt_fd);
          } else if (MQTT_Data.streamA==0x00) {              //归仓模式
              Lcd12864.display_GB2312_string(7,1,16,mqtt_00);
          }
      }
    }
  }
  /*****************************************/
  byte nROWCOL = TouchKey.scanPort();
  if (nROWCOL != 0xf0)
  {
    MQTT_Data.streamA = nROWCOL;
    strcpy(MQTT_Data.streamB, MQTT_Data.id);
    Lcd12864.display_QRCODE_string(1,1,6,qrcodemap);
    Lcd12864.display_LOGO_string(1,49,10,logo);
    if (nROWCOL == 0x00) {                    //归仓模式
      HC595.HC595(MQTT_Data.streamA);
      Lcd12864.display_GB2312_string(7,1,16,mqtt_00);
      digitalWrite(LEDA,HIGH);
    } else if (nROWCOL == 0xfd) {            //移仓模式
      HC595.HC595(0x00);
      Lcd12864.display_GB2312_string(7,1,16,mqtt_fd);
      digitalWrite(LEDA,LOW);
    } else if (nROWCOL == 0xfe) {           //盘点模式
      HC595.HC595(0x00);
      sendMQTTData();
      delay(2000);
      getMQTTData();
      if (MQTT_Data.ISOK) {
        for (int i=0;i<sizeof(mqtt_fe);i++){
          mqtt_temp[i] = mqtt_fe[i];
        }
        for (int i=0;i<MQTT_Data.LengthJ;i++) {
          mqtt_temp[(sizeof(mqtt_temp)-MQTT_Data.LengthJ-MQTT_Data.LengthK)+i] = MQTT_Data.streamJ[i];
        }
        for (int i=0;i<MQTT_Data.LengthK;i++) {
          mqtt_temp[(sizeof(mqtt_temp)-MQTT_Data.LengthK)+i] = MQTT_Data.streamK[i];
        }
        Lcd12864.display_GB2312_string(7,1,16,mqtt_temp);
      }
      digitalWrite(LEDA,LOW);
    } else if (nROWCOL == 0xff) {
      HC595.HC595(0x00);
      MQTTSerial.println(init_cmd);
      delay(2000);
      getMQTTData();
      HC595.HC595(MQTT_Data.streamA);
      digitalWrite(LEDA,LOW);
    } else {
      HC595.HC595(MQTT_Data.streamA);
      sendMQTTData();
      delay(2000);
      getMQTTData();
      HC595.HC595(MQTT_Data.streamA);
      if (MQTT_Data.ISOK and MQTT_Data.streamA != 0x00){    //配货模式
        Lcd12864.display_GB2312_string(7,1,16,mqtt_fc);
        make3line();
        digitalWrite(LEDA,HIGH);
      }
    }
  }
  /*****************************************/
  RC522.loop_RC522();
  if (RC522.nuidPICC[0] != 0 || 
      RC522.nuidPICC[1] != 0 ||
      RC522.nuidPICC[2] != 0 ||
      RC522.nuidPICC[3] != 0 )  {
      Lcd12864.display_QRCODE_string(1,1,6,qrcodemap);
      Lcd12864.display_LOGO_string(1,49,10,logo);
      digitalWrite(LEDA,HIGH);
      MQTT_Data.LengthB = 0;                                     //清空卡号
      memset(MQTT_Data.streamB, 0, sizeof MQTT_Data.streamB);    //清空卡号
      MQTT_Data.LengthB = 2*RC522.nuidPICC_SIZE;  
      RC522.get_RC522_CardID(mqtt_temp);
      for (int i=0;i<MQTT_Data.LengthB;i++)
      {
        MQTT_Data.streamB[i] = mqtt_temp[i];
      }
      CleanTEMP();
      makeCardNo();
      for (int i=0; i<MQTT_Data.LengthB; i++)
      {
        mqtt_temp[6+i] = MQTT_Data.streamB[i];
      }
      Lcd12864.display_GB2312_string(7,1,16,mqtt_temp);

      MQTT_Data.CMD = 0;
      sendMQTTData();
  }
  //MQTTSerial.println("loop end!"); 
}
/* void printMQTTData(void)
{
  DebugSerial.println( MQTT_Data.CMD );
  DebugSerial.println( MQTT_Data.id );
  DebugSerial.println( MQTT_Data.LengthA );
  DebugSerial.println( MQTT_Data.streamA, DEC);
  DebugSerial.println( MQTT_Data.LengthB );
  DebugSerial.println( MQTT_Data.streamB );
  DebugSerial.println( MQTT_Data.LengthC );
  DebugSerial.println( MQTT_Data.streamC );
  DebugSerial.println( MQTT_Data.LengthD );
  DebugSerial.println( MQTT_Data.streamD );
  DebugSerial.println( MQTT_Data.LengthE );
  DebugSerial.println( MQTT_Data.streamE );
  DebugSerial.println( MQTT_Data.LengthF );
  DebugSerial.println( MQTT_Data.streamF );
  DebugSerial.println( MQTT_Data.LengthG );
  DebugSerial.println( MQTT_Data.streamG );
  DebugSerial.println( MQTT_Data.LengthH );
  DebugSerial.println( MQTT_Data.streamH );
  DebugSerial.println( MQTT_Data.LengthI );
  DebugSerial.println( MQTT_Data.streamI ); 
  DebugSerial.println( MQTT_Data.LengthJ );
  DebugSerial.println( MQTT_Data.streamJ );
  DebugSerial.println( MQTT_Data.LengthK );
  DebugSerial.println( MQTT_Data.streamK ); 
} */
void readRxBuffer(void){
  while (MQTTSerial.available())
  {
    RxBuffer[ii++] = char(MQTTSerial.read());
    delay(2);
    if (ii == bufferLength) clrRxBuffer();
  }
}
void clrRxBuffer(void)
{
  memset(RxBuffer, 0, bufferLength);  //清空
  ii = 0;
}
void clrData_Buffer(void)
{
  memset(MQTT_Data.MQTT_Buffer, 0, sizeof MQTT_Data.MQTT_Buffer);  //清空
  memset(MQTT_Data.streamC, 0, sizeof MQTT_Data.streamC);          //清空
  memset(MQTT_Data.streamD, 0, sizeof MQTT_Data.streamD);          //清空
  memset(MQTT_Data.streamE, 0, sizeof MQTT_Data.streamE);          //清空
  memset(MQTT_Data.streamF, 0, sizeof MQTT_Data.streamF);          //清空
  memset(MQTT_Data.streamG, 0, sizeof MQTT_Data.streamG);          //清空
  memset(MQTT_Data.streamH, 0, sizeof MQTT_Data.streamH);          //清空
  memset(MQTT_Data.streamI, 0, sizeof MQTT_Data.streamI);          //清空
  memset(MQTT_Data.streamJ, 0, sizeof MQTT_Data.streamJ);          //清空
  memset(MQTT_Data.streamK, 0, sizeof MQTT_Data.streamK);          //清空
  MQTT_Data.LengthC = 0;
  MQTT_Data.LengthD = 0;
  MQTT_Data.LengthE = 0;
  MQTT_Data.LengthF = 0;
  MQTT_Data.LengthG = 0;
  MQTT_Data.LengthH = 0;
  MQTT_Data.LengthI = 0;
  MQTT_Data.LengthJ = 0;
  MQTT_Data.LengthK = 0;
}
void makeQRCODE(void)
{
    // Create the QR code
    // QRCode qrcode;
    uint8_t qrcodeBytes[qrcode_getBufferSize(7)];
    qrcode_initText(&qrcode, qrcodeBytes, 7, 0, qrcode_str);
    for (uint8 i = 0; i < 6; i++){
      for (uint8 j = 0; j < 6; j++){
        for (uint8 x = 0; x < 8 ; x++){
          unsigned char Mbyte=0;
          for (uint8  y= 0; y < 8; y++){
             unsigned char hs = qrcode_getModule(&qrcode, 8*j+x, 8*i+y) ? 1 : 0;
             hs = hs<<y;
             Mbyte = Mbyte + hs;
             if (y==7){
                 qrcodemap[48*i+8*j+x] = Mbyte;
                 Mbyte=0;
             }
          }
        }
      }
    }
}
void display_qrcode_logo(void)
{
  Lcd12864.clear_screen();
  Lcd12864.display_QRCODE_string(1,1,6,qrcodemap);
  Lcd12864.display_LOGO_string(1,49,10,logo);
}
void getMQTTData(void)
{
  char *MQTT_BufferHead, *MQTT_BufferTail;
  readRxBuffer();
  if ((MQTT_BufferHead = strstr(RxBuffer, "[[$MQTT,")) != NULL)
  {
    if ((MQTT_BufferTail = strstr(MQTT_BufferHead, ",]]")) != NULL)
    {
      if(MQTT_BufferTail > MQTT_BufferHead)
      {
        clrData_Buffer();
        memcpy(MQTT_Data.MQTT_Buffer, MQTT_BufferHead, MQTT_BufferTail - MQTT_BufferHead);
        //DebugSerial.println( MQTT_Data.MQTT_Buffer );
        char *result = NULL;
        unsigned int nn = 0;
        unsigned int mm = 0;
        result = strtok(MQTT_Data.MQTT_Buffer, comma);
        while( result != NULL )
        {
          if (nn==1){
            MQTT_Data.CMD = atoi( result );                      //命令字
          }
          if (nn==2){
            memcpy(MQTT_Data.id, result, strlen(result));        //Sn设备编号
          }
          if (nn==3){
            MQTT_Data.LengthA = strlen(result);
            MQTT_Data.streamA = atoi( result );
            //memcpy(MQTT_Data.streamA,result,MQTT_Data.LengthA);  //单据编号
          }
          if (nn==4){
            MQTT_Data.ISOK = atoi( result );                      //是否正确
          }
          if (nn==5 and strcmp(MQTT_Data.streamB,result)==0){
            mm = 1;                                              //卡号比对是否一致
          }
          if (nn==6 and mm==1){
            MQTT_Data.LengthC = strlen(result);
            memcpy(MQTT_Data.streamC,result,MQTT_Data.LengthC);  //条码
          }
          if (nn==7 and mm==1){
            MQTT_Data.LengthD = strlen(result);
            memcpy(MQTT_Data.streamD,result,MQTT_Data.LengthD);  //StyleNo
          }
          if (nn==8 and mm==1){
            MQTT_Data.LengthE = strlen(result);
            memcpy(MQTT_Data.streamE,result,MQTT_Data.LengthE);  //ColorNo
          }
          if (nn==9 and mm==1){
            MQTT_Data.LengthF = strlen(result);
            memcpy(MQTT_Data.streamF,result,MQTT_Data.LengthF);  //SizeNo
          }
          if (nn==10 and mm==1){
            MQTT_Data.LengthG = strlen(result);
            memcpy(MQTT_Data.streamG,result,MQTT_Data.LengthG);  //StyleName
          }
          if (nn==11 and mm==1){
            MQTT_Data.LengthH = strlen(result);
            memcpy(MQTT_Data.streamH,result,MQTT_Data.LengthH);  //ColorName
          }
          if (nn==12 and mm==1){
            MQTT_Data.LengthI = strlen(result);
            memcpy(MQTT_Data.streamI,result,MQTT_Data.LengthI);  //SizeName
          }
          if (nn==13 and mm==1){
            MQTT_Data.LengthJ = strlen(result);
            memcpy(MQTT_Data.streamJ,result,MQTT_Data.LengthJ);  //QTY
          }
          if (nn==14 and mm==1){
            MQTT_Data.LengthK = strlen(result);
            memcpy(MQTT_Data.streamK,result,MQTT_Data.LengthK);  //Unit
          }
          nn++;
          result = strtok(NULL, comma);
        } 
        digitalWrite(pin_PC13,!digitalRead(pin_PC13));
      }
    }
  }
  if (MQTT_Data.ISOK){
      Buzzer.BuzzerOn();
      Lcd12864.display_GB2312_string(7,1,16,mqtt_good);
  } else {
      Buzzer.BuzzerWarning();
      Lcd12864.display_GB2312_string(7,1,16,mqtt_fail);
  }
  clrRxBuffer();
}
void sendMQTTData(void)
{
  clrData_Buffer();
  MQTT_Data.ISOK = 0;                                        //清空是否正确
  strcat(MQTT_Data.MQTT_Buffer, "app.mqtt_upload([[ {\"SN\":\"");
  strcat(MQTT_Data.MQTT_Buffer, MQTT_Data.id);
  strcat(MQTT_Data.MQTT_Buffer, "\",");
  strcat(MQTT_Data.MQTT_Buffer, "\"Index\":\"");
  if (MQTT_Data.streamA==0xfd){
    memcpy(MQTT_Data.streamI, "253", 3);
  } else if (MQTT_Data.streamA==0xfe) {
    memcpy(MQTT_Data.streamI, "254", 3);
  } else {
    int i=0;
    while (MQTT_Data.streamA>0)
    {
      MQTT_Data.streamA = MQTT_Data.streamA >> 1;
      i++;
    }
    memset(MQTT_Data.streamI, i + 0x30, 1);
  }
  strcat(MQTT_Data.MQTT_Buffer, MQTT_Data.streamI);
  strcat(MQTT_Data.MQTT_Buffer, "\",");
  strcat(MQTT_Data.MQTT_Buffer, "\"CardID\":\"");
  strcat(MQTT_Data.MQTT_Buffer, MQTT_Data.streamB);
  strcat(MQTT_Data.MQTT_Buffer, "\",");
  strcat(MQTT_Data.MQTT_Buffer, "\"BarCode\":\"");
  strcat(MQTT_Data.MQTT_Buffer, MQTT_Data.streamC);
  strcat(MQTT_Data.MQTT_Buffer, "\"} ]])");
  MQTTSerial.println( MQTT_Data.MQTT_Buffer );
}
void makeSnUid(void)
{
  for (int i=0;i<sizeof(sn_uid)+sizeof(MQTT_Data.id);i++){
    qrcode_str[sizeof(qrcode_str)-sizeof(sn_uid)-sizeof(MQTT_Data.id)+i]=0x00;
  }
  for (int i=0;i<sizeof(sn_uid);i++){
    qrcode_str[sizeof(qrcode_str)-sizeof(sn_uid)-sizeof(MQTT_Data.id)+i]=sn_uid[i];
  }
  for (int i=0;i<sizeof(MQTT_Data.id);i++){
    qrcode_str[sizeof(qrcode_str)-sizeof(MQTT_Data.id)+i]=MQTT_Data.id[i];
  }
}
void makeCardNo(void)
{
  mqtt_temp[0]=0xBF;
  mqtt_temp[1]=0xA8;
  mqtt_temp[2]=0xBA;
  mqtt_temp[3]=0xC5;
  mqtt_temp[4]=0xA3;
  mqtt_temp[5]=0xBA;
}
void CleanTEMP(void)
{
  for (int i=0;i<sizeof(mqtt_temp);i++){
    mqtt_temp[i] = 0x20;
  }
}
void make3line(void)
{
  unsigned char temp3Line[48];
  unsigned char tempLine[46];
  if (MQTT_Data.LengthG + MQTT_Data.LengthH + MQTT_Data.LengthI + MQTT_Data.LengthJ + MQTT_Data.LengthK < 40)
  {
    for (int i=0;i<sizeof(temp3Line);i++)
    {
      temp3Line[i] = 0x20;
    }
    for (int i=0;i<sizeof(tempLine);i++)
    {
      tempLine[i] = 0x20;
    }
    /************************************/
    for (int i=0;i<MQTT_Data.LengthG;i++)
    {
      tempLine[i] = MQTT_Data.streamG[i];
    }
    tempLine[MQTT_Data.LengthG+0] = 0x20;
    for (int i=0;i<MQTT_Data.LengthH;i++)
    {
      tempLine[MQTT_Data.LengthG+1+i] = MQTT_Data.streamH[i];
    }
    tempLine[MQTT_Data.LengthG+MQTT_Data.LengthH+1] = 0x20;
    for (int i=0;i<MQTT_Data.LengthI;i++)
    {
      tempLine[MQTT_Data.LengthG+MQTT_Data.LengthH+2+i] = MQTT_Data.streamI[i];
    }
    tempLine[MQTT_Data.LengthG+MQTT_Data.LengthH+MQTT_Data.LengthI+2] = 0x20;
    for (int i=0;i<MQTT_Data.LengthJ;i++)
    {
      tempLine[MQTT_Data.LengthG+MQTT_Data.LengthH+MQTT_Data.LengthI+3+i] = MQTT_Data.streamJ[i];
    }
    //tempLine[MQTT_Data.LengthG+MQTT_Data.LengthH+MQTT_Data.LengthI+MQTT_Data.LengthJ+3] = 0x20;
    for (int i=0;i<MQTT_Data.LengthK;i++)
    {
      tempLine[MQTT_Data.LengthG+MQTT_Data.LengthH+MQTT_Data.LengthI+MQTT_Data.LengthJ+3+i] = MQTT_Data.streamK[i];
    }
    tempLine[MQTT_Data.LengthG+MQTT_Data.LengthH+MQTT_Data.LengthI+MQTT_Data.LengthJ+MQTT_Data.LengthK+3] = 0x20;
    /************************************/
    int j = 0;
    for (int i = 0; i < sizeof(tempLine); i++)
    {
      if (tempLine[i] > 0xA0) //is GB2312
      {
        if ( (i==15-j) or (i==31-j) ){
          temp3Line[j+i] = 0x20;
          j++;
        }
        temp3Line[j+i]   = tempLine[i];
        temp3Line[j+i+1] = tempLine[i+1];
        i++;
      } else {
        temp3Line[j+i] = tempLine[i];
      }
    }
    Lcd12864.display_LOGO_string(1,1,16,temp3Line);
  }
}
