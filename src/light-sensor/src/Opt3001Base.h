
#ifndef Opt3001Base_h_
#define Opt3001Base_h_

class Opt3001Base {
public:
	virtual bool init() { return true; }
	virtual float readLux()  { return 1.0f; }
};

#endif
