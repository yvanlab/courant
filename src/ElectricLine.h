

#ifndef ElectricLine_h
#define ElectricLine_h

#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#else

#include "WProgram.h"

#endif
#include "BaseManager.h"

#include <ADS1115.h>

class ElectricLine : public BaseManager
{

  public:
    ADS1115   *m_adc;
    uint8_t   m_address;
    uint32_t  m_cumulCurrent = 0;
    uint8_t   m_nbreMeasure = 0;
    uint16_t  m_valueCurrent = 0;

    /*unsigned int m_maxCurrentValue = 0;
    unsigned int m_minCurrentValue = 1026;
    unsigned char m_nbreMeasure = 0;*/
    /*unsigned int m_valueCurrent = 0;
    unsigned int m_valuePower = 0;


    double  m_wattHour = 0.0;
    double  m_previousWattHour = 0.0;
    unsigned long  m_previousTimeMeasure = 0;

    unsigned char  m_pinCurrent = A0;

    unsigned int  m_globalIntervalValue = 0;
    unsigned char m_globalNbreValue = 0;
*/
    ElectricLine(uint8_t pinLed,ADS1115 *adc, uint8_t address=0);

    String toString(boolean bJson);
    uint16_t measureCurrent();
    uint16_t getCourant();
    double getKWattHour();

    String getClassName(){return "ElectricLine";}

  private:
    const uint32_t twoPeriodsMS = 2000/50;
    uint16_t readAnalog();


};

#endif
