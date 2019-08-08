/*
 * TouchKey 
 * July , 2019
 * created by david   
 * for details, see http://www.arduino.cn
*/

#ifndef TouchKey_H_
#define TouchKey_H_

#define ROWS    4
#define COLS    3

class TouchKey
{
  private://函数内部用  是冒号不是分号
    /***********************************************************************
     * 4x4矩阵键盘控制
     ***********************************************************************/
     const int rowPins[ROWS] = {PA3, PB0, PB1, PB10};     //定义行引脚
     const int colPins[COLS] = {PA0, PA1, PA2};           //定义列引脚
     /*char keys[ROWS][COLS] = {
          {'1','2','3','A'},
          {'4','5','6','B'},
          {'7','8','9','C'},
          {'*','0','#','D'}
     };  //建立二维数组，用于设置按键的输出字符 */
     unsigned char ReadscanPort(byte scanPort);
     const unsigned char ledsArray[ROWS][COLS] = {
          {0x01,0x02,0x04},
          {0x08,0x10,0x20},
          {0x40,0x80,0xff},
          {0xfe,0x00,0xfd}
     };
  public: //函数外部用  是冒号不是分号
     
     unsigned char scanPort();
};
#endif
