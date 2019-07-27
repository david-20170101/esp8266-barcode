/*
 * 74HC595 
 * July , 2019
 * created by david   
 * for details, see http://www.arduino.cn
*/

#include "Arduino.h"
#include "74HC595.h"

void IC74HC595::HC595(int led_state)
{
  pinMode(clockPin,OUTPUT);
  pinMode(dataPin, OUTPUT); 
  pinMode(latchPin,OUTPUT);
  boolean ledPin;              //led的状态变量，0或者1
  digitalWrite(latchPin,LOW);  //开始输入数据
  for(int i = 0; i <= 7;i++){
    //将1按位左移，&上led_state可以判断led_state的各位是1或者0，从而决定ledPin的HIGH或者LOW
    if(led_state & (1<<i)){
      ledPin = HIGH;
      }
    else
      ledPin = LOW;
    digitalWrite(dataPin,ledPin);  //往data里存入数据
    digitalWrite(clockPin,HIGH);   //锁存数据
    digitalWrite(clockPin,LOW);    //准备输入下一个数据
  }
  digitalWrite(clockPin,LOW);      //全部输入完毕
  digitalWrite(latchPin,HIGH);     //送出数据
  /*********************************************************************************
  送出数据之后，你可能发现，你输入的数字和实际亮的灯的顺序不对，首先，你要将灯按照D0-D7顺序排好，
  最重要的是，你要知道74HC595的数据时怎样输入的。
  *********************************************************************************/
}
