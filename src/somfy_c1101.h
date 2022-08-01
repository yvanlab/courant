#ifndef somfy_c1101
#define somfy_c1101

#include "Arduino.h"
#include <ELECHOUSE_CC1101_SRC_DRV.h>

namespace SomfyC1101
{

    enum SOMFY_READ_PHASE
    {
        PHASE_SYNCHRO_HIGH_HW = 0,
        PHASE_SYNCHRO_LOW_HW = 1,
        PHASE_SYNCHRO_SW = 2,
        PHASE_MESSAGE_READING = 3,
        PHASE_MESSAGE_READY = 4
    };

    struct Remote
    {
        uint8_t rollingCode[2]; // the current rolling code
        uint8_t address[3];     // the address of the remote
    };

    struct RemoteCommand
    {
        uint16_t rollingCode; // the current rolling code
        uint32_t address;     // the address of the remote
        uint8_t command;      // the last command
        time_t  time;
        bool checkSum;        // checksum status
    };

    const uint32_t DELAY_SYNCHRO_HW = 2416;
    const uint32_t DELAY_SYNCHRO_SW = 4750;
    const uint32_t DELAY_HALF_SYMBOL = 604;
    const uint32_t DELAY_SYMBOL = 1208;
    const uint32_t DELAY_TOLEREANCE = 150;
    const uint32_t DELAY_INTERFRAME = 30415;

    void begin(uint8_t pinRX);
    
    bool transmit(RemoteCommand tx);

    inline bool inRange(uint32_t tm, uint32_t ms);
    void IRAM_ATTR handleRX();

    bool handle();
    RemoteCommand * getReceptedCmd();

    void processFrame();
    uint8_t calculateChecksum(uint8_t *frame);
    void unscrambleFrame(uint8_t *frame);
    int findRemoteWithAddress(Remote r);

    String toString(RemoteCommand *cmd);

    extern uint8_t m_pinRX;
    extern uint8_t m_state;
    //extern uint16_t m_cpt;
    extern bool m_halfway; // Boolean is set if a statechange occurs at the start of a new bit. We then still have to wait for a falling or rising edge
    extern int m_bits;
    extern uint8_t m_frame[7]; // payload buffer
    extern uint32_t m_timeout;
    extern RemoteCommand m_rCmd;
    extern bool  m_cmdReceived;
};
#endif