
#ifndef BleConn_h_
#define BleConn_h_

#include <mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "ble/DiscoveredCharacteristic.h"
#include "ble/DiscoveredService.h"
#include "BLEProtocol.h"
#include <cstdarg>
// #include "HADevShadow.h"
#include <vector>
#include <map>


#include "json/src/json.hpp"
using json = nlohmann::json;


#define BLE_LOG_ENABLED

#ifdef BLE_LOG_ENABLED
#define BLE_LOG(format, ...) dbg (format, ##__VA_ARGS__)
#else
#define BLE_LOG(format, ...)
#endif

#include "HABleEvents.h"
#include "HABleServiceDefs.h"
#include "HAProvision.h"
#define DEV_PROVISION_ID GW_ID1

static const uint8_t DEVICE_NAME[] = "GW_ID1";
static const Gap::Address_t BLE_NW_ADDR = {HOME_ID, 0x00, 0x00, 0xE1, 0x01, DEV_PROVISION_ID};

using namespace std;

using DebugPrintFuncT = void (*)(const char*, va_list);

std::vector<DiscoveredCharacteristic> discoveredChars;
std::map<UUID::ShortUUIDBytes_t, GattAttribute::Handle_t> cccdList;

class BleConn : private mbed::NonCopyable<BleConn> {
private:
    BLE& ble;
    events::EventQueue& evq;
    bool isProcessing;
    // int scanCnt;


    // map devices over connection handles

    // std::shared_ptr<HAAttrib> actDevice;

    void scheduleBleEvents(BLE::OnEventsToProcessCallbackContext *context);

    // connection & initialization
    void onTimeout(const Gap::TimeoutSource_t source);
    void onInitComplete(BLE::InitializationCompleteCallbackContext *event);
    void scan();
    void onAdDetected(const Gap::AdvertisementCallbackParams_t *params);
    void onConnected(const Gap::ConnectionCallbackParams_t *connection_event);
    void onServiceDiscovery(const DiscoveredService *service);
    void onServiceDiscoveryTermination(Gap::Handle_t connectionHandle);
    void onCharacteristicDiscovery(const DiscoveredCharacteristic *cp);
    void onCharDescriptorDisc(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t *p);
    void onCharDescriptorDiscTermination(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *p);
    void onDisconnected(const Gap::DisconnectionCallbackParams_t *event);

    // data communication
    void onHVX(const GattHVXCallbackParams *p);
    void onDataRead(const GattReadCallbackParams *p);
    void onDataWritten(const GattWriteCallbackParams *p);
    DebugPrintFuncT debugPrint;
    void dbg(const char*, ...);

    Callback<void(const char*)> respCb;

    // void printDeviceCharacteristics(const HADevShadow& dev);
public:
    BleConn(EventQueue& evq) : evq(evq), ble(BLE::Instance()),
                               isProcessing(false), //scanCnt(0),
                               debugPrint(nullptr) 
    {    }

    void init(Callback<void(const char*)>, DebugPrintFuncT);

    void setDebugPrintCallback(DebugPrintFuncT d) {
        this->debugPrint = d;
    }

    void userCommand(const char*);
};


void BleConn::userCommand(const char* data) {
    auto root = json::parse(data);

    Gap::Handle_t connHandle = root["conn"]; // connection handle
    GattAttribute::Handle_t charHandle = root["char"]; // characteristic handle
    uint8_t cmd = root["cmd"]; // get=0 | set=1
    uint32_t val=0;
    uint16_t size=0;
    if(cmd) {
        val = root["val"]; // state specific value if cmd=1
        size = root["size"]; // state specific value if cmd=1
    }
    if(cmd) {
        if(size>0) {
            this->ble.gattClient().write(GattClient::GATT_OP_WRITE_REQ, 
                connHandle, charHandle, size, reinterpret_cast<const uint8_t*>(&val));
        }
        else {
            BLE_LOG("[error] BleConn::userCommand invalid command");
        }

    } else {
        this->ble.gattClient().read(connHandle, charHandle, 0);
    }
}

