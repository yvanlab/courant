

#ifndef SettingManager_h
#define SettingManager_h

#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#else

#include "WProgram.h"

#endif

#include <EEPROM.h>
#include <BaseManager.h>
#include <BaseSettingManager.h>

class SettingManager : public BaseSettingManager
{
  public:
//    float m_HUM_SEUIL; //float
    SettingManager(unsigned char pinLed);


};

#endif
