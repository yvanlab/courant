

#ifndef ElectricLineA0_h
#define ElectricLineA0_h

#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#else

#include "WProgram.h"

#endif
#include <SensorBase.h>


class ElectricLineA0 : public SensorBase
{

  public:

    ElectricLineA0(uint8_t pinLed);

    float mesure();

    float getKWattHour() {return 0.0;};

    String getClassName(){return "ElectricLine";}

  private:
    const uint32_t twoPeriodsMS = 4000/50;
    float m_kwh=0;



};

#endif
