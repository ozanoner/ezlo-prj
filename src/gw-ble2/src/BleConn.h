
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
    void startDescriptorDiscovery();

    // data communication
    void onHVX(const GattHVXCallbackParams *p);
    void onDataRead(const GattReadCallbackParams *p);
    void onDataWritten(const GattWriteCallbackParams *p);

    DebugPrintFuncT debugPrint;
    void dbg(const char*, ...);

    Callback<void(const char*)> respCb;

    static bool isTrackedChar(UUID::ShortUUIDBytes_t charU);

    // void printDeviceCharacteristics(const HADevShadow& dev);

    struct DataPack {
        Gap::Handle_t connHandle;
        GattAttribute::Handle_t handle;
        uint16_t len;
        const uint8_t *data;
        uint8_t eventType;
    };
    void sendDataPack(const DataPack& p);

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
                BLE_LOG("[error] BleConn::userCommand read failed (%d)", ret);
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


    err = ble.gap().setScanParams(800, 400, 0, true);
    if(err != BLE_ERROR_NONE) {
        BLE_LOG("[error] setScanParams (%d)", err);
    }

    // WARNING!! The following code stops multi peripheral connection!!
    // ble.gap().setScanParams();
    // ble.gap().setActiveScanning(true);
    /* all calls are serialised on the user thread through the event queue */
    ble.gap().onConnection(this, &BleConn::onConnected);
    ble.gap().onDisconnection(this, &BleConn::onDisconnected);


    // BLEProtocol::Address_t lightSensorAddr = BLEProtocol::Address_t(BLEProtocol::AddressType::PUBLIC,
    //     {HOME_ID, 0x00, 0x00, 0xE1, 0x01, LS_ID8});
    // BLEProtocol::Address_t addresses[2] = {lightSensorAddr};
    // Gap::Whitelist_t whitelist {addresses, 1, 2};

    // err = ble.gap().setWhitelist(whitelist);
    // if(err != BLE_ERROR_NONE) {
    //     BLE_LOG("[error] setWhitelist");
    // }
    // err = ble.gap().setScanningPolicyMode(Gap::SCAN_POLICY_FILTER_ALL_ADV);
    // if(err != BLE_ERROR_NONE) {
    //     BLE_LOG("[error] setScanningPolicyMode (%d)", err);
    // }

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
    if(this->isProcessing) {
        BLE_LOG("[Info] Gap::startScan exit. in process");
        return;
    }

    ble_error_t err;

    BLE_LOG("[Info] Gap::startScan");

    err = ble.gap().startScan(this, &BleConn::onAdDetected);
    if(err != BLE_ERROR_NONE) {
        // if scanning, err == BLE_ERROR_PARAM_OUT_OF_RANGE
        if(err != BLE_ERROR_PARAM_OUT_OF_RANGE) {
            BLE_LOG("[Error] Gap::startScan (%d)", err);
            err = ble.gap().stopScan();
            if(err != BLE_ERROR_NONE) {
                BLE_LOG("[Error] Gap::stopScan (%d)", err);
            }
        }
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
        BLE_LOG("[Error] Gap::connect (%d)", err);
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

    evq.call(this, &BleConn::scan);
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
}

void BleConn::onServiceDiscoveryTermination(Gap::Handle_t connectionHandle)
{
    BLE_LOG("[Info] BleConn::onServiceDiscoveryTermination");

    // for(const auto& c: discoveredChars) {

    //     BLE_LOG("[Info] starting desc discovery for (%x)", c.getUUID().getShortUUID());
    //     c.discoverDescriptors(
    //             makeFunctionPointer(this, &BleConn::onCharDescriptorDisc),
    //             makeFunctionPointer(this, &BleConn::onCharDescriptorDiscTermination));
    // }
    // discoveredChars.clear();

    this->startDescriptorDiscovery();

    isProcessing = false;
    evq.call(this, &BleConn::scan);
}

void BleConn::startDescriptorDiscovery() {
    if(discoveredChars.size()>0) {
        discoveredChars.back().discoverDescriptors(
            makeFunctionPointer(this, &BleConn::onCharDescriptorDisc),
            makeFunctionPointer(this, &BleConn::onCharDescriptorDiscTermination));
        discoveredChars.pop_back();
    }
}

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
    auto cUuid = p->characteristic.getUUID().getShortUUID();
    BLE_LOG("[Info] BleConn::onCharDescriptorDisc for (%x)", cUuid);

    if (p->descriptor.getUUID() == BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG)
    {
        BLE_LOG("[Info] CCCD found");
        cccdList[p->characteristic.getUUID().getShortUUID()] = p->descriptor.getAttributeHandle();
        // ble.gattClient().terminateCharacteristicDescriptorDiscovery(p->characteristic);
    }
}

