
#ifndef BleConn_h_
#define BleConn_h_

#include <mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "ble/DiscoveredCharacteristic.h"
#include "ble/DiscoveredService.h"
#include "BLEProtocol.h"
#include <cstdarg>
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


Gap::Address_t WHITE_LIST[8] = {
{HOME_ID, 0x00, 0x00, 0xE1, 0x01, BTN1_ID2},
{HOME_ID, 0x00, 0x00, 0xE1, 0x01, BTN2_ID3},
{HOME_ID, 0x00, 0x00, 0xE1, 0x01, LED_ID4},
{HOME_ID, 0x00, 0x00, 0xE1, 0x01, RGBLED_ID5},
{HOME_ID, 0x00, 0x00, 0xE1, 0x01, PLUG_ID6},
{HOME_ID, 0x00, 0x00, 0xE1, 0x01, DIMMER_ID7},
{HOME_ID, 0x00, 0x00, 0xE1, 0x01, LS_ID8}
};


using namespace std;

using DebugPrintFuncT = void (*)(const char*, va_list);

std::vector<DiscoveredCharacteristic> discoveredChars;
std::map<UUID::ShortUUIDBytes_t, GattAttribute::Handle_t> cccdList;

class BleConn : private mbed::NonCopyable<BleConn> {
private:
    BLE& ble;
    events::EventQueue& evq;
    bool isProcessing;

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

