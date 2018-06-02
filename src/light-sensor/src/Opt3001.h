/*
references:
https://github.com/energia/Energia/blob/master/libraries/OPT3001/OPT3001.cpp
https://github.com/MyOctopus/sensors-samples/blob/master/python-mraa-samples/human_eye_lux_opt3001.py
https://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git/tree/drivers/iio/light/opt3001.c
http://www.ti.com/lit/ds/symlink/opt3001.pdf
https://docs.mbed.com/docs/mbed-os-api-reference/en/5.1/APIs/interfaces/digital/I2C/
*/

/*
i2c addressing: (datasheet @page 12)
0x44 - gnd
0x45 - vdd
0x46
0x47
*/
#include "mbed.h"
#include "Opt3001Base.h"
#include "SEGGER_RTT.h"


#ifndef Opt3001_h_
#define Opt3001_h_

class Opt3001: public Opt3001Base {
public:
	Opt3001(I2C& p): port(p), ready(false) {	}
	bool init() override {
		// default config, datasheet@page 21
		char cmd[3]= {0x01, 0xc4, 0x10};
		this->ready = this->port.write(I2C_ADDR, cmd, sizeof(cmd))==0;
		DPRN("isReady: %d\n", this->ready);
		return this->ready;
	}
	// float readLux() override {
	uint16_t readLux() override {
		if(!this->ready)
			return -1;
		// refer to datasheet
		// to command result reg @addr 0x0
		char cmd[1]= {0x0};
		if(this->port.write(I2C_ADDR, cmd, 1)) {
			DPRN("write failed\n");
			return -1;
		}

		char resp[2];
		if(this->port.read(I2C_ADDR, resp, 2)) {
			DPRN("read failed\n");
			return -1;
		}
		DPRN("lux: %x-%x", resp[0], resp[1]);
		return *reinterpret_cast<uint16_t*>(resp);
		// int exp = (0xf0 & (uint8_t)resp[1])>>4;
		// int man = (0x0f & (uint8_t)resp[1])<<8 & (uint8_t)resp[0];
		// return 0.01f * pow(2, exp) * man;
	}
private:
	// https://os.mbed.com/handbook/I2C
	I2C& port;
	const static uint8_t I2C_ADDR = 0x44 << 1; // mbed requirement
	bool ready;
};

#endif
