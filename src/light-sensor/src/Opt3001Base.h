
#ifndef Opt3001Base_h_
#define Opt3001Base_h_

// for testing purpose
class Opt3001Base {
public:
	virtual bool init() { return true; }
	virtual uint16_t readLux()  { return 1; }
	// virtual float readLux()  { return 1.0f; }
};

#endif
