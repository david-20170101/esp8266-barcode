/*
 * RC522 
 * July , 2019
 * created by david   
 * for details, see http://www.arduino.cn
*/

#include <arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include "RC522.h"

MFRC522 mfrc522(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key; 

RC522::RC522(void)
{
}
void RC522::initial_RC522(void)
{
  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;                      //key.keyByte[i] = 0x8F;
  }

  //Serial.println(F("This code scan the MIFARE Classsic NUID."));
  //Serial.print(F("Using the following key:"));
  //printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}
void RC522::loop_RC522(void)
{
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! mfrc522.PICC_ReadCardSerial())
    return;

  //Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  //Serial.println(mfrc522.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    //Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (mfrc522.uid.uidByte[0] != nuidPICC[0] || 
      mfrc522.uid.uidByte[1] != nuidPICC[1] || 
      mfrc522.uid.uidByte[2] != nuidPICC[2] || 
      mfrc522.uid.uidByte[3] != nuidPICC[3] ) {
      //Serial.println(F("A new card has been detected."));

      // Store NUID into nuidPICC array
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        nuidPICC[i] = mfrc522.uid.uidByte[i];
      }
      nuidPICC_SIZE = mfrc522.uid.size;
      //Serial.println(F("The NUID tag is:"));
      //Serial.print(F("In hex: "));
      //printHex(mfrc522.uid.uidByte, rfid.uid.size);
      //Serial.println();
      //Serial.print(F("In dec: "));
      //printDec(mfrc522.uid.uidByte, rfid.uid.size);
      //Serial.println();
  }
  //else Serial.println(F("Card read previously."));

  // Halt PICC
  mfrc522.PICC_HaltA();

  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
}
void RC522::get_RC522_CardID(unsigned char *cardID)
{
  for (byte i = 0; i < sizeof(nuidPICC); i++) {
    unsigned char CID = nuidPICC[i];
    CID = CID>>4;
    if(CID<10){
       cardID[2*i] = CID+48;
    } else {
       cardID[2*i] = CID+55;
    }
    CID = nuidPICC[i];
    CID = CID<<4;
    CID = CID>>4;
    if(CID<10){
       cardID[2*i+1] = CID+48;
    } else {
       cardID[2*i+1] = CID+55;
    }
  }
  for (byte i = 0; i < sizeof(nuidPICC); i++) {
    nuidPICC[i]=0x00;
  }
}