void BleConn::dbg(const char* fmt, ...) {
    if(this->debugPrint == nullptr)
        return;
    va_list args;
    va_start(args, fmt);
    this->debugPrint(fmt, args);
    va_end(args);
}


void BleConn::init(Callback<void(const char*)> respCb, DebugPrintFuncT debugpf=nullptr) {

    if (ble.hasInitialized())
        return;

    this->debugPrint = debugpf;
    this->respCb = respCb;

    ble.onEventsToProcess(
        makeFunctionPointer(this, &BleConn::scheduleBleEvents));
    ble.gap().onTimeout(
        makeFunctionPointer(this, &BleConn::onTimeout));

    ble_error_t error = ble.init(this, &BleConn::onInitComplete);
    if (error) {
        BLE_LOG("[Error] BLE::init.");
        return;
    }
}

void BleConn::scheduleBleEvents(BLE::OnEventsToProcessCallbackContext *context)
{
    evq.call(mbed::callback(&context->ble, &BLE::processEvents));
}

/** called if timeout is reached during advertising, scanning
     *  or connection initiation */
void BleConn::onTimeout(const Gap::TimeoutSource_t source)
{
    switch (source) {
        case Gap::TIMEOUT_SRC_ADVERTISING:
            BLE_LOG("[Warning] Stopped advertising early due to timeout parameter");
            break;
        case Gap::TIMEOUT_SRC_SCAN:
            BLE_LOG("[Warning] Stopped scanning early due to timeout parameter");
            break;
        case Gap::TIMEOUT_SRC_CONN:
            BLE_LOG("[Warning] Failed to connect after scanning %d advertisements");
            // evq.call(this, &BleConn::print_performance);
            // evq.call(this, &BleConn::demo_mode_end);
            break;
        default:
            BLE_LOG("[Error] Unexpected timeout");
            break;
    }
}

void BleConn::onInitComplete(BLE::InitializationCompleteCallbackContext *event)
{
    if (event->error)
    {
        BLE_LOG("[Error] BleConn::onInitComplete input event");
        return;
    }

    ble.gap().setAddress(Gap::AddressType_t::PUBLIC, BLE_NW_ADDR);
    
    /* all calls are serialised on the user thread through the event queue */
    ble.gap().onConnection(this, &BleConn::onConnected);
    ble.gap().onDisconnection(this, &BleConn::onDisconnected);

    ble.gattClient().onHVX(
        makeFunctionPointer(this, &BleConn::onHVX));

    ble.gattClient().onDataRead(
        makeFunctionPointer(this, &BleConn::onDataRead));

    ble.gattClient().onDataWritten(
        makeFunctionPointer(this, &BleConn::onDataWritten));

    ble.gattClient().onServiceDiscoveryTermination(
        makeFunctionPointer(this, &BleConn::onServiceDiscoveryTermination));


    // timeout: 0->disabled
    if (ble.gap().setScanParams(800, 400, 0, true) != BLE_ERROR_NONE) {
        BLE_LOG("[Error] Gap::setScanParams");
        return;
    }

    // BLE_LOG("[Info] BleConn::onInitComplete completed. Scanning..");
    evq.call(this, &BleConn::scan);
    // this->scan();
}

void BleConn::scan()
{
    BLE_LOG("[Info] Gap::startScan");
    if(ble.gap().startScan(this, &BleConn::onAdDetected)!= BLE_ERROR_NONE) {
        BLE_LOG("[Error] Gap::startScan");
        return;
    }
}

void BleConn::onAdDetected(const Gap::AdvertisementCallbackParams_t *params)
{
    if (isProcessing) {
        // BLE_LOG("[Info] BleConn::onAdDetected. isProcessing..");
        return;
    }
    
    if (params->peerAddr[HOME_ID_IDX] != HOME_ID) {
        // BLE_LOG("[Info] BleConn::onAdDetected. invalid home_id (%hhx)", 
        //     params->peerAddr[HOME_ID_IDX]);
        return;
    }
    isProcessing = true;

    BLE_LOG("[Info] Gap::connect");
    if(ble.gap().connect(params->peerAddr, 
            BLEProtocol::AddressType_t::PUBLIC, nullptr, nullptr)) {
        isProcessing = false;
        BLE_LOG("[Error] Gap::connect");
    }
}

