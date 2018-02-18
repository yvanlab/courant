

#ifndef LuxManager_h
#define LuxManager_h

#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#else

#include "WProgram.h"

#endif
//#include "BaseManager.h"

#include <ADCBase.h>

#include <ADS1115.h>

class LuxManager : public ADCBase
{

  public:


    LuxManager(uint8_t pinLed,ADS1115 *adc, uint8_t address=0):ADCBase(pinLed,adc, address){
    };

    float mesure(){
      m_adc->setMultiplexer(m_address);
      float value = readAnalog();
      DEBUGLOGF("readLux %d : %d\n",m_address, (int)value );
      m_capteurValue.mesure(value);
      setADCStatus();
    }

    String getClassName(){return "LuxManager";}


};

#endif
