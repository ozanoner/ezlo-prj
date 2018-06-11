#ifndef PeripheralBleConn_h_
#define PeripheralBleConn_h_


#include <mbed.h>
#include <ble/BLE.h>
#include <GattCharacteristic.h>
#include "SEGGER_RTT.h"
#include "HABleServiceDefs.h"


class PeripheralBleConn: private mbed::NonCopyable<PeripheralBleConn> {
protected:
    BLE& ble;
    events::EventQueue& evq;
    void scheduleBleEvents(BLE::OnEventsToProcessCallbackContext *context);

    void onTimeout(const Gap::TimeoutSource_t source);
    void onInitComplete();
    void onBleInitError(BLE& ble, ble_error_t error);

    void onConnected(const Gap::ConnectionCallbackParams_t* event);
    void onDisconnected(const Gap::DisconnectionCallbackParams_t* event);

    // data communication
    // register from derived devices
    virtual void onDataWritten(const GattWriteCallbackParams* params);
    virtual void onDataRead(const GattReadCallbackParams* params);
    virtual void onDataSent(unsigned params);
public:
    PeripheralBleConn(EventQueue& evq) : evq(evq), ble(BLE::Instance()) { }
    virtual void init();

};


void PeripheralBleConn::init() {
    ble.onEventsToProcess(
        makeFunctionPointer(this, &PeripheralBleConn::scheduleBleEvents));
    ble.gap().onTimeout(
        makeFunctionPointer(this, &PeripheralBleConn::onTimeout));
}

void PeripheralBleConn::scheduleBleEvents(BLE::OnEventsToProcessCallbackContext *context)
{
    evq.call(mbed::callback(&context->ble, &BLE::processEvents));
}

/** called if timeout is reached during advertising, scanning
     *  or connection initiation */
void PeripheralBleConn::onTimeout(const Gap::TimeoutSource_t source)
{
    DPRN("[Warning] PeripheralBleConn::onTimeout (%d)\n", source);
}

void PeripheralBleConn::onBleInitError(BLE &ble, ble_error_t error)
{
    DPRN("[Warning] PeripheralBleConn::onBleInitError (%d)\n", error);
}

// device-specific values are in their own implementation
void PeripheralBleConn::onInitComplete()
{
    if(ble.gap().setTxPower(4)!=BLE_ERROR_NONE) {
        DPRN("[error] setTxPower\n");
    }

    ble.gap().onConnection(this, &PeripheralBleConn::onConnected);
    ble.gap().onDisconnection(this, &PeripheralBleConn::onDisconnected);

    /* setup advertising */
    ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED | GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
    ble.gap().setAdvertisingInterval(1000); /* 1000ms. */
}

void PeripheralBleConn::onConnected(const Gap::ConnectionCallbackParams_t *conn)
{
    DPRN("[Info] PeripheralBleConn::onConnected (%x)\n", conn->handle);
    this->ble.gap().stopAdvertising();
}

void PeripheralBleConn::onDisconnected(const Gap::DisconnectionCallbackParams_t *event)
{
    DPRN("[Warning] PeripheralBleConn::onDisconnected\n");
    this->ble.gap().startAdvertising();
}

void PeripheralBleConn::onDataRead(const GattReadCallbackParams* params) {
    DPRN("[info] PeripheralBleConn::onDataRead. h:(%x)-offset(%x)\n", params->handle, params->offset);
}

void PeripheralBleConn::onDataSent(unsigned count) {
    DPRN("[info] PeripheralBleConn::onDataSent. count(%u)\n", count);
}

void PeripheralBleConn::onDataWritten(const GattWriteCallbackParams* params) {
    DPRN("[info] PeripheralBleConn::onDataWritten. h:(%x) status:(%d)\n", params->handle, params->status);
}

#endif