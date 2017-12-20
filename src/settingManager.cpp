

#include "SettingManager.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

SettingManager::SettingManager(unsigned char pinLed) : BaseSettingManager(pinLed){

}

/*String SettingManager:: toString(boolean bJson = false){
  //return "SSID[" + String(m_ssid) + "] PASS[" + m_password + "] privateKey[" + m_privateKey + "] publicKey[" + m_publicKey + "] Hum["+m_HUM_SEUIL+"]";
  return BaseSettingManager::toString(bJson) + " Hum["+m_HUM_SEUIL+"]";
}
unsigned char SettingManager::readData(){
  BaseSettingManager::readData();
  switchOn();
  char tmp[8];
  readEEPROM(tmp);
  //Serial.print("m_HUM_SEUIL-Read");Serial.print(":");DEBUGLOG(tmp);
  m_HUM_SEUIL = atof(tmp);
  //Serial.print("m_HUM_SEUIL");Serial.print(":");DEBUGLOG(m_HUM_SEUIL);
  switchOff();
}
unsigned char SettingManager::writeData(){
  BaseSettingManager::writeData();
  switchOn();
  char tmp[8];
  dtostrf(m_HUM_SEUIL, 8, 6, &tmp[0]);
  //Serial.print("m_HUM_SEUIL");Serial.print(":");DEBUGLOG(tmp);
  writeEEPROM(tmp);
  EEPROM.commit();
  switchOff();
}*/