    static bool isTrackedChar(UUID::ShortUUIDBytes_t charU);

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
    BLE_LOG("[info] BleConn::userCommand received (%s)", data);
    try {
        auto root = json::parse(data);
        Gap::Handle_t connHandle = root["conn"]; // connection handle
        // value handle
        GattAttribute::Handle_t handle = root["handle"]; 
        uint8_t cmd = root["cmd"]; // get=0 | set=1
        uint32_t val=0;
        uint16_t size=0;
        if(cmd) {
            val = root["val"]; // state specific value if cmd=1
            size = root["size"]; 
        }
        if(cmd) {
            if(size>0) {
                ble_error_t ret = this->ble.gattClient().write(GattClient::GATT_OP_WRITE_REQ, 
                    connHandle, handle, size, reinterpret_cast<const uint8_t*>(&val));
                if(ret!=BLE_ERROR_NONE)
                    BLE_LOG("[error] BleConn::userCommand write failed (%d)", ret);
            }
            else {
                BLE_LOG("[error] BleConn::userCommand invalid command");
            }

        } else {
            ble_error_t ret = this->ble.gattClient().read(connHandle, handle, 0);
            if(ret!=BLE_ERROR_NONE)
                BLE_LOG("[error] BleConn::userCommand read failed");
        }
    }
    catch(...) {
        BLE_LOG("[error] BleConn::userCommand exception occured");
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
        BLE_LOG("[Error] BLE::init (%d)", error);
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
    BLE_LOG("[Warning] BleConn::onTimeout (%d)", source);
    // switch (source) {
    //     case Gap::TIMEOUT_SRC_ADVERTISING:
    //         BLE_LOG("[Warning] Stopped advertising early due to timeout parameter");
    //         break;
    //     case Gap::TIMEOUT_SRC_SCAN:
    //         BLE_LOG("[Warning] Stopped scanning early due to timeout parameter");
    //         break;
    //     case Gap::TIMEOUT_SRC_CONN:
    //         BLE_LOG("[Warning] Failed to connect after scanning %d advertisements");
    //         break;
    //     default:
    //         BLE_LOG("[Error] Unexpected timeout");
    //         break;
    // }
}

void BleConn::onInitComplete(BLE::InitializationCompleteCallbackContext *event)
{
    if (event->error)
    {
        BLE_LOG("[Error] BleConn::onInitComplete input event");
        return;
    }

    ble_error_t err;

    ble.gap().setAddress(Gap::AddressType_t::PUBLIC, BLE_NW_ADDR);
    if(ble.gap().setTxPower(4)!=BLE_ERROR_NONE) {
        BLE_LOG("[error] setTxPower");
    }
    ble.gap().setScanParams();
    /* all calls are serialised on the user thread through the event queue */
    ble.gap().onConnection(this, &BleConn::onConnected);
    ble.gap().onDisconnection(this, &BleConn::onDisconnected);
    
    Gap::Whitelist_t whitelist {reinterpret_cast<BLEProtocol::Address_t*>(WHITE_LIST), 7, 8};
    err = ble.gap().setWhitelist(whitelist);
    if(err != BLE_ERROR_NONE) {
        BLE_LOG("[error] setWhitelist");
    }

    ble.gattClient().onHVX(makeFunctionPointer(this, &BleConn::onHVX));
    ble.gattClient().onDataRead(makeFunctionPointer(this, &BleConn::onDataRead));
    ble.gattClient().onDataWritten(makeFunctionPointer(this, &BleConn::onDataWritten));
    ble.gattClient().onServiceDiscoveryTermination(
        makeFunctionPointer(this, &BleConn::onServiceDiscoveryTermination));

    // BLE_LOG("[Info] BleConn::onInitComplete completed. Scanning..");
    evq.call(this, &BleConn::scan);
}

void BleConn::scan()
{
    BLE_LOG("[Info] Gap::startScan");
    ble_error_t err = ble.gap().startScan(this, &BleConn::onAdDetected);
    if(err != BLE_ERROR_NONE) {
        BLE_LOG("[Error] Gap::startScan (%d)", err);
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

    // Gap::ConnectionParams_t p {0x0006, 0x0C80, 0x01F3, 0x0C80};
    
    ble_error_t err = ble.gap().connect(params->peerAddr, 
            BLEProtocol::AddressType_t::PUBLIC, nullptr, nullptr);
            // BLEProtocol::AddressType_t::PUBLIC, &p, nullptr);
    if(err != BLE_ERROR_NONE) {
        isProcessing = false;
        BLE_LOG("[Error] Gap::connect");
    }
}

void BleConn::onConnected(const Gap::ConnectionCallbackParams_t *conn)
{
    BLE_LOG("[Info] BleConn::onConnected (%x)", conn->handle);

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
    BLE_LOG("[Warning] BleConn::onDisconnected (%x)", event->reason);

    json j;
    j["evt"] = BLE_EVENT_onDisconnected;
    j["conn"] = event->handle;
    this->respCb(j.dump().c_str());

}

bool BleConn::isTrackedChar(UUID::ShortUUIDBytes_t charU) {
    return (charU & 0xA100)==0xA100 || (charU & 0xA200)==0xA200 ;
}

void BleConn::onServiceDiscovery(const DiscoveredService* service)
{
    auto shortUUID = service->getUUID().getShortUUID();
    if(BleConn::isTrackedChar(shortUUID)) {
        BLE_LOG("[Info] DISCOVERED SERVICE(%x)", shortUUID);
    }

    // BLE_LOG("[Info] BleConn::onServiceDiscovery");
    // // short uuid expected
    // if (service->getUUID().shortOrLong() == UUID::UUID_TYPE_SHORT)
    // {
    //     auto shortUUID = service->getUUID().getShortUUID();
    //     if(shortUUID == BUTTON1_SERVICE_UUID 
    //         || shortUUID == BUTTON1_SERVICE_UUID
    //         || shortUUID == BUTTON2_SERVICE_UUID
    //         || shortUUID == LIGHT_SERVICE_UUID
    //         || shortUUID == LED_SERVICE_UUID
    //         || shortUUID == RGBLED_SERVICE_UUID
    //         || shortUUID == PLUG_SERVICE_UUID
    //         || shortUUID == DIMMER_SERVICE_UUID) 
    //     {
    //         // this->actDevice->serviceId = shortUUID;
    //         BLE_LOG("[Info] DISCOVERED SERVICE(%x)", shortUUID);
    //     }
    //     else {
    //         // TODO: disconnect & disable to reconnect
    //         // BLE_LOG("[Warning] INVALID SERVICE(%x)", shortUUID);
    //         // this->ble.gattClient().terminateServiceDiscovery();
    //     }
    // }
    // else {
    //     // this->ble.gattClient().terminateServiceDiscovery();
    // }
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
   
}

void BleConn::onCharDescriptorDisc(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t* p)
{
    BLE_LOG("[Info] BleConn::onCharDescriptorDisc");
    if (p->descriptor.getUUID() == BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG)
    {
        BLE_LOG("[Info] CCCD found");
        cccdList[p->characteristic.getUUID().getShortUUID()] = p->descriptor.getAttributeHandle(); 
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
    BLE_LOG("[Info] BleConn::onHVX (data:%x)", *(p->data));

    json j;
    j["evt"] = BLE_EVENT_onHVX;
    j["conn"] = p->connHandle;
    j["valh"] = p->handle;
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
    BLE_LOG("[Info] BleConn::onDataRead (data:%x)", *(p->data));

    if(p->status == BLE_ERROR_NONE) {
        json j;
        j["evt"] = BLE_EVENT_onDataRead;
        j["conn"] = p->connHandle;
        j["valh"] = p->handle;
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
    BLE_LOG("[Info] BleConn::onDataWritten (status:%d)", p->status);
}

#endif