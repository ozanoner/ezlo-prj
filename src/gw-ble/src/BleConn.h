
#ifndef BleConn_h_
#define BleConn_h_

#include <mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "ble/DiscoveredCharacteristic.h"
#include "ble/DiscoveredService.h"
#include <map>
#include "BLEProtocol.h"
#include <cstdarg>
#include "HADevShadow.h"
#include <utility>
#include <memory>

#include "json/src/json.hpp"
using json = nlohmann::json;



#define DEBUG_PRINT_ON

#ifdef DEBUG_PRINT_ON
#define DPRN_BLE(format, ...) dbg (format, ##__VA_ARGS__)
#else
#define DPRN_BLE(format, ...)
#endif

#include "HABleServiceDefs.h"
#include "HAProvision.h"
#define DEV_PROVISION_ID GW_ID1

static const uint8_t DEVICE_NAME[] = "ComodoHA_GW";
static const Gap::Address_t BLE_NW_ADDR = {HOME_ID, 0x00, 0x00, 0xE1, 0x01, DEV_PROVISION_ID};

using namespace std;

using DebugPrintFuncT = void (*)(const char*, va_list);


class BleConn : private mbed::NonCopyable<BleConn> {
private:
    BLE& ble;
    events::EventQueue& evq;
    bool isProcessing;
    int scanCnt;


    // map devices over connection handles
    map<Gap::Handle_t, std::shared_ptr<HADevShadow>> devices;
    map<uint8_t, std::shared_ptr<HADevShadow>> devicesById;

    std::shared_ptr<HADevShadow> actDevice;

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

    bool deviceExists(Gap::Handle_t h) const { 
        return this->devices.find(h) != this->devices.end(); 
    }
    bool deviceExists(uint8_t id) const { 
        return this->devicesById.find(id) != this->devicesById.end(); 
    }

    std::shared_ptr<HADevShadow> getDevice(Gap::Handle_t h) {
        if(deviceExists(h))
            return this->devices[h];
        return nullptr;
    }

    std::shared_ptr<HADevShadow> getDevice(uint8_t id) {
        if(deviceExists(id))
            return this->devicesById[id];
        return nullptr;
    }

    HADevShadow::ResponseCallbackT respCb;

public:
    BleConn(EventQueue& evq) : evq(evq), ble(BLE::Instance()),
                               isProcessing(false), scanCnt(0),
                               debugPrint(nullptr) 
    {    }

    void init(HADevShadow::ResponseCallbackT, DebugPrintFuncT);

    void setDebugPrintCallback(DebugPrintFuncT d) {
        this->debugPrint = d;
    }

    void userCommand(const char*);
};


