

#include "somfy_c1101.h"
#include "baseManager.h"
#include <functional>

using namespace SomfyC1101;

// SomfyC1101(uint8_t pinRX)
// {
//     m_pinRX = pinRX;
//     m_state = 0;
// }

namespace SomfyC1101
{

    uint8_t m_pinRX;
    uint8_t m_state;
    uint16_t m_cpt = 0;
    bool m_halfway = false; // Boolean is set if a statechange occurs at the start of a new bit. We then still have to wait for a falling or rising edge
    int m_bits = 0;
    uint8_t m_frame[7]; // payload buffer
    uint32_t m_timeout = 0;
    RemoteCommand m_rCmd;
    bool m_cmdReceived = false;

    void begin(uint8_t pinRX)
    {

        m_pinRX = pinRX;
        m_state = 0;

        pinMode(m_pinRX, INPUT_PULLDOWN_16);
        digitalWrite(m_pinRX, 0);

        if (ELECHOUSE_cc1101.getCC1101())
        { // Check the CC1101 Spi connection.
            DEBUGLOG("C1101:Connection OK");
        }
        else
        {
            DEBUGLOG("C1101:Connection Error");
        }

        ELECHOUSE_cc1101.setMHZ(433.42); // Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.
        ELECHOUSE_cc1101.Init();         // must be set to initialize the cc1101!
        // ELECHOUSE_cc1101.setPA(10);       // set TxPower. The following settings are possible depending on the frequency band.  (-30  -20  -15  -10  -6    0    5    7    10   11   12)   Default is max!

        ELECHOUSE_cc1101.SetRx(); // set Receive on
        delay(200);
        DEBUGLOGF("C1101 cfg [%d]\n",ELECHOUSE_cc1101.getMode());
        delay(200);
        ELECHOUSE_cc1101.SetTx(); // set Receive on
        DEBUGLOGF("C1101 cfg [%d]\n",ELECHOUSE_cc1101.getMode());
        
        ELECHOUSE_cc1101.SetRx(); // set Receive on
        attachInterrupt(m_pinRX, handleRX, CHANGE);
        DEBUGLOGF("Reception\n");
    }

    RemoteCommand *getReceptedCmd() {
        m_cmdReceived = false;
        return &m_rCmd;
    }
    
    bool handle()
    {
        if (m_state == PHASE_SYNCHRO_LOW_HW)
        {
            m_timeout = millis();
        }
        else if (m_state == PHASE_MESSAGE_READY)
        {
            //ELECHOUSE_cc1101.SetTx();
            processFrame();
            //ELECHOUSE_cc1101.SetRx();
            m_cmdReceived = true;
            m_state = PHASE_SYNCHRO_HIGH_HW;
            m_timeout = 0;
        }
        else if (m_state != PHASE_SYNCHRO_HIGH_HW)
        {
            if (m_timeout != 0 && (millis() - m_timeout) > 200)
            {
                DEBUGLOGF("Timout:[%d]\n", m_state);
                m_state = PHASE_SYNCHRO_HIGH_HW;
                m_timeout = 0;
            }
        }
        return m_cmdReceived;
    }

    inline bool inRange(uint32_t tm, uint32_t ms)
    {
        return tm > ms - 150 && tm < ms + 150;
    }

    void IRAM_ATTR handleRX()
    {
        static uint32_t last;
        static uint8_t nb_hw;
        int b;
        int val = digitalRead(m_pinRX);

        int tm = micros() - last;

        last += tm;

        switch (m_state)
        {
        case PHASE_SYNCHRO_HIGH_HW: // found a hardware sync HIGH - Goto m_state 1
            if (inRange(tm, DELAY_SYNCHRO_HW) && val == LOW)
            {
                m_state = PHASE_SYNCHRO_LOW_HW;
                // m_cpt = 0;
            }
            break;
        case PHASE_SYNCHRO_LOW_HW: // found a hardware sync LOW - Goto m_state 2
            if (inRange(tm, DELAY_SYNCHRO_HW) && val == HIGH)
            {
                if (nb_hw > 6)
                    m_state = PHASE_SYNCHRO_SW;
                nb_hw++;
            }
            // //dd[m_cpt++] = tm;
            break;
        case PHASE_SYNCHRO_SW: // found a software sync! - Goto m_state 3 - Ready to receive the bites
            nb_hw = 0;
            if (inRange(tm, DELAY_SYNCHRO_SW) && val == LOW)
            {
                m_state = PHASE_MESSAGE_READING;
                m_bits = 0;
            }
            // dd[m_cpt++] = tm;
            break;
        case PHASE_MESSAGE_READING: // receiving payload
            // dd[m_cpt++] = tm;
            b = m_bits / 8;
            if (inRange(tm, DELAY_HALF_SYMBOL))
            {
                if (m_halfway)
                {
                    bitWrite(m_frame[b], 7 - m_bits % 8, val);
                    m_bits++;
                    m_halfway = false;
                }
                else
                {
                    m_halfway = true;
                }
            }
            else if (inRange(tm, DELAY_SYMBOL))
            {
                bitWrite(m_frame[b], 7 - m_bits % 8, val);
                m_bits++;
            }
            else
            {
                Serial.printf("P%d\n", tm);
            }
            if (m_bits == 56)
            {
                m_state = PHASE_MESSAGE_READY;
            }
            break;
        case PHASE_MESSAGE_READY:
            m_state = PHASE_MESSAGE_READY;
            break;
        default:
            m_state = PHASE_SYNCHRO_HIGH_HW;
            nb_hw = 0;
            break;
        }
        digitalWrite(22, val);
    }

