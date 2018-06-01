
#ifndef BleConn_h_
#define BleConn_h_

#include <mbed.h>
#include "ble/BLE.h"
#include <GattCharacteristic.h>
#include "SEGGER_RTT.h"

#include "HABleServiceDefs.h"
#include "HAProvision.h"
#define DEV_PROVISION_ID DIMMER_ID7

static const uint8_t DEVICE_NAME[] = "Comodo_Dimmer";
static const Gap::Address_t BLE_NW_ADDR = {HOME_ID, 0x00, 0x00, 0xE1, 0x01, DEV_PROVISION_ID};
static const uint16_t SERVICE_UUID_LIST[] = {DIMMER_SERVICE_UUID};

using namespace std;

class BleConn : private mbed::NonCopyable<BleConn> {
private:
    BLE& ble;
    events::EventQueue& evq;
    void scheduleBleEvents(BLE::OnEventsToProcessCallbackContext *context);

    // characteristics
    ReadWriteGattCharacteristic<uint8_t>  dimmerState;
    ReadOnlyArrayGattCharacteristic<uint8_t, 16>  energyData;

    // connection & initialization
    void onTimeout(const Gap::TimeoutSource_t source);
    void onInitComplete(BLE::InitializationCompleteCallbackContext* event);
    void onBleInitError(BLE& ble, ble_error_t error);

    void onConnected(const Gap::ConnectionCallbackParams_t* event);
    void onDisconnected(const Gap::DisconnectionCallbackParams_t* event);

    // data communication
    void onDataWritten(const GattWriteCallbackParams* params);
    uint8_t initialEnergyData[16] {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
public:
    BleConn(EventQueue& evq) : evq(evq), ble(BLE::Instance()), 
        dimmerState(DIMMER_STATE_CHARACTERISTIC_UUID, 0),
        energyData(DIMMER_ENERGY_CHARACTERISTIC_UUID, 
            initialEnergyData, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY)
             {    }

    void init();
    void sendEnergyData(const uint32_t* data);
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
    
    ble.gap().setAddress(Gap::AddressType_t::PUBLIC, BLE_NW_ADDR);
    
    /* all calls are serialised on the user thread through the event queue */
    ble.gap().onConnection(this, &BleConn::onConnected);
    ble.gap().onDisconnection(this, &BleConn::onDisconnected);
    ble.gattServer().onDataWritten(this, &BleConn::onDataWritten);

   // triac
	// actuatedLED.period(0.02); // 50hz
	// actuatedLED = 1; // off=0% duty

    // dimmerService = new DimmerService(ble);

    GattCharacteristic* charTable[] = {&this->dimmerState, &this->energyData};
    GattService service(DIMMER_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
    ble.gattServer().addService(service);

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
    DPRN("[Info] BleConn::onConnected (%x)", conn->handle);
    this->ble.gap().stopAdvertising();
}

void BleConn::onDisconnected(const Gap::DisconnectionCallbackParams_t *event)
{
    DPRN("[Warning] BleConn::onDisconnected\n");
    this->ble.gap().startAdvertising();
}

void BleConn::onDataWritten(const GattWriteCallbackParams *params) {
    if ((params->handle == this->dimmerState.getValueHandle()) && (params->len == 1)) {
		// pwmout, smooth transition can be added
        // actuatedLED = 1 - ((float)(*(params->data)))/255;
        // triac

        DPRN("[info] new dimmer state (%x)\n", *(params->data));
    }
}


void BleConn::sendEnergyData(const uint32_t* data) {
    // ble.gattServer().write(buttonState.getValueHandle(), (uint8_t *)&newState, sizeof(bool));

    this->ble.gattServer().write(this->energyData.getValueHandle(), (uint8_t *)data, 16);
}


#endif