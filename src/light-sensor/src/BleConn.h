
#ifndef BleConn_h_
#define BleConn_h_

#include <mbed.h>
#include "ble/BLE.h"
#include <GattCharacteristic.h>
#include "SEGGER_RTT.h"

#include "HABleServiceDefs.h"
#include "HAProvision.h"

// comes from common
#include "PeripheralBleConn.h"


#define DEV_PROVISION_ID LS_ID8

static const uint8_t DEVICE_NAME[] = "LS_ID8";
static const Gap::Address_t BLE_NW_ADDR = {HOME_ID, 0x00, 0x00, 0xE1, 0x01, DEV_PROVISION_ID};
static const uint16_t SERVICE_UUID_LIST[] = {LIGHT_SERVICE_UUID};


using namespace std;

class BleConn : public PeripheralBleConn {
private:
    // in PeripheralBleConn
    // BLE& ble;
    // events::EventQueue& evq;

    // characteristics
    ReadOnlyGattCharacteristic<uint16_t>  lightState;

    // ble init
    void onInitComplete(BLE::InitializationCompleteCallbackContext* event);

    // data communication
    void onDataWritten(const GattWriteCallbackParams* params) override;
    void onDataRead(const GattReadCallbackParams* params) override;
    void onDataSent(unsigned params) override;
    
public:
    // BleConn(EventQueue& evq) : evq(evq), ble(BLE::Instance()), 
    BleConn(EventQueue& evq) : PeripheralBleConn(evq), 
        lightState(LIGHT_STATE_CHARACTERISTIC_UUID, nullptr, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY)
             {    }

    void init() override;
    void updateSensorState(const uint16_t data);
};


void BleConn::init() {
    if (ble.hasInitialized())
        return;

    PeripheralBleConn::init();

    ble_error_t error = ble.init(this, &BleConn::onInitComplete);
    if (error) {
        DPRN("[Error] BLE::init.(%d)\n", error);
        return;
    }
    DPRN("[Info] BleConn::init completed\n");
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

    PeripheralBleConn::onInitComplete();

    
    ble.gap().setAddress(Gap::AddressType_t::PUBLIC, BLE_NW_ADDR);
    /* setup advertising */
    // ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LIST_16BIT_SERVICE_IDS, 
        (uint8_t *)SERVICE_UUID_LIST, sizeof(SERVICE_UUID_LIST));
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::COMPLETE_LOCAL_NAME, (uint8_t *)DEVICE_NAME, sizeof(DEVICE_NAME));


    GattCharacteristic* charTable[] = {&this->lightState};
    GattService service(LIGHT_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
    ble.gattServer().addService(service);

    // ble communication callbacks
    ble.gattServer().onDataRead(this, &BleConn::onDataRead); // doesn't run??
    ble.gattServer().onDataSent(this, &BleConn::onDataSent); // for notification
    ble.gattServer().onDataWritten(this, &BleConn::onDataWritten);


    ble.gap().startAdvertising();
    DPRN("[Info] BleConn::onInitComplete completed. starting ad..\n");
}


void BleConn::onDataRead(const GattReadCallbackParams* params) {
    PeripheralBleConn::onDataRead(params);
}

void BleConn::onDataSent(unsigned count) {
    PeripheralBleConn::onDataSent(count);
}

void BleConn::onDataWritten(const GattWriteCallbackParams* params) {
    PeripheralBleConn::onDataWritten(params);
}


void BleConn::updateSensorState(const uint16_t data) {
    this->ble.gattServer().write(this->lightState.getValueHandle(), (uint8_t *)&data, sizeof(uint16_t));
}


#endif