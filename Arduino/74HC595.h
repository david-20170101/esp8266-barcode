/*
 * 74HC595 
 * July , 2019
 * created by david   
 * for details, see http://www.arduino.cn
*/

#ifndef IC74HC595_H_
#define IC74HC595_H_

/***********************************************************************
 * arduino连接74HC595, 74HC595上各个输出IO口上连接一个LED, 指定某个灯亮或者灭
 ***********************************************************************/
 #define clockPin   PB9  //时钟引脚设置 11脚SH_CP时钟线引脚clock
 #define latchPin   PB8  //锁存引脚设置 12脚ST_CP锁存线LATCH
 #define dataPin    PB7  //数据引脚设置 14脚DS数据引脚data

class IC74HC595
{
  //private://函数内部用  是冒号不是分号
    
  public: //函数外部用  是冒号不是分号
    void HC595(int led_state);
};
#endif
