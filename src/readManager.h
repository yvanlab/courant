

#ifndef readManager_h
#define BaseMareadManager_hnager_h

#if defined(ARDUINO) && ARDUINO >= 100

#include "Arduino.h"

#else

#include "WProgram.h"

#endif

class ReadManager
{
  public:
    ReadManager(unsigned char adress) {m_adress=adress;};
    int read(){setChannel(); return analogRead(A0);}
    void setChannel() {};
  private:
    unsigned char m_adress;
  };

#endif
