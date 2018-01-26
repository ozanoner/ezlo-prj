
#include "mbed.h"
#include <Serial.h>
#include "SerialReaderBase.h"
#include <string>
#include <sstream>

#ifndef SerialReadWrite_h_
#define SerialReadWrite_h_

template<typename T>
Serial& operator<<(Serial& s, T d) {
	std::stringstream ss;
	ss << d;
	s.printf(ss.str().c_str());
	return s;
}

class SerialReadWrite: public SerialReaderBase {

public:
	SerialReadWrite(): esp32(USBTX, USBRX, 9600) {
		esp32.set_flow_control(Serial::Disabled);
		esp32.attach([this]()->void{this->receivedCb();});

/*
		esp32.attach([this]()->void {
			if(this->esp32.readable()) {
				this->esp32 << (char)this->esp32.getc();
			}
		});
*/
	}

	char getc() override { return (char)this->esp32.getc(); }
	bool available() override { return this->esp32.readable(); }
	void write(const std::string& s) {
		esp32 << STX << s << ETX;
	}

private:
	Serial esp32;
};

#endif
