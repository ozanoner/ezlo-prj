
#include <vector>
#include <string>
#include "SerialReaderBase.h"


#ifndef SerialData_h_
#define SerialData_h_

#define STX '!'
#define ETX '@'



class SerialData {
public:
	using NewDataCbT = std::function<void(std::string)>;

	SerialData(SerialReaderBase* r):
	reader(r), started(false), err(false), buff(64) {
		this->reader->attach([this]()->void{this->processData();});
	}

	void attach(NewDataCbT cb) { this->newDataCb = cb; }

private:
	SerialReaderBase* reader;
	bool started;
	bool err;

	NewDataCbT newDataCb;
	std::vector<char> buff;

	void processData() {
		if(!this->reader->available())
			return;
		char c = this->reader->getc();
		if(c==ETX) {
			if(!this->err)
				this->newDataCb(std::string(buff.begin(), buff.end()));
			this->err=this->started=false;
			this->buff.clear();
		}
		else if(!this->err) {
			if(c==STX)
				this->started=true;
			else if(this->started)
				this->buff.push_back(c);
		}
	}
};

#endif