    void processFrame()
    {
        
        unscrambleFrame(m_frame);

        // Remote r; // initialize a new remote object and copy the received data in it
        // memcpy(&r, &m_frame[2], 5);

        m_rCmd.command = m_frame[1] >> 4;
        m_rCmd.address = ((uint32_t)m_frame[4] << 16) + ((uint32_t)m_frame[5] << 8) + m_frame[6];
        m_rCmd.rollingCode = ((uint16_t)m_frame[2] << 8) + m_frame[3];
        m_rCmd.checkSum = calculateChecksum(m_frame);
        m_rCmd.time = now();
        DEBUGLOGF("add[%x], rol[%d], cmd[%d], chckSum[%d]\n", m_rCmd.address, m_rCmd.rollingCode, m_rCmd.command, m_rCmd.checkSum);
    }

    /*
     * When a frame has been received it is scrambled. This function decodes the received frame
     */
    void unscrambleFrame(uint8_t *frame)
    {
        uint8_t temp[7];

        memcpy(temp, frame, 7);

        // unscramble
        for (int i = 1; i < 7; i++)
        {
            frame[i] = temp[i] ^ temp[i - 1];
        }
    }

    /*
     * After that a frame has been decoded we could calculate the checksum which should be always zero on received frames.
     */
    uint8_t calculateChecksum(uint8_t *frame)
    {
        // Checksum calculation: a XOR of all the nibbles
        byte checksum = 0;
        for (byte i = 0; i < 7; i++)
        {
            checksum = checksum ^ frame[i] ^ (m_frame[i] >> 4);
        }

        return checksum & 0b1111; // We keep the last 4 bits only
    }

    bool transmit(RemoteCommand tx)
    {
        // Construction de la trame claire
    //     byte data[7];
    //     data[0] = 0xA0;
    //     data[1] = tx.command << 4;
    //     data[2] = (tx.rollingCode & 0xFF00) >> 8;
    //     data[3] = tx.rollingCode & 0x00FF;
    //     data[4] = (tx.address&0xFF0000) >> 16;
    //     data[5] = (tx.address&0xFF00) >> 8;
    //     data[6] = (tx.address&0xFF); 

    //     // Calcul du checksum
    //     byte cksum = 0;
    //     for (int i = 0; i < 7; ++i)
    //         cksum = cksum ^ data[i] ^ (data[i] >> 4);
    //     data[1] = data[1] + (cksum & 0x0F);

    //     // Obsufscation
    //     for (int i = 1; i < 7; ++i)
    //         data[i] = data[i] ^ data[i - 1];

    //     // Emission wakeup, synchro hardware et software
    //     digitalWrite(m_pinRX, 1);
    //     delayMicroseconds(k_tempo_wakeup_pulse);
    //     digitalWrite(m_pinRX, 0);
    //     delayMicroseconds(k_tempo_wakeup_silence);

    //     for (int i = 0; i < 7; ++i)
    //     {
    //         digitalWrite(m_pinRX, 1);
    //         delayMicroseconds(DELAY_SYNCHRO_HW);
    //         digitalWrite(m_pinRX, 0);
    //         delayMicroseconds(DELAY_SYNCHRO_HW);
    //     }

    //     digitalWrite(m_pinRX, 1);
    //     delayMicroseconds(DELAY_SYNCHRO_SW);
    //     digitalWrite(m_pinRX, 0);
    //     delayMicroseconds(DELAY_HALF_SYMBOL);

    //     // Emission des donnees
    //     for (int i = 0; i < 56; ++i)
    //     {
    //         byte bit_to_transmit = (data[i / 8] >> (7 - i % 8)) & 0x01;
    //         if (bit_to_transmit == 0)
    //         {
    //             digitalWrite(m_pinRX, 1);
    //             delayMicroseconds(DELAY_HALF_SYMBOL);
    //             digitalWrite(m_pinRX, 0);
    //             delayMicroseconds(DELAY_HALF_SYMBOL);
    //         }
    //         else
    //         {
    //             digitalWrite(m_pinRX, 0);
    //             delayMicroseconds(DELAY_HALF_SYMBOL);
    //             digitalWrite(m_pinRX, 1);
    //             delayMicroseconds(DELAY_HALF_SYMBOL);
    //         }
    //     }

    //     digitalWrite(m_pinRX, 0);
    //     delayMicroseconds(DELAY_INTERFRAME);
    // 
    return true;
    }
}