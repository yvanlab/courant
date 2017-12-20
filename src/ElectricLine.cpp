
#include "ElectricLine.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif



ElectricLine::ElectricLine(uint8_t pinLed, ADS1115 *adc, uint8_t address)
  : BaseManager(pinLed) {
  m_adc = adc;
  m_address = address;
}

/*double ElectricManager::getInstantKWH() {
  // extrapolation sur 24H a partir d une 1mn de mesure
  double result = m_valueCurrent;
  return ((result/1000.0)*220*60)/(1000.0*3600);
}
*/

/*
{
  "current" : "10250",
  "KWT"  : "40.25",
  "LAST_KWT"  : 20.25
},
*/
String ElectricLine::toString(boolean bJson) {
  //return F("To be done");
  if (bJson==JSON_TEXT)
    return  String ("\"current\":\"" + String(m_valueCurrent) +\
            "\",\"KWH\":\"" + "To be done" +\
            "\",\"lastKWT\":\"" + "To be done"  +"\"");
  else {
    String mesure = "Measure  sampling["+String (m_nbreMeasure) + "] - Cumul[" + String (m_cumulCurrent) + "]";
    mesure += "\nCurrent[" + String (m_valueCurrent)  + " mA]";
    return mesure;
  }
}


uint16_t ElectricLine::measureCurrent(){
  // read during 2 periods 50 Hz :
  switchOn();
  uint16_t maxCurrentValue = 0;
  uint16_t minCurrentValue = 65535;
  uint16_t nbreMeasure = 0;
  uint32_t startMS = millis();
  while ((millis()-startMS)< twoPeriodsMS) {
      uint16_t currentValue = readAnalog();
      //DEBUGLOG (currentValue);
      if (currentValue > maxCurrentValue ) maxCurrentValue = currentValue;
      if (currentValue < minCurrentValue ) minCurrentValue = currentValue;
      nbreMeasure ++;
  }
  // 535 mv  ==> 2000W    ==> I = 9 000mA
  // 3300 mv ==> 12 336W  ==> I = 56 000mA
  // 1 mv    ==> 3.7 W    ==> I = 17 mA
  // 1 mv    == 3 300/1024  ==  3
  const int coefCurrent = 1; // mA((2000/535)/220)*(3300/1024);
  m_valueCurrent = (maxCurrentValue-minCurrentValue)*coefCurrent;
  m_cumulCurrent += m_valueCurrent;
  m_nbreMeasure++;
  
  #ifdef MCPOC_MOCK
  setStatus(0, "mocked");
  #else
  setStatus(0, "ok");
  #endif

  switchOff();
  return m_valueCurrent;
}

/*unsigned long int ElectricManager::readCumulCurrent() {
  m_globalIntervalValue += readCurrent();
  m_globalNbreValue++;

}*/

uint16_t ElectricLine::getCourant(){
  uint16_t valueCurrent = 0;
  if (m_nbreMeasure == 0)
    valueCurrent = measureCurrent();
  else
    valueCurrent = m_cumulCurrent/m_nbreMeasure;
  m_nbreMeasure = 0;
  return valueCurrent;
}


uint16_t ElectricLine::readAnalog() {
  #ifdef MCPOC_MOCK
  return random(65534);
  #else
  return m_adc.getConversionP0N3();
  #endif


}