void BleConn::onConnected(const Gap::ConnectionCallbackParams_t *conn)
{
    BLE_LOG("[Info] BleConn::onConnected (%x)", conn->handle);

    // this->actDevice = std::make_shared<HAAttrib>();
    // this->actDevice->peerAddr = conn->peerAddr[DEV_ID_IDX];
    // this->actDevice->connHandle = conn->handle;

    // auto devId = conn->peerAddr[DEV_ID_IDX];
    // if(this->deviceExists(devId)) {
    //     this->actDevice = this->devicesById[devId];
    // }
    // else {
    //     this->actDevice = std::make_shared<HADevShadow>();
    //     std::memcpy(this->actDevice->address, conn->peerAddr, BLEProtocol::ADDR_LEN);
    //     this->actDevice->setResponseCallback(this->respCb);
    //     this->devicesById.emplace(devId, this->actDevice);
    //     this->devices.emplace(conn->handle, this->actDevice);
    // }

    // this->actDevice->onConnected(conn->handle);

    json j;
    j["evt"] = BLE_EVENT_onConnected;
    j["peer"] = conn->peerAddr[DEV_ID_IDX];
    j["conn"] = conn->handle;
    this->respCb(j.dump().c_str());

    ble.gattClient().launchServiceDiscovery(conn->handle,
            makeFunctionPointer(this, &BleConn::onServiceDiscovery),
            makeFunctionPointer(this, &BleConn::onCharacteristicDiscovery));
}

void BleConn::onDisconnected(const Gap::DisconnectionCallbackParams_t *event)
{
    BLE_LOG("[Warning] BleConn::onDisconnected");

    json j;
    j["evt"] = BLE_EVENT_onDisconnected;
    j["conn"] = event->handle;
    this->respCb(j.dump().c_str());

    // if(this->deviceExists(event->handle)) {
    //     this->devices[event->handle]->onDisconnected();
    // }
}


void BleConn::onServiceDiscovery(const DiscoveredService* service)
{
    BLE_LOG("[Info] BleConn::onServiceDiscovery");
    // short uuid expected
    if (service->getUUID().shortOrLong() == UUID::UUID_TYPE_SHORT)
    {
        auto shortUUID = service->getUUID().getShortUUID();
        if(shortUUID == BUTTON1_SERVICE_UUID 
            || shortUUID == BUTTON1_SERVICE_UUID
            || shortUUID == BUTTON2_SERVICE_UUID
            || shortUUID == LIGHT_SERVICE_UUID
            || shortUUID == LED_SERVICE_UUID
            || shortUUID == RGBLED_SERVICE_UUID
            || shortUUID == PLUG_SERVICE_UUID
            || shortUUID == DIMMER_SERVICE_UUID) 
        {
            // this->actDevice->serviceId = shortUUID;
            BLE_LOG("[Info] DISCOVERED SERVICE(%x)", shortUUID);
        }
        else {
            // TODO: disconnect & disable to reconnect
            // BLE_LOG("[Warning] INVALID SERVICE(%x)", shortUUID);
            // this->ble.gattClient().terminateServiceDiscovery();
        }
    }
    else {
        // this->ble.gattClient().terminateServiceDiscovery();
    }
}

void BleConn::onServiceDiscoveryTermination(Gap::Handle_t connectionHandle)
{
    BLE_LOG("[Info] BleConn::onServiceDiscoveryTermination");

    for(const auto& c: discoveredChars) {
        c.discoverDescriptors(
                makeFunctionPointer(this, &BleConn::onCharDescriptorDisc),
                makeFunctionPointer(this, &BleConn::onCharDescriptorDiscTermination));    
    }
    discoveredChars.clear();
   
    isProcessing = false;
    evq.call(this, &BleConn::scan);
}

