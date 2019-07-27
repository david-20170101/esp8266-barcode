/*
 * mini12864LCD 
 * August , 2012
 * created by haishen   
 * for details, see http://www.arduino.cn
*/

#include <arduino.h>
#include <SPI.h>
#include "Lcd12864.h"
SPIClass SPI_2(2); //Create an instance of the SPI Class called SPI_2 that uses the 2nd SPI Port
// unsigned char i=0,j=0;
lcd12864::lcd12864()
{
}
/********************
 * 数据移位  最高位存入，
 ********************/
#if LCD_SPI==1
#else
void lcd12864::SendByte(unsigned char Dbyte)
{
  unsigned char TEMP; 
  TEMP=Dbyte;
  for(int i=0;i<8;i++)
  {
    digitalWrite(LSCK,LOW);
    TEMP=(Dbyte<<i)&0X80;
    digitalWrite(LSID,TEMP);
    digitalWrite(LSCK,HIGH);
  }
}
#endif
/****************
 * 写指令
 **************/
void lcd12864::write_cmd(unsigned char Cbyte)
{
  digitalWrite(LCS,LOW);
  digitalWrite(LAO,LOW);
#if LCD_SPI==1
  SPI_2.transfer(Cbyte);
#else
  SendByte(Cbyte);
#endif
}
/***************
 * 写数据
 ******************/
void lcd12864:: write_data(unsigned char Dbyte)
{
  digitalWrite(LCS,LOW);
  digitalWrite(LAO,HIGH);
#if LCD_SPI==1
  SPI_2.transfer(Dbyte);
#else
  SendByte(Dbyte);
#endif
}
/*******************
 * 初始化；
 *********************/
void lcd12864::initial_lcd()
{
  //L 命令 H  数据   
  //片选   低有效
  //使能信号（E) 到高有效
  //数据信号
  //复位
  pinMode(LAO,OUTPUT);
  pinMode(LCS,OUTPUT);
  pinMode(LRST,OUTPUT);
  pinMode(ROMCS,OUTPUT);
  pinMode(LEDA,OUTPUT);
#if LCD_SPI==1
  // initialize SPI_2:
  SPI_2.begin(); 
  SPI_2.setBitOrder(MSBFIRST);
  SPI_2.setClockDivider(SPI_CLOCK_DIV2);
#endif 
  digitalWrite(LEDA,LOW); 
  digitalWrite(ROMCS,LOW);
  digitalWrite(LCS,LOW);
  digitalWrite(LRST,LOW);  
  delay(200);
  digitalWrite(LRST,HIGH);
  delay(1000);  
  write_cmd(0xe2);//system reset
  delay(200);

  write_cmd(0x23);//SET VLCD RESISTOR RATIO
  write_cmd(0xa2);//BR=1/9
  write_cmd(0xa0);//set seg direction
  write_cmd(0xc8);//set com direction
  write_cmd(0x2f);//set power control
  write_cmd(0x40);//set scroll line
  write_cmd(0x81);//SET ELECTRONIC VOLUME
  write_cmd(0x20);//set pm: 通过改变这里的数值来改变电压 
  //write_cmd(0xa6);//set inverse display    a6 off, a7 on
  //write_cmd(0xa4);//set all pixel on
  write_cmd(0xaf);//set display enable
  clear_screen();
}
/*****************
 * 清屏；取模顺序是列行式，
 * 从上到下，低位在前，从左到右；
 * 先选择页地址0-7，再选择列0-127
 * 页码是直接读取8位数据作为地址；
 * 列是先读取高四位，后读取低四位；
 **********************/
void lcd12864::clear_screen()
{  
  unsigned char x,y;
  for(y=0;y<8;y++)
  {    
    write_cmd(0xb0+y);
    write_cmd(0x10);    
    write_cmd(0x00);
    for(x=0;x<132;x++)  write_data(0); 
  } 
}

void lcd12864::lcd_address(unsigned char page,unsigned char column)
{
  column=column-0x01;
  write_cmd(0xb0+page-1);           //设置页地址，每8 行为一页，全屏共64 行，被分成8 页
  write_cmd(0x10+(column>>4&0x0f)); //设置列地址的高4 位
  write_cmd(column&0x0f);           //设置列地址的低4 位
}
/*************************
 * 8*8字符，取模顺序是列行式，
 * 从上到下，高位在前，从左到右；
 * 先选择页地址0-7，再选择列0-130
 * 页码是直接读取8位数据作为地址；
 * 列是先读取高四位，后读取低四位；
 **********************/
void lcd12864::display_graphic_8x8(unsigned char page,unsigned char column,unsigned char *dp)
{    
  int i;
  lcd_address(page,column);
  for (i=0;i<8;i++)
  {
    write_data(*dp); //写数据到 LCD,每写完一个 8 位的数据后列地址自动加 1
    dp++;
  }
}
/********************************************************
 * 显示16x16 点阵图像、汉字、生僻字或16x16 点阵的其他图标
 ********************************************************/
void lcd12864::display_graphic_16x16(unsigned char page,unsigned char column,unsigned char *dp)
{
  int i,j;
  for(j=0;j<2;j++)
  {
    lcd_address(page+j,column);
    for (i=0;i<16;i++)
    {
      write_data(*dp); //写数据到LCD,每写完一个8 位的数据后列地址自动加1
      dp++;
    }
  }
}
/*************************************************************
 * 显示 8x16 点阵图像、ASCII, 或 8x16 点阵的自造字符、其他图标
 *************************************************************/
