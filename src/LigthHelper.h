#ifndef LigthHelper_h
#define LigthHelper_h

#include "Arduino.h"
#include "DelayHelper.h"


class LigthHelper : public DelayHelper
{

  public:

    LigthHelper(uint8_t pin) : DelayHelper() {
        m_pin=pin,
        pinMode(pin, OUTPUT);
        digitalWrite(m_pin, HIGH );
    };

    void handle() {
        if (!isDone()) {
            if  (!m_isOn) {
                digitalWrite(m_pin, LOW);
                m_isOn = true;
                DEBUGLOGF("LigthHelper[%d] : OnN\n",m_pin);
            }
        } else {
            if  (m_isOn) {
                digitalWrite(m_pin, HIGH );
                m_isOn = false;
                 DEBUGLOGF("LigthHelper[%d] : Off\n",m_pin);
            }
        }
    }

    

    
  private:
    uint8_t m_pin;
    bool m_isOn = false;


};

#endif
