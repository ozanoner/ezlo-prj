#ifndef HAStateReadWrite_h_
#define HAStateReadWrite_h_


#include "HAStateReadonly.h"

template <typename Tval>
class HAStateReadWrite: HAStateReadonly<Tval> {
protected:
    Tval newval;

public:
    void setValue(Tval n)=0;
};

#endif