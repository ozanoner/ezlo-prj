
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


#define DEV_PROVISION_ID LED_ID4

static const uint8_t DEVICE_NAME[] = "LED_ID4";
static const Gap::Address_t BLE_NW_ADDR = {HOME_ID, 0x00, 0x00, 0xE1, 0x01, DEV_PROVISION_ID};
static const uint16_t SERVICE_UUID_LIST[] = {LED_SERVICE_UUID};

using namespace std;

class BleConn : public PeripheralBleConn {
private:
    // characteristics
    ReadWriteGattCharacteristic<uint8_t>  ledDuty;

    // ble init
    void onInitComplete(BLE::InitializationCompleteCallbackContext* event);

    // data communication
    void onDataWritten(const GattWriteCallbackParams* params) override;
    void onDataRead(const GattReadCallbackParams* params) override;
    void onDataSent(unsigned params) override;

    Callback<void(uint8_t)> dataWrittenCb;

    
public:
    BleConn(EventQueue& evq, uint8_t initialDuty) : PeripheralBleConn(evq), 
        dataWrittenCb(nullptr),
        ledDuty(LED_STATE_CHARACTERISTIC_UUID, &initialDuty)
             {    }

    void init() override;
    void setDataWrittenCb(Callback<void(uint8_t)> cb) {
        this->dataWrittenCb = cb;
    }
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


    GattCharacteristic* charTable[] = {&this->ledDuty};
    GattService service(LED_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
    ble.gattServer().addService(service);

    // ble communication callbacks
    ble.gattServer().onDataRead(this, &BleConn::onDataRead);
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
    if ((params->handle == this->ledDuty.getValueHandle()) && (params->len == 1)) {
        DPRN("[info] new dim state (%x)\n", *(params->data));
        if(this->dataWrittenCb!=nullptr) 
            this->dataWrittenCb(*(params->data));
    }
}


#endif