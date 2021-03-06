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
MFRC522::StatusCode status;

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

      // Store cardUID into cardUID array
      // Serial.println(F("Authenticating using key A..."));
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK) {
          // Serial.print(F("PCD_Authenticate() failed: "));
          // Serial.println(mfrc522.GetStatusCodeName(status));
          return;
      }
      // Read block
      byte byteCount = sizeof(buffer);
      status = mfrc522.MIFARE_Read(block, buffer, &byteCount);
      if (status != MFRC522::STATUS_OK) {
          // Serial.print(F("MIFARE_Read() failed: "));
          // Serial.println(mfrc522.GetStatusCodeName(status));
      } else {
          bool check = false;
          byte code ;
          for(int i=0; i<sizeof(cardBuffer)/2; i++){
              code = ~buffer[i+sizeof(cardBuffer)/2];
              if ( buffer[i] == code ){
                  check = true;
              } else {
                  break;
              }
          }
          if (check){
              for(int i=0; i<sizeof(cardBuffer)/2; i++){
                if ((i<3) && (buffer[i]==0xff)) {
                  cardBuffer[2*i]   = ~buffer[i];
                  cardBuffer[2*i+1] = ~buffer[i];
                } else {
                  code = buffer[i];
                  code = code >> 4;
                  cardBuffer[2*i] = code + 0x30;
                  code = buffer[i];
                  code = code << 4;
                  code = code >> 4;
                  cardBuffer[2*i+1] = code + 0x30;
                }
              }
              cardBuffer[2]=0x00;
              for(int i=0; i<sizeof(cardBuffer); i++){
                if ((i+3)>sizeof(cardBuffer)){
                  cardBuffer[i] = 0x00;
                } else {
                  cardBuffer[i] = cardBuffer[i+3];
                }
              }
          } else {
              return;
          }
      }
      // Store NUID into nuidPICC array
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        nuidPICC[i] = mfrc522.uid.uidByte[i];
      }
      nuidPICC_SIZE = mfrc522.uid.size;
  }

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
void RC522::get_RC522_CardUID(unsigned char *cardUID)
{
  for (byte i = 0; i < cardUID_SIZE; i++) {
    cardUID[i] = cardBuffer[i];
  }
}
