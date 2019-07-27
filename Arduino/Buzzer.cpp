/*
 * Buzzer 
 * July , 2019
 * created by david   
 * for details, see http://www.arduino.cn
*/

#include "Arduino.h"
#include "Buzzer.h"

void Buzzer::BuzzerOn(void)
{
  pinMode(pinBuzzer, OUTPUT);
  digitalWrite(pinBuzzer, HIGH);
  //用tone()函数发出频率为frequency的波形
  tone(pinBuzzer, frequency );
  delay(50); //等待1000毫秒
  noTone(pinBuzzer);//停止发声
}
void Buzzer::BuzzerWarning(void)
{
  BuzzerOn();
  delay(50);
  BuzzerOn();
}