// void BleConn::printDeviceCharacteristics(const HADevShadow& dev) {
//     BLE_LOG("[Info] device (%x)", dev.devId());
//     for(const auto& p: dev.connInfo->characteristics) {
//         BLE_LOG("[Info] characteristic: %x", p.first);
//     }
// }


// https://os.mbed.com/docs/v5.7/mbed-os-api-doxy/class_discovered_characteristic.html
void BleConn::onCharacteristicDiscovery(const DiscoveredCharacteristic* discChar)
{
    BLE_LOG("[Info] BleConn::onCharacteristicDiscovery");

    auto shortUUID = discChar->getUUID().getShortUUID();
    json j;
    j["evt"] = BLE_EVENT_onCharacteristicDiscovery;
    j["conn"] = discChar->getConnectionHandle();
    j["char"] = shortUUID; // for read
    j["valh"] = discChar->getValueHandle(); // for write
    this->respCb(j.dump().c_str());

    if(discChar->getProperties().notify())
        discoveredChars.push_back(*discChar);


    // auto device = this->getDevice(discChar->getConnectionHandle());
    // if(device == nullptr) {
    //     // not in the devices map
    //     BLE_LOG("[Error] no device with that connection handle");
    //     return;
    // }

    // auto shortUUID = discChar->getUUID().getShortUUID();
    // // bool continueWithDesc=false;
    // switch(device->deviceType()) {
    //     case BUTTON1_SERVICE_UUID:
    //         if(shortUUID == BUTTON_STATE_CHARACTERISTIC_UUID
    //             || shortUUID == BATTERY_STATE_CHARACTERISTIC_UUID) 
    //         {
    //             BLE_LOG("[Info] BleConn::onCharacteristicDiscovery. button or battery characteristic");
    //             device->connInfo->characteristics.emplace(shortUUID, 
    //                 std::make_shared<const DiscoveredCharacteristic>(std::move(*discChar)));
    //             device->notifyList->push_back(device->connInfo->characteristics[shortUUID]);
    //             // continueWithDesc = true;
    //             printDeviceCharacteristics(*device);
    //         }
    //     break;
    //     case DIMMER_SERVICE_UUID:
    //         if(shortUUID == DIMMER_STATE_CHARACTERISTIC_UUID) 
    //         {
    //             BLE_LOG("[Info] BleConn::onCharacteristicDiscovery. dimmerstate characteristic");
    //             device->connInfo->characteristics.emplace(shortUUID, 
    //                 std::make_shared<const DiscoveredCharacteristic>(std::move(*discChar)));
    //             device->notifyList->push_back(device->connInfo->characteristics[shortUUID]);
    //             // continueWithDesc = true;
    //             printDeviceCharacteristics(*device);
    //         }
    //     break;
    //     case LIGHT_SERVICE_UUID:
    //         if(shortUUID == LIGHT_STATE_CHARACTERISTIC_UUID) 
    //         {
    //             BLE_LOG("[Info] BleConn::onCharacteristicDiscovery. light characteristic");
    //             device->connInfo->characteristics.emplace(shortUUID, 
    //                 std::make_shared<const DiscoveredCharacteristic>(std::move(*discChar)));
    //             device->notifyList->push_back(device->connInfo->characteristics[shortUUID]);
    //             // continueWithDesc = true;
    //             printDeviceCharacteristics(*device);
    //         }
    //     break;
        
    //     default: // unknown device type??
    //     break;
    // }
   
}

