
#include "ElectricLineA0.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif



ElectricLineA0::ElectricLineA0(uint8_t pinLed)
  : SensorBase(pinLed) {
}


float ElectricLineA0::mesure(){
  // read during 2 periods 50 Hz :
  switchOn();
  float maxCurrentValue = -MAXFLOAT;
  float minCurrentValue = MAXFLOAT;
  uint16_t nbreMeasure = 0;
  unsigned long startMS = millis();
  while ((millis()-startMS)< twoPeriodsMS) {
    #ifdef MCPOC_MOCK
      float currentValue =  random(1024);
    #else
      float currentValue = analogRead(A0);
    #endif
      
      //DEBUGLOG (currentValue);
      if (currentValue > maxCurrentValue ) maxCurrentValue = currentValue;
      if (currentValue < minCurrentValue ) minCurrentValue = currentValue;
      nbreMeasure ++;
  }
  // 535 mv  ==> 2000W    ==> I = 9 000mA
  // 3300 mv ==> 12 336W  ==> I = 56 000mA
  // 1 mv    ==> 3.7 W    ==> I = 17 mA
  // 1 mv    == 3 300/1024  ==  3
  const int coefCurrent = 60; // mA((2000/535)/220)*(3300/1024);
  float amplitude = (maxCurrentValue-minCurrentValue);
  const int offset = 1;
  //remove default offset
  if (amplitude>offset) amplitude = amplitude - offset;
  else amplitude = 0;
  float valueCurrent = amplitude*coefCurrent;
  //m_cumulCurrent += m_valueCurrent;
  //m_nbreMeasure++;
  DEBUGLOGF("readCourant A0 : nb mesure %d, %d ,%dmA, %dW\n", nbreMeasure, (int)amplitude, (int)(valueCurrent),(int)((valueCurrent*220.0)/1000.0) );
  #ifdef MCPOC_MOCK
  setStatus(0, "mocked");
  #else
  setStatus(0, "ok");
  #endif
  m_capteurValue.mesure(valueCurrent);
  switchOff();
  return valueCurrent;
}

/*float storeKWH() {
  m_wattHour += (((currentMS-m_previousTimeMeasure)/1000.0)*(average*220.0))/1000.0; //watt.s
}*/
