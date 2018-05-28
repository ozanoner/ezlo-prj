#ifndef HADevShadow_h_
#define HADevShadow_h_

#include <inttypes.h>
#include <BLEProtocol.h>
#include <UUID.h>
#include <string>
#include "HABleServiceDefs.h"
#include <map>
#include <Gap.h>
#include "json/src/json.hpp"
using json = nlohmann::json;

class BleConn;

class HADevShadow
{
  friend class BleConn;

public:
  using ResponseCallbackT = void (*)(const char *);

protected:
  struct ConnInfoT
  {
    GattAttribute::Handle_t connectionHandle;
    // <charUUID, characteristic>
    map<UUID::ShortUUIDBytes_t, std::shared_ptr<const DiscoveredCharacteristic>> characteristics;
    ble::attribute_handle_t lastCccdHandle;
  };

  bool connected;
  std::unique_ptr<ConnInfoT> connInfo;

  BLEProtocol::AddressBytes_t address;
  UUID::ShortUUIDBytes_t serviceId;

  void onHVX(const GattHVXCallbackParams *p);
  void onDataRead(const GattReadCallbackParams *p);
  virtual void onDataWritten(const GattWriteCallbackParams *p) {}

  void onConnected(Gap::Handle_t);
  void onDisconnected();

  ResponseCallbackT respCb;
  void sendResponse(GattAttribute::Handle_t handle, const uint8_t* data, uint8_t offset=0);
public:
  HADevShadow() : respCb(nullptr), connected(false) {}
  // service id as deviceType
  UUID::ShortUUIDBytes_t deviceType() const { return serviceId; }
  // last byte of mac address
  uint8_t devId() const { return this->address[DEV_ID_IDX]; }

  bool isConnected() const { return this->connected; }

  virtual std::string toString() {}

  void read(UUID::ShortUUIDBytes_t cUuid);
  void write(UUID::ShortUUIDBytes_t cUuid, int value);

  void setResponseCallback(ResponseCallbackT cb)
  {
    this->respCb = cb;
  }
};

void HADevShadow::onConnected(Gap::Handle_t h)
{
  this->connInfo.reset(new ConnInfoT());
  this->connInfo->connectionHandle = h;
  this->connected = true;
}

void HADevShadow::onDisconnected()
{
  this->connected = false;
}

void HADevShadow::read(UUID::ShortUUIDBytes_t cUuid)
{
  if (!this->connected)
    return;

  auto l = this->connInfo->characteristics;
  if (l.find(cUuid) != l.end())
  {
    auto cc = l[cUuid];
    if (cc->getProperties().read())
    {
      cc->read();
    }
  }
}

void HADevShadow::write(UUID::ShortUUIDBytes_t cUuid, int value)
{
  if (!this->connected)
    return;

  auto l = this->connInfo->characteristics;
  if (l.find(cUuid) != l.end())
  {
    auto cc = l[cUuid];
    if (cc->getProperties().write())
    {
      // check characteristic uuid and convert write type
    }
  }
}

void HADevShadow::onHVX(const GattHVXCallbackParams *p)
{
  if (this->respCb == nullptr)
    return;
  // p->handle -> characteristic->getValueHandle()
  // p->len and p->data
  // DPRN("hvx_handler: conn:%x attr:%x data:%x\n", p->connHandle, p->handle, *p->data);

  this->sendResponse(p->handle, p->data);
}

void HADevShadow::onDataRead(const GattReadCallbackParams *p) {
  if (this->respCb == nullptr)
    return;
  if(p->status != BLE_ERROR_NONE) 
    return;

  this->sendResponse(p->handle, p->data, p->offset);
  
  // p->handle getValueHandle
  // p->offset, p->len, p->data
}

void HADevShadow::sendResponse(GattAttribute::Handle_t handle, const uint8_t* data, uint8_t offset) {
  json j;
  j["dev"]=this->devId();

  for(auto const& pair: this->connInfo->characteristics) {
    if(pair.second->getValueHandle()==handle) {
      // found
      j["state"]=pair.first;
      switch (pair.first)
      {
        case BUTTON_STATE_CHARACTERISTIC_UUID:
          j["val"]=data[offset];
          break;
      
        default:
          break;
      }
      break; // DONE
    }
  }

  this->respCb(j.dump().c_str());
}


#endif