void BleConn::onCharDescriptorDiscTermination(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *p)
{
    auto cUuid = p->characteristic.getUUID().getShortUUID();
    BLE_LOG("[Info] BleConn::onCharDescriptorDiscTermination for (%x)", cUuid);

    auto it = cccdList.find(p->characteristic.getUUID().getShortUUID());
    if(it != cccdList.end()) {
        uint16_t notification_enabled = 1;
        ble_error_t err = ble.gattClient().write(
            GattClient::GATT_OP_WRITE_CMD,
            p->characteristic.getConnectionHandle(),
            it->second,
            sizeof(notification_enabled),
            reinterpret_cast<const uint8_t *>(&notification_enabled));
        if(err == BLE_ERROR_NONE) {
            BLE_LOG("[Info] CCCD notification enabled for (%x)", cUuid);
        }
        else {
            BLE_LOG("[Error] CCCD notification FAILED for (%x), err=(%d)", cUuid, err);
        }
        cccdList.erase(it);
    }
    this->startDescriptorDiscovery();
}

void BleConn::sendDataPack(const DataPack& p) {
    json j;
    j["evt"] = p.eventType;
    j["conn"] = p.connHandle;
    j["valh"] = p.handle;

    auto len = p.len;
    char buff[2*len+1];
    buff[2*len]=0;
    for(int i=0; i<len; i++) {
        sprintf(buff+i*2, "%02x", p.data[i]);
    }
    j["val"]=std::string(buff);

    this->respCb(j.dump().c_str());
}

// https://docs.mbed.com/docs/ble-api/en/master/api/structGattHVXCallbackParams.html
void BleConn::onHVX(const GattHVXCallbackParams *p)
{
    BLE_LOG("[Info] BleConn::onHVX (data:%x)", *(p->data));

    DataPack p1 {p->connHandle, p->handle, p->len, p->data, BLE_EVENT_onHVX};
    this->sendDataPack(p1);
    evq.call(this, &BleConn::scan);
    // json j;
    // j["evt"] = BLE_EVENT_onHVX;
    // j["conn"] = p->connHandle;
    // j["valh"] = p->handle;

    // auto len = p->len;
    // char buff[2*len+1];
    // buff[2*len]=0;
    // for(int i=0; i<len; i++) {
    //     sprintf(buff+i*2, "%02x", p->data[i]);
    // }
    // j["val"]=std::string(buff);

    // this->respCb(j.dump().c_str());
    // evq.call(this, &BleConn::scan);
}

void BleConn::onDataRead(const GattReadCallbackParams *p)
{
    BLE_LOG("[Info] BleConn::onDataRead data:(%02x) len:(%d)", *(p->data), p->len);

    if(p->status == BLE_ERROR_NONE) {
        DataPack p1 {p->connHandle, p->handle, p->len, p->data, BLE_EVENT_onDataRead};
        this->sendDataPack(p1);
        evq.call(this, &BleConn::scan);
    
        // json j;
        // j["evt"] = BLE_EVENT_onDataRead;
        // j["conn"] = p->connHandle;
        // j["valh"] = p->handle;

        // char buff[2*(p->len)+1];
        // buff[2*(p->len)]=0;
        // for(int i=0; i<p->len; i++) {
        //     sprintf(buff+i*2, "%02x", p[i].data);
        // }
        // // if(p->len == 4) {
        // //     uint32_t val = *(reinterpret_cast<const uint32_t*>(p->data));
        // //     j["val"] = val;
        // // } else {
        // //     j["val"] = *(p->data);
        // // }
        // j["val"]=std::string(buff);
        // this->respCb(j.dump().c_str());
    }
    else {
        BLE_LOG("[error] BleConn::onDataRead (err:%d)", p->error_code);
    }
    evq.call(this, &BleConn::scan);
}

void BleConn::onDataWritten(const GattWriteCallbackParams *p)
{
    BLE_LOG("[Info] BleConn::onDataWritten (status:%d)", p->status);
    evq.call(this, &BleConn::scan);
}

#endif