void BleConn::onCharDescriptorDisc(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t* p)
{
    BLE_LOG("[Info] BleConn::onCharDescriptorDisc");
    if (p->descriptor.getUUID() == BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG)
    {
        BLE_LOG("[Info] CCCD found");
        cccdList[p->characteristic.getUUID().getShortUUID()] = p->descriptor.getAttributeHandle(); 
        // auto device = this->getDevice(p->characteristic.getConnectionHandle());
        // if(device == nullptr) {
        //     BLE_LOG("[Error] no device with that connection handle");
        //     return;
        // }
        // device->connInfo->lastCccdHandle = p->descriptor.getAttributeHandle();
        // // BLE_LOG("_CCCD found: %02x\n", device->connInfo->lastCccdHandle);
        ble.gattClient().terminateCharacteristicDescriptorDiscovery(p->characteristic);
    }
}

void BleConn::onCharDescriptorDiscTermination(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *p)
{
    BLE_LOG("[Info] BleConn::onCharDescriptorDiscTermination");

    auto it = cccdList.find(p->characteristic.getUUID().getShortUUID());
    if(it != cccdList.end()) {
        uint16_t notification_enabled = 1;
        ble.gattClient().write(
            GattClient::GATT_OP_WRITE_CMD,
            p->characteristic.getConnectionHandle(),
            it->second,
            sizeof(notification_enabled),
            reinterpret_cast<const uint8_t *>(&notification_enabled));
        cccdList.erase(it);
        BLE_LOG("[Info] CCCD notification enabled");
    }
}

// https://docs.mbed.com/docs/ble-api/en/master/api/structGattHVXCallbackParams.html
void BleConn::onHVX(const GattHVXCallbackParams *p)
{
    // _event_queue.call(this, &GwDevice::hvx_send_data, p->connHandle, p->handle, *p->data);
    // this->hvx_send_data(p->connHandle, p->handle, *p->data);
    // BLE_LOG("hvx_handler: conn:%x attr:%x data:%x\n", p->connHandle, p->handle, *p->data);
    BLE_LOG("[Info] BleConn::onHVX (data:%x)", *p->data);

    // auto device = this->getDevice(p->connHandle);
    // if(device == nullptr)
    //     return;
    // device->onHVX(p);

    json j;
    j["evt"] = BLE_EVENT_onHVX;
    j["conn"] = p->connHandle;
    j["char"] = p->handle;
    if(p->len == 4) {
        uint32_t val = *(reinterpret_cast<const uint32_t*>(p->data));
        j["val"] = val; 
    } else {
        j["val"] = *(p->data); 
    }
    this->respCb(j.dump().c_str());
}

void BleConn::onDataRead(const GattReadCallbackParams *p)
{
    // BLE_LOG("onDataRead: conn:%x attr:%x\n", p->connHandle, p->handle);
    // esp32_comm.printf("onDataRead: conn:%x attr:%x\n", p->connHandle, p->handle);
    BLE_LOG("[Info] BleConn::onDataRead (data:%x)", *p->data);

    // auto device = this->getDevice(p->connHandle);
    // if(device == nullptr)
    //     return;
    // device->onDataRead(p);

    if(p->status == BLE_ERROR_NONE) {
        json j;
        j["evt"] = BLE_EVENT_onDataRead;
        j["conn"] = p->connHandle;
        j["char"] = p->handle;
        if(p->len == 4) {
            uint32_t val = *(reinterpret_cast<const uint32_t*>(p->data));
            j["val"] = val; 
        } else {
            j["val"] = *(p->data); 
        }
        this->respCb(j.dump().c_str());
    }
    else {
        BLE_LOG("[error] BleConn::onDataRead (err:%d)", p->error_code);
    }

}

void BleConn::onDataWritten(const GattWriteCallbackParams *p)
{
    // BLE_LOG("onDataWritten: conn:%x attr:%x, status:%d\n", p->connHandle, p->handle, p->status);
    BLE_LOG("[Info] BleConn::onDataWritten (status:%d)", p->status);
    // esp32_comm.printf("onDataWritten: conn:%x attr:%x, status:%d\n", p->connHandle, p->handle, p->status);
    // auto device = this->getDevice(p->connHandle);
    // if(device == nullptr)
    //     return;
    // device->onDataWritten(p);
}

#endif