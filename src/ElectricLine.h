

#ifndef ElectricLine_h
#define ElectricLine_h

#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#else

#include "WProgram.h"

#endif
#include "BaseManager.h"

#include <ADCBase.h>

#include <ADS1115.h>

class ElectricLine : public ADCBase
{

  public:

    ElectricLine(uint8_t pinLed,ADS1115 *adc, uint8_t address=0);

    float mesure();


    float getKWattHour() {return 0.0;};

    String getClassName(){return "ElectricLine";}

  private:
    const uint32_t twoPeriodsMS = 4000/50;
    float m_kwh=0;



};

#endif
