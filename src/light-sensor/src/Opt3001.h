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

#ifndef Opt3001_h_
#define Opt3001_h_

class Opt3001: public Opt3001Base {
public:
	Opt3001(I2C& p): port(p): ready(false) {	}
	bool init() override {
		// default config, datasheet@page 21
		this->ready = this->port.write(I2C_ADDR, {0x01, 0xc4, 0x10}, 3)==0;
		return this->ready;
	}
	float readLux() override {
		// refer to datasheet
		// to command result reg @addr 0x0
		if(!this->port.write(I2C_ADDR, {0x0}, 1))
			throw exception();

		uint8_t resp[2];
		if(!this->port.read(I2C_ADDR, resp, 2))
			throw exception();
		int exp = (0xf0 & resp[1])>>4;
		int man = (0x0f & resp[1])<<8 & resp[0];
		return 0.01f * pow(2, exp) * man;
	}
private:
	I2C port;
	const static uint8_t I2C_ADDR = 0x44 << 1; // mbed requirement
	bool ready;
};

#endif
