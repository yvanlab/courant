
#include "EauCapteur.h"

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#include <FunctionalInterrupt.h>


EauCapteur::EauCapteur(uint8_t pinFlowmeter, uint8_t pinLed)
  : SensorBase(pinLed/*,capteurValue::MODE_CUMULATIVE*/) {

    attachInterrupt(digitalPinToInterrupt(pinFlowmeter), std::bind(&EauCapteur::pin_ISR, this), RISING);
    m_startCompteTour = millis();

}


float EauCapteur::mesure(){
  switchOn();
  #ifdef MCPOC_MOCK
     float_t compteur = (float_t)random(500);
  #else
    float_t compteur = (float_t)m_compteTour;
  #endif
  float_t duration = (float_t)(millis() - m_startCompteTour);
  m_compteTour = 0;
  m_startCompteTour = millis();
  
  DEBUGLOGF("flowmeter : Nb tour [%d], duree[%d]\n", duration);
  
  //F(Hz)=8.1*Q - 3(+/-10%) Q=L/mi
  float_t litres = compteur/FlowMeterContante; 
  float_t litrePerMinute = (litres/duration)*60000.0;
  DEBUGLOGF("flowmeter : Litre [%f], debit[%f]\n", litres,litrePerMinute);
  #ifdef MCPOC_MOCK
  setStatus(0, "mocked");
  #else
  setStatus(0, "ok");
  #endif
  m_capteurValue.mesure(litres);
  switchOff();
  return litres;
}

