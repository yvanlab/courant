
#include "ElectricLine.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif



ElectricLine::ElectricLine(uint8_t pinLed, ADS1115 *adc, uint8_t address)
  : ADCBase(pinLed,adc,  address) {
}


float ElectricLine::mesure(){
  // read during 2 periods 50 Hz :
  switchOn();
  float maxCurrentValue = 0;
  float minCurrentValue = 65535;
  uint16_t nbreMeasure = 0;
  uint32_t startMS = millis();
  m_adc->setMultiplexer(m_address);
  while ((millis()-startMS)< twoPeriodsMS) {
      float currentValue = readAnalog();
      //DEBUGLOG (currentValue);
      if (currentValue > maxCurrentValue ) maxCurrentValue = currentValue;
      if (currentValue < minCurrentValue ) minCurrentValue = currentValue;
      nbreMeasure ++;
  }

  // 90   ==> 400W    ==> I = 1 818 mA  => 1 ==> 20 mA
  // 83   ==> 96 mA                     => 1 ==> 1,15 mA
  // 5550  ==> 2000W  ==> I = 9 091 mA  => 1 ==> 1.6 mA
  // 1 mv    ==> 3.7 W    ==> I = 17 mA
  // 1 mv    == 3 300/1024  ==  3
  const float coefCurrent = 15; // mA((2000/535)/220)*(3300/1024);
  const float offset = 4;//23; // mesure;


  float amplitude = (maxCurrentValue-minCurrentValue);
  //remove default offset
  if (amplitude>offset) amplitude = amplitude - offset;
  else amplitude = 0;
  float valueCurrent = amplitude*coefCurrent;
  //m_cumulCurrent += m_valueCurrent;
  //m_nbreMeasure++;
  DEBUGLOGF("readCourant %d : nb mesure %d, %d ,%dmA, %dW\n",m_address, nbreMeasure, (int)amplitude, (int)(valueCurrent),(int)((valueCurrent*220.0)/1000.0) );
  #ifdef MCPOC_MOCK
  setStatus(0, "mocked");
  #else
  setStatus(0, "ok");
  #endif
  m_capteurValue.mesure(valueCurrent);
  setADCStatus();
  switchOff();
  return valueCurrent;
}

/*float storeKWH() {
  m_wattHour += (((currentMS-m_previousTimeMeasure)/1000.0)*(average*220.0))/1000.0; //watt.s
}*/
