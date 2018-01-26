
#include <functional>

#ifndef SerialReaderBase_h_
#define SerialReaderBase_h_



class SerialReaderBase {
public:
	using ReceivedCbT = std::function<void()>;

	virtual char getc() { return 0; }
	virtual bool available() {return false; }
	void attach(ReceivedCbT cb) {
		this->receivedCb = cb;
	}
protected:
	ReceivedCbT receivedCb;
};

#endif
