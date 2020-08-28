

#ifndef EauCapteur_h
#define EauCapteur_h

#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#else

#include "WProgram.h"

#endif
#include <SensorBase.h>


class EauCapteur : public SensorBase
{

  public:

    EauCapteur(uint8_t pinFlowmeter, uint8_t pinLed);

    void pin_ISR()  {
      m_compteTour++;
    }

    float mesure();

    float getKWattHour() {return 0.0;};

    String getClassName(){return "EauCapteur";}

  private:
    float FlowMeterContante = 440;
    uint16_t m_compteTour;
    uint32_t m_startCompteTour;
};

#endif
