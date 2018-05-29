
#ifndef STPM01Driver_h_
#define STPM01Driver_h_

#include "PinNames.h"
#include "HAHardwareDefs.h"
#include "SPI.h"
#include "SEGGER_RTT.h"


extern "C" {

int goodParity(uint8_t* bp) {
    uint8_t parity = *bp;
    for(int i=1; i<4; i++) {
        parity ^= bp[i];
    }
    parity ^= parity<<4, parity &= 0xf0;
    return parity == 0xf0;  
}

}

class STPM01Driver: private NonCopyable<STPM01Driver> {
protected:
    SPI& spi;
    DigitalOut& cs;
    DigitalOut& syn;
    uint32_t data[8];
public:
    STPM01Driver(SPI& s1, DigitalOut& cs1, DigitalOut& syn1): 
        spi(s1), cs(cs1), syn(syn1) {    }

    void init() {
        spi.format(8,3); // 8bits, pol=1, phase=1
        spi.frequency(32000000);
    }
    void read() {
        syn=0; cs=0; syn=1; // latching

        uint32_t val = 0;
        for(int i=0; i<8; i++) {
            val = (uint8_t)spi.write(0);
            val |= (uint32_t)((uint8_t)spi.write(0)) << 8;
            val |= (uint32_t)((uint8_t)spi.write(0)) << 16;
            val |= (uint32_t)((uint8_t)spi.write(0)) << 24;
            data[i] = val;
        }

        cs=1; // stop SPI

        DPRN("new read\n");
        for(int i=0; i<8; i++) {
            DPRN("%x ", data[i]);
            uint8_t* bp = (uint8_t*)(data+i);
            if(goodParity(bp)) {
                DPRN("(good parity)\n");
            }
            else {
                DPRN("(bad parity)\n");
            }
        }
    }
};


#endif
