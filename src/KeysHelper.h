#ifndef KeysHelper_h
#define KeysHelper_h

#include "Arduino.h"

#include <SD.h>
#include <LittleFS.h>
#include "somfy_c1101.h" 

#define MAX_KEYS  10

class KeysHelper 
{
  public:
    KeysHelper(uint8_t aa) {
        m_nbAuthorisedKeys =aa;
        m_nbReceivedKeys =0;
    };
    
    void begin() {
        loadKeys();
    };

    String toJson(SomfyC1101::RemoteCommand *rc) {
        String str ;
        str += "{\"address\":\"" + String(rc->address,HEX) + "\",";
        str += "\"time\":\"" +  getDateTimeStr(rc->time) + "\",";
        str += "\"rollingCode\":\"" + String(rc->rollingCode) + "\"}";
        return str;
    };
    
    String toJson() {
        String str = "\"authorized\":[";
        for (int8_t i=0 ; i< m_nbAuthorisedKeys ; i++) {
            str += toJson(&m_authorizedKeys[i]);
            if (i<m_nbAuthorisedKeys-1)
                str += ",";
        }
        str += "],\"received\":[";
        for (int8_t i=0 ; i< m_nbReceivedKeys ; i++) {
            str += toJson(&m_receivedKeys[i]);
            if (i<(m_nbReceivedKeys-1))
                str += ",";
        }
        str += "]";
      return str;
    };

    
    bool addKeyReceived(SomfyC1101::RemoteCommand *rc) {
        for (uint8_t i =0; i < m_nbReceivedKeys; i++ ) {
            if (m_receivedKeys[i].address == rc->address) {
                m_receivedKeys[i].rollingCode = rc->rollingCode;
                m_receivedKeys[i].time = rc->time;
                DEBUGLOGF("Keys Received [%x][%d]\n",m_receivedKeys[i].address,  m_receivedKeys[i].rollingCode);
                return true;
            };
        }
        if (m_nbReceivedKeys < MAX_KEYS) {
            m_receivedKeys[m_nbReceivedKeys].address = rc->address;
            m_receivedKeys[m_nbReceivedKeys].rollingCode = rc->rollingCode;
            m_receivedKeys[m_nbReceivedKeys].time = rc->time;
            DEBUGLOGF("Keys Received [%x][%d]\n",m_receivedKeys[m_nbReceivedKeys].address,  m_receivedKeys[m_nbReceivedKeys].rollingCode);
        }
        m_nbReceivedKeys++;
        return false;
    };
    
    bool lookForKey(SomfyC1101::RemoteCommand *rc) {
        addKeyReceived(rc);
        for(uint8_t i=0;i<m_nbAuthorisedKeys;i++) {
            if (m_authorizedKeys[i].address == rc->address) {
                return true;
            }
        }
        return false;
        
    };

    void loadKeys() {
        char cfgFileName[] =  "/keys";
        File file = LittleFS.open(cfgFileName, "r");
        if (file){
            while (file.available() && m_nbAuthorisedKeys < MAX_KEYS ) {
                String sLgn = file.readStringUntil('\n');
                String sAdre = sLgn.substring(0,sLgn.indexOf(','));
                String sRolling = sLgn.substring(sLgn.indexOf(',')+1, sLgn.lastIndexOf(','));

                m_authorizedKeys[m_nbAuthorisedKeys].address = strtol(sAdre.c_str(), NULL, 16);
                m_authorizedKeys[m_nbAuthorisedKeys].rollingCode = sRolling.toInt();
                m_authorizedKeys[m_nbAuthorisedKeys].time = now();

                DEBUGLOGF("Keys [%d][%s][%x][%d]\n",m_nbAuthorisedKeys,sAdre.c_str(),  m_authorizedKeys[m_nbAuthorisedKeys].address,m_authorizedKeys[m_nbAuthorisedKeys].rollingCode);
                m_nbAuthorisedKeys++; 
            }        
            file.close();
        } else {
            DEBUGLOGF("%s file does not exist\n", cfgFileName);
        }
        return;
    };


    String getDateTimeStr (time_t moment) {
        char timeStr[25];
        sprintf (timeStr, "%02d/%02d/%4d %02d:%02d:%02d",day (moment), month (moment), year (moment), hour (moment), minute (moment), second (moment));

        return timeStr;
    }


  private:
    SomfyC1101::RemoteCommand m_authorizedKeys[MAX_KEYS];
    SomfyC1101::RemoteCommand m_receivedKeys[MAX_KEYS];
    uint8_t m_nbAuthorisedKeys = 0;
    uint8_t m_nbReceivedKeys = 0;
};


#endif
