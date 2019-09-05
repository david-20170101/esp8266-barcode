/*
 * RC522 
 * July , 2019
 * created by david   
 * for details, see http://www.arduino.cn
*/

#ifndef RC522_H_
#define RC522_H_

#define SS_PIN    PA4
//#define SCK_PIN   PA5      //时钟SCLK
//#define MOSI_PIN  PA7
//#define MISO_PIN  PA6
#define RST_PIN   PB11
 
class RC522
{
  private://函数内部用  是冒号不是分号
    // Init array that will store new NUID 
    unsigned char cardBuffer[16];
    byte buffer[18];
    byte block = 16;

  public: //函数外部用  是冒号不是分号
    byte nuidPICC[4]={0x00,0x00,0x00,0x00};
    unsigned char nuidPICC_SIZE;
    unsigned char cardUID_SIZE = 13;
    RC522();   //构造函数 
    void initial_RC522(void);
    void loop_RC522(void);
    void get_RC522_CardID(unsigned char *cardID);
    void get_RC522_CardUID(unsigned char *cardUID);
};
#endif