void lcd12864::display_graphic_8x16(unsigned char page,unsigned char column,unsigned char *dp)
{
  int i,j;
  for(j=0;j<2;j++)
  {
    lcd_address(page+j,column);
    for (i=0;i<8;i++)
    {
      write_data(*dp); //写数据到 LCD,每写完一个 8 位的数据后列地址自动加 1
      dp++;
    }
 }
}
/*************************************************************
 * 显示 128x64 点阵图像
 *************************************************************/
void lcd12864::display_128x64(unsigned char *dp)
{
  for(int j=0;j<8;j++)
  {
    lcd_address(j+1,1);
    for (int i=0;i<128;i++)
    {
      write_data(*dp); //写数据到 LCD,每写完一个 8 位的数据后列地址自动加 1
      dp++;
    }
  }
}
long lcd12864::getAddr(uint16_t str)
{
  byte hb = (byte)((str & 0xff00) >> 8);
  byte lb = (byte)(str & 0x00ff);
  long Address;
  if (hb > 0xA0 && lb > 0xA0)//is GB2312
  {
    long BaseAdd = 0;
    if (hb == 0xA9 && lb >= 0xA1)
    {
      Address = (282 + (lb - 0xA1 ));//8位机在此出现了溢出，所以分三步
      Address = Address * 32;
      Address += BaseAdd;
    }
    else if (hb >= 0xA1 && hb <= 0xA3 && lb >= 0xA1)
    {
      Address = ((hb - 0xA1) * 94 + (lb - 0xA1));
      Address = Address * 32;
      Address += BaseAdd;
    }
    else if (hb >= 0xB0 && hb <= 0xF7 && lb >= 0xA1)
    {
      Address = ((hb - 0xB0) * 94 + (lb - 0xA1) + 846);
      Address = Address * 32;
      Address += BaseAdd;
    }
  } else {//is ASCII
    long BaseAdd = 0x03b7c0;
    if (lb >= 0x20 && lb <= 0x7E)
    {
      Address = (lb - 0x20 ) * 16 + BaseAdd;
    }
  }
  return Address;
}
void lcd12864::fetchBitmap16(long address, unsigned char *dp)
{
  byte hb = (byte)((address & 0x00ff0000) >> 16);
  byte mb = (byte)((address & 0x0000ff00) >> 8);
  byte lb = (byte)(address & 0x000000ff);
  digitalWrite(ROMCS, LOW);//选通字库
  SPI_2.transfer(0x0b);//移入命令字
  SPI_2.transfer(hb);//移入地址次高位
  SPI_2.transfer(mb);//移入地址次低位
  SPI_2.transfer(lb);//移入地址最低位
  SPI_2.transfer(0x8d);//移入dummy byte
  for (int i = 0; i < 16; i++)
  {
    dp[i] = SPI_2.transfer(0x00); //读出16bytes
  }
  digitalWrite(ROMCS, HIGH);//反选通字库
}

void lcd12864::fetchBitmap32(long address, unsigned char *dp)
{
  byte hb = (byte)((address & 0x00ff0000) >> 16);
  byte mb = (byte)((address & 0x0000ff00) >> 8);
  byte lb = (byte)(address & 0x000000ff);
  digitalWrite(ROMCS, LOW);//选通字库
  SPI_2.transfer(0x0b);//移入命令字
  SPI_2.transfer(hb);//移入地址次高位
  SPI_2.transfer(mb);//移入地址次低位
  SPI_2.transfer(lb);//移入地址最低位
  SPI_2.transfer(0x8d);//移入dummy byte
  for (int i = 0; i < 32; i++)
  {
    dp[i] = SPI_2.transfer(0x00); //读出32bytes
  }
  digitalWrite(ROMCS, HIGH);//反选通字库
}
void lcd12864::display_GB2312_string(unsigned char page,unsigned char column, int len, unsigned char *dp)
{
  for (int i = 0; i < len; i++)
  {
    if (dp[i] > 0xA0) //is GB2312
    {
      int code = (int)(dp[i] * 256) + (int)dp[i + 1];
      unsigned char bmp[32];
      fetchBitmap32(getAddr(code), bmp);
      display_graphic_16x16(page, column, bmp);
      column += 16;
      i++;
    } else { //is ASCII
      int code = (int)dp[i];
      unsigned char bmp[16];
      fetchBitmap16(getAddr(code), bmp);
      display_graphic_8x16(page, column, bmp);
      column += 8;
    }
  }
}
void lcd12864::display_QRCODE_string(unsigned char page,unsigned char column, int len, unsigned char *dp)
{
  int i,j,k;
  unsigned char temp;
  temp = column;
  for(k=0;k<len;k++)
  {
    column = temp;
    for(j=0;j<len;j++)
    {
      lcd_address(page+k,column);
      for (i=0;i<8;i++)
      {
        write_data(*dp); //写数据到 LCD,每写完一个 8 位的数据后列地址自动加 1
        dp++;
      }
      column += 8;
    }
  }
}
void lcd12864::display_LOGO_string(unsigned char page,unsigned char column, int len, unsigned char *dp)
{
  unsigned char temp;
  temp = column;
  for (int j = 0; j < 3; j++)
  {
    column = temp ;
    for (int i = 0; i < len; i++)
    {
      if (dp[len*j+i] > 0xA0) //is GB2312
      {
        int code = (int)(dp[len*j+i] * 256) + (int)dp[len*j+i + 1];
        unsigned char bmp[32];
        fetchBitmap32(getAddr(code), bmp);
        display_graphic_16x16(page+2*j, column, bmp);
        column += 16;
        i++;
      } else { //is ASCII
        int code = (int)dp[len*j+i];
        unsigned char bmp[16];
        fetchBitmap16(getAddr(code), bmp);
        display_graphic_8x16(page+2*j, column, bmp);
        column += 8;
      }
    }
  }
}
