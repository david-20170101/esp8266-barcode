/*
 * Buzzer 
 * July , 2019
 * created by david   
 * for details, see http://www.arduino.cn
*/

#ifndef BUZZER_H_
#define BUZZER_H_

#define pinBuzzer PC14

class Buzzer
{
  private://函数内部用  是冒号不是分号
    long frequency = 1500; //频率, 单位Hz
    
  public: //函数外部用  是冒号不是分号
    void BuzzerOn(void);
    void BuzzerWarning(void);
};
#endif
