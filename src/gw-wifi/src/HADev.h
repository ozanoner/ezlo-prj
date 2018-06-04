#ifndef HADev_h_
#define HADev_h_

#include <inttypes.h>
#include <vector>

struct HAAttrib_t {
    uint16_t charUuid;
    uint16_t valHandle;
    uint32_t val;
};

struct HADev_t {
    uint8_t devId;
    uint16_t connHandle;
    bool connected;
    std::vector<HAAttrib_t> attribs;
};

#endif