void BleConn::userCommand(const char* data) {
    auto root = json::parse(data);

    uint8_t devId = root["dev"]; // last byte of mac addres
    UUID::ShortUUIDBytes_t cUuid = root["state"]; // characteristic handle
    uint8_t cmd = root["cmd"]; // get=0 | set=1
    int setValue = root["val"]; // state specific value if cmd=1

    auto dev = this->getDevice(devId);
    if(dev == nullptr || !dev->isConnected()) {
        return;
    }
    if(cmd) {
        dev->write(cUuid, setValue);
    }
    else {
        dev->read(cUuid);
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


void BleConn::init(HADevShadow::ResponseCallbackT respCb, DebugPrintFuncT debugpf=nullptr) {

    if (ble.hasInitialized()) {
        // printf("Ble instance already initialised.");
        return;
    }

    this->debugPrint = debugpf;
    this->respCb = respCb;

    ble.onEventsToProcess(
        makeFunctionPointer(this, &BleConn::scheduleBleEvents));
    ble.gap().onTimeout(
        makeFunctionPointer(this, &BleConn::onTimeout));

    ble_error_t error = ble.init(this, &BleConn::onInitComplete);
    if (error) {
        DPRN_BLE("Error returned by BLE::init.");
        return;
    }

    evq.dispatch_forever();
}

void BleConn::scheduleBleEvents(BLE::OnEventsToProcessCallbackContext *context)
{
    evq.call(mbed::callback(&context->ble, &BLE::processEvents));
}

/** called if timeout is reached during advertising, scanning
     *  or connection initiation */
void BleConn::onTimeout(const Gap::TimeoutSource_t source)
{
    switch (source)
    {
    case Gap::TIMEOUT_SRC_ADVERTISING:
        DPRN_BLE("Stopped advertising early due to timeout parameter");
        break;
    case Gap::TIMEOUT_SRC_SCAN:
        DPRN_BLE("Stopped scanning early due to timeout parameter");
        break;
    case Gap::TIMEOUT_SRC_CONN:
        DPRN_BLE("Failed to connect after scanning %d advertisements");
        // evq.call(this, &BleConn::print_performance);
        // evq.call(this, &BleConn::demo_mode_end);
        break;
    default:
        DPRN_BLE("Unexpected timeout");
        break;
    }
}

void BleConn::onInitComplete(BLE::InitializationCompleteCallbackContext *event)
{
    if (event->error)
    {
        DPRN_BLE("Error during the initialisation");
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

    evq.call_every(1000, this, &BleConn::scan);
    // evq.call(this, &BleConn::scan);
    isProcessing = false;
}

void BleConn::scan()
{
    // timeout: 0->disabled
    ble_error_t error = ble.gap().setScanParams(800, 400, 0, true);
    if (error)
    {
        DPRN_BLE("Error during Gap::setScanParams");
        return;
    }

    /* start scanning and attach a callback that will handle advertisements
         * and scan requests responses */
    error = ble.gap().startScan(this, &BleConn::onAdDetected);
    if (error) {
        DPRN_BLE("Error during Gap::startScan");
        return;
    }

    DPRN_BLE("Scanning started (interval: %dms, window: %dms, timeout: %ds).",
         400, 400, 0);
}

void BleConn::onAdDetected(const Gap::AdvertisementCallbackParams_t *params)
{
    if (isProcessing)
        return;

    // home address 0xCC
    if (params->peerAddr[HOME_ID_IDX] != HOME_ID)
        return;
    isProcessing = true;

    ble_error_t error = ble.gap().connect(params->peerAddr, BLEProtocol::AddressType_t::PUBLIC, nullptr, nullptr);
    if (error)
    {
        isProcessing = false;
        DPRN_BLE("Error during Gap::connect");
        return;
    }

}

void BleConn::onConnected(const Gap::ConnectionCallbackParams_t *conn)
{
    DPRN_BLE("connection handle %x, Connected to:", conn->handle);

    this->actDevice = nullptr;
    auto devId = conn->peerAddr[DEV_ID_IDX];
    if(this->deviceExists(devId)) {
        this->actDevice = this->devicesById[devId];
    }
    else {
        this->actDevice = std::make_shared<HADevShadow>();
        std::memcpy(this->actDevice->address, conn->peerAddr, BLEProtocol::ADDR_LEN);
        this->actDevice->setResponseCallback(this->respCb);
        this->devicesById.emplace(devId, this->actDevice);
        this->devices.emplace(conn->handle, this->actDevice);
    }

    this->actDevice->onConnected(conn->handle);
    ble.gattClient().launchServiceDiscovery(conn->handle,
            makeFunctionPointer(this, &BleConn::onServiceDiscovery),
            makeFunctionPointer(this, &BleConn::onCharacteristicDiscovery));
}

void BleConn::onDisconnected(const Gap::DisconnectionCallbackParams_t *event)
{
    DPRN_BLE("Disconnected");
    if(this->deviceExists(event->handle)) {
        this->devices[event->handle]->onDisconnected();
    }
}


void BleConn::onServiceDiscovery(const DiscoveredService *service)
{
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
            this->actDevice->serviceId = service->getUUID().getShortUUID();
        }
        else {
            // TODO: disconnect & disable to reconnect
        }
    }
}

void BleConn::onServiceDiscoveryTermination(Gap::Handle_t connectionHandle)
{
    DPRN_BLE("terminated SD for handle %u", connectionHandle);

    // Right place to stop. characteristic discovery has info about connection
    // DONT USE actDevice from that point on
    isProcessing = false;
}

// https://os.mbed.com/docs/v5.7/mbed-os-api-doxy/class_discovered_characteristic.html
void BleConn::onCharacteristicDiscovery(const DiscoveredCharacteristic *discChar)
{
    auto device = this->getDevice(discChar->getConnectionHandle());
    if(device == nullptr) {
        // not in the devices map
        return;
    }

    auto shortUUID = discChar->getUUID().getShortUUID();
    bool continueWithDesc=false;
    switch(device->deviceType()) {
        case BUTTON1_SERVICE_UUID:
            if(shortUUID == BUTTON_STATE_CHARACTERISTIC_UUID
                || shortUUID == BATTERY_STATE_CHARACTERISTIC_UUID) 
            {
                device->connInfo->characteristics.emplace(shortUUID, 
                    std::make_shared<const DiscoveredCharacteristic>(std::move(*discChar)));
                continueWithDesc = true;
            }
        break;
        default: // unknown device type??
        break;
    }
    if(continueWithDesc) {
        discChar->discoverDescriptors(
            makeFunctionPointer(this, &BleConn::onCharDescriptorDisc),
            makeFunctionPointer(this, &BleConn::onCharDescriptorDiscTermination));
    }
}

void BleConn::onCharDescriptorDisc(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t* p)
{
    DPRN_BLE("checking _CCCD\n");

    if (p->descriptor.getUUID() == BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG)
    {
        auto device = this->getDevice(p->characteristic.getConnectionHandle());
        if(device == nullptr)
            return;
        device->connInfo->lastCccdHandle = p->descriptor.getAttributeHandle();
        DPRN_BLE("_CCCD found: %02x\n", device->connInfo->lastCccdHandle);
        ble.gattClient().terminateCharacteristicDescriptorDiscovery(p->characteristic);
    }
}

void BleConn::onCharDescriptorDiscTermination(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t *p)
{
    DPRN_BLE("in charDescDiscTermCb\n");

    auto device = this->getDevice(p->characteristic.getConnectionHandle());
    if(device == nullptr)
        return;

    uint16_t notification_enabled = 1;
    ble.gattClient().write(
        GattClient::GATT_OP_WRITE_CMD,
        p->characteristic.getConnectionHandle(),
        device->connInfo->lastCccdHandle,
        sizeof(notification_enabled),
        reinterpret_cast<const uint8_t *>(&notification_enabled));
}

// https://docs.mbed.com/docs/ble-api/en/master/api/structGattHVXCallbackParams.html
void BleConn::onHVX(const GattHVXCallbackParams *p)
{
    // _event_queue.call(this, &GwDevice::hvx_send_data, p->connHandle, p->handle, *p->data);
    // this->hvx_send_data(p->connHandle, p->handle, *p->data);
    DPRN_BLE("hvx_handler: conn:%x attr:%x data:%x\n", p->connHandle, p->handle, *p->data);

    auto device = this->getDevice(p->connHandle);
    if(device == nullptr)
        return;
    device->onHVX(p);
}

void BleConn::onDataRead(const GattReadCallbackParams *p)
{
    DPRN_BLE("onDataRead: conn:%x attr:%x\n", p->connHandle, p->handle);
    // esp32_comm.printf("onDataRead: conn:%x attr:%x\n", p->connHandle, p->handle);

    auto device = this->getDevice(p->connHandle);
    if(device == nullptr)
        return;
    device->onDataRead(p);
}

void BleConn::onDataWritten(const GattWriteCallbackParams *p)
{
    DPRN_BLE("onDataWritten: conn:%x attr:%x, status:%d\n", p->connHandle, p->handle, p->status);
    // esp32_comm.printf("onDataWritten: conn:%x attr:%x, status:%d\n", p->connHandle, p->handle, p->status);
    auto device = this->getDevice(p->connHandle);
    if(device == nullptr)
        return;
    device->onDataWritten(p);
}

#endif