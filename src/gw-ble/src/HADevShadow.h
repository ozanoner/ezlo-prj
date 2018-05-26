#ifndef HADevShadow_h_
#define HADevShadow_h_

#include <inttypes.h>
#include <BLEProtocol.h>
#include <UUID.h>
#include <vector>
#include <string>
#include "HABleServiceDefs.h"


class BleConn;

class HADevShadow {
friend class BleConn;
protected:
    bool connected;

    BLEProtocol::AddressBytes_t address;
    UUID::ShortUUIDBytes_t serviceId;
    vector<DiscoveredCharacteristic> characteristics;
    // temp var to be used with notification registration
    ble::attribute_handle_t lastCccdHandle;
    // ble connection identifier, set with onConnected
    GattAttribute::Handle_t connectionHandle;

public:
    // service id as deviceType
    UUID::ShortUUIDBytes_t deviceType() const { return serviceId; }
    // last byte of mac address
    uint8_t devId() const { return this->address[5]; }

    bool isConnected() const { return this->connected; }
    virtual std::string toString() { }
    // notification
    virtual void onHVX(const GattHVXCallbackParams *p) { }
    // query result
    virtual void onDataRead(const GattReadCallbackParams *p) { }
    // write result
    virtual void onDataWritten(const GattWriteCallbackParams *p) { }
};



#endif
