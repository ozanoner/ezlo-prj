
#ifndef BleConn_h_
#define BleConn_h_

#include <mbed.h>
#include "ble/BLE.h"
#include <GattCharacteristic.h>
#include "SEGGER_RTT.h"

#include "HABleServiceDefs.h"
#include "HAProvision.h"

#define DEV_PROVISION_ID LS_ID8

static const uint8_t DEVICE_NAME[] = "LS_ID8";
static const Gap::Address_t BLE_NW_ADDR = {HOME_ID, 0x00, 0x00, 0xE1, 0x01, DEV_PROVISION_ID};
static const uint16_t SERVICE_UUID_LIST[] = {LIGHT_SERVICE_UUID};


using namespace std;

class BleConn : private mbed::NonCopyable<BleConn> {
private:
    BLE& ble;
    events::EventQueue& evq;
    void scheduleBleEvents(BLE::OnEventsToProcessCallbackContext *context);

    // characteristics
    ReadOnlyGattCharacteristic<uint16_t>  lightState;

    // connection & initialization
    void onTimeout(const Gap::TimeoutSource_t source);
    void onInitComplete(BLE::InitializationCompleteCallbackContext* event);
    void onBleInitError(BLE& ble, ble_error_t error);

    void onConnected(const Gap::ConnectionCallbackParams_t* event);
    void onDisconnected(const Gap::DisconnectionCallbackParams_t* event);

    // data communication
    // void onDataWritten(const GattWriteCallbackParams* params);
    void onDataRead(const GattReadCallbackParams* params);
    void onDataSent(unsigned params);
    
public:
    BleConn(EventQueue& evq) : evq(evq), ble(BLE::Instance()), 
        lightState(LIGHT_STATE_CHARACTERISTIC_UUID, nullptr, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY)
             {    }

    void init();
    void updateSensorState(const uint16_t data);
};


void BleConn::init() {
    if (ble.hasInitialized())
        return;

    ble.onEventsToProcess(
        makeFunctionPointer(this, &BleConn::scheduleBleEvents));
    ble.gap().onTimeout(
        makeFunctionPointer(this, &BleConn::onTimeout));

    ble_error_t error = ble.init(this, &BleConn::onInitComplete);
    if (error) {
        DPRN("[Error] BLE::init.\n");
        return;
    }
    DPRN("[Info] BleConn::init completed\n");
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
        DPRN("[Warning] Stopped advertising early due to timeout parameter\n");
        break;
    case Gap::TIMEOUT_SRC_SCAN:
        DPRN("[Warning] Stopped scanning early due to timeout parameter\n");
        break;
    case Gap::TIMEOUT_SRC_CONN:
        DPRN("[Warning] Failed to connect after scanning %d advertisements\n");
        // evq.call(this, &BleConn::print_performance);
        // evq.call(this, &BleConn::demo_mode_end);
        break;
    default:
        DPRN("[Error] Unexpected timeout\n");
        break;
    }
}

void BleConn::onBleInitError(BLE &ble, ble_error_t error)
{
    /* Initialization error handling should go here */
}

void BleConn::onDataRead(const GattReadCallbackParams* params) {
    DPRN("[info] BleConn::onDataRead. (%x)-(%x)\n", params->handle, params->offset);
}
void BleConn::onDataSent(unsigned count) {
    DPRN("[info] BleConn::onDataSent. (%u)\n", count);
}


void BleConn::onInitComplete(BLE::InitializationCompleteCallbackContext *event)
{
    if (event->error)
    {
        DPRN("[Error] BleConn::onInitComplete input event\n");
        this->onBleInitError(this->ble, event->error);
        return;
    }

    if(ble.getInstanceID() != BLE::DEFAULT_INSTANCE) {
        DPRN("[Error] Not default BLE instance\n");
        return;
    }

    if(ble.gap().setTxPower(4)!=BLE_ERROR_NONE) {
        DPRN("[error] setTxPower\n");
    }
    ble.gap().setAddress(Gap::AddressType_t::PUBLIC, BLE_NW_ADDR);
    
    /* all calls are serialised on the user thread through the event queue */
    ble.gap().onConnection(this, &BleConn::onConnected);
    ble.gap().onDisconnection(this, &BleConn::onDisconnected);
    // ble.gattServer().onDataWritten(this, &BleConn::onDataWritten);


    GattCharacteristic* charTable[] = {&this->lightState};
    GattService service(LIGHT_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
    ble.gattServer().addService(service);
    ble.gattServer().onDataRead(this, &BleConn::onDataRead);
    ble.gattServer().onDataSent(this, &BleConn::onDataSent);

    /* setup advertising */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, 
        (uint8_t *)SERVICE_UUID_LIST, sizeof(SERVICE_UUID_LIST));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(1000); /* 1000ms. */
    ble.gap().startAdvertising();

    DPRN("[Info] BleConn::onInitComplete completed. starting ad..\n");
}



void BleConn::onConnected(const Gap::ConnectionCallbackParams_t *conn)
{
    DPRN("[Info] BleConn::onConnected (%x)\n", conn->handle);
    this->ble.gap().stopAdvertising();
}

void BleConn::onDisconnected(const Gap::DisconnectionCallbackParams_t *event)
{
    DPRN("[Warning] BleConn::onDisconnected\n");
    this->ble.gap().startAdvertising();
}


void BleConn::updateSensorState(const uint16_t data) {
    this->ble.gattServer().write(this->lightState.getValueHandle(), (uint8_t *)&data, sizeof(uint16_t));
}


#endif