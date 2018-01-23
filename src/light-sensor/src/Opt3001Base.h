
#ifndef Opt3001Base_h_
#define Opt3001Base_h_

class Opt3001Base {
public:
	virtual bool init()=0;
	virtual float readLux()=0;
};

#endif
