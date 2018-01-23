
#ifndef Opt3001Dummy_h_
#define Opt3001Dummy_h_

class Opt3001Dummy: public Opt3001Base {
public:
	bool init() override { return true; }
	float readLux() override { return 1.0f; }
};

#endif
