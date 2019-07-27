/*
 * mini12864LCD 
 * August , 2012
 * created by haishen   
 * for details, see http://www.arduino.cn
*/

#ifndef LCD12864_H_
#define LCD12864_H_

#if LCD_SPI==1
// inslude the SPI library:
#include <SPI.h>
#endif

#define LCD_SPI 1             //use SPI 1:use SPI 0;IO simulate

//#define MOSI PB15           //数据信号SI
//#define MISO PB14           //数据信号SO
//#define SCK  PB13           //时钟SCLK
#define ROMCS  PA8            //ROM片选 低有效
#define LEDA   PB5            //背光电源正极3.3V
//#define VSS                 //接地
//#define VDD                 //3.3V
//#define LSCK PB13           //时钟     use SPI must connection to PIN 13
//#define LSID PB15           //数据信号 use SPI must connection to PIN 11
#define LAO  PA11             //L 命令 H  数据
#define LRST PA12             //复位，给液晶复位
#define LCS  PB12             //LCD片选 低有效

#define  Display  0xAF      //显示开启
#define  Power    0x2F      //电源全开
#define  VO       0x80      //对比度调节       
#define  AllPoint 0xA4      //非全屏显示
#define  ADCset   0xA1      //负向，131-4
#define  COMset   0xc3      //com0-com63
#define  ELECTVO  0x81      //亮度调节  调节颜色   不同的模块需要不同的  ELECTSET
#define  ELECTSET 0x2a      //亮度数值  调节颜色   不同的模块需要不同的  数值
#define  BIASSET  0xA1      //占空比1/9

class lcd12864
{
   public:
    lcd12864();  //构造函数 
    void SendByte(unsigned char Dbyte);
    void write_cmd(unsigned char Cbyte);
    void write_data(unsigned char Dbyte);
    void initial_lcd();
    void clear_screen();
    void display_128x64(unsigned char *dp);
    void lcd_address(unsigned char page,unsigned char column);
    void display_graphic_8x8(unsigned char page,unsigned char column,unsigned char *dp);
    void display_graphic_8x16(unsigned char page,unsigned char column,unsigned char *dp);
    void display_graphic_16x16(unsigned char page,unsigned char column,unsigned char *dp);
    long getAddr(uint16_t str);
    void fetchBitmap16(long address, unsigned char *dp);
    void fetchBitmap32(long address, unsigned char *dp);
    void display_GB2312_string(unsigned char page,unsigned char column,int len,unsigned char *dp);
    void display_QRCODE_string(unsigned char page,unsigned char column, int len, unsigned char *dp);
    void display_LOGO_string(unsigned char page,unsigned char column, int len, unsigned char *dp);
};
#endif
