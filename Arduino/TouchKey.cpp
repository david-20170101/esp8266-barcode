/*
 * TouchKey 
 * July , 2019
 * created by david   
 * for details, see http://www.arduino.cn
*/

#include <stdlib.h>
#include <stdio.h>
#include "Arduino.h"
#include "TouchKey.h"

unsigned char TouchKey::scanPort()
{
  byte returnVal = 0;
  byte nROW = ReadscanPort(0x0f);
  byte nCOL = ReadscanPort(0xf0);
  if  (nROW==0 & nCOL==0)
  {
    returnVal = 0xff;
  } else {
    nROW = nROW >> 1;
    nCOL = nCOL >> 1;
    if(nROW==4) nROW--;
    if(nCOL==4) nCOL--;
    returnVal = ledsArray[nROW][nCOL];
  }
  return returnVal;
}
unsigned char TouchKey::ReadscanPort(byte scanPort)
{
  unsigned int val;
  val = 0;
  if(0x0f == scanPort) //主要是为了获取行扫描码，相当于低四位往高四位送数
  {
    for(int i=0;i<COLS;i++)
    {
      pinMode(colPins[i],OUTPUT);
      digitalWrite(colPins[i],HIGH);
    }
    for(int i=0;i<ROWS;i++)
    {
      pinMode(rowPins[i],OUTPUT);
      digitalWrite(rowPins[i],LOW);       //读取时，先清空原有数据，反正数据在不断的输出过来
      pinMode(rowPins[i],INPUT);
      byte nRead = digitalRead(rowPins[i]);
      if(nRead>0)
      {
        bitSet(val,(ROWS-i-1)); //val |= nRead >> (ROWS-i-1)
      }
    }
  } else {  //主要是为了获取列扫描码，相当于高四位往低四位送数
    for(int i=0;i<ROWS;i++)
    {
      pinMode(rowPins[i],OUTPUT);
      digitalWrite(rowPins[i],HIGH);
    }
    for(int i=0;i<COLS;i++)
    {
      pinMode(colPins[i],OUTPUT);
      digitalWrite(colPins[i],LOW);       //读取时，先清空原有数据，反正数据在不断的输出过来
      pinMode(colPins[i],INPUT);
      byte nRead = digitalRead(colPins[i]);
      if(nRead>0)
      {
        bitSet(val,(COLS-i-1)); //val |= nRead >> (COLS-i-1)
      }
    }
  }
  return val;
}
