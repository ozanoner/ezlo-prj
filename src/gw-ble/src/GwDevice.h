/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * https://os.mbed.com/teams/mbed-os-examples/code/mbed-os-example-ble-GAP/
 * 
 */


#ifndef GwDevice_h_
#define GwDevice_h_

#include <map>

#include <mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"

#include "ble/DiscoveredCharacteristic.h"
#include "ble/DiscoveredService.h"


static const uint8_t DEVICE_NAME[]        = "GW_device";
static const Gap::Address_t  BLE_gw_addr       = {0xCC, 0x00, 0x00, 0xE1, 0x01, 0x01};



/** Demonstrate advertising, scanning and connecting
 */
class GwDevice : private mbed::NonCopyable<GwDevice>
{
public:
    GwDevice() :
        _ble(BLE::Instance()),
        _led1(LED1, 0),
        _is_connecting(false),
        _scan_count(0) { };

    ~GwDevice()
    {
        if (_ble.hasInitialized()) {
            _ble.shutdown();
        }
    };

    /** Start BLE interface initialisation */
    void run()
    {
        ble_error_t error;

        if (_ble.hasInitialized()) {
            printf("Ble instance already initialised.\r\n");
            return;
        }

        /* this will inform us off all events so we can schedule their handling
         * using our event queue */
        _ble.onEventsToProcess(
            makeFunctionPointer(this, &GwDevice::schedule_ble_events)
        );

        /* handle timeouts, for example when connection attempts fail */
        _ble.gap().onTimeout(
            makeFunctionPointer(this, &GwDevice::on_timeout)
        );

        error = _ble.init(this, &GwDevice::on_init_complete);

        if (error) {
            printf("Error returned by BLE::init.\r\n");
            return;
        }

        /* to show we're running we'll blink every 500ms */
        _event_queue.call_every(500, this, &GwDevice::blink);

        /* this will not return until shutdown */
        _event_queue.dispatch_forever();
    };

private:
    /** This is called when BLE interface is initialised and starts the first mode */
    void on_init_complete(BLE::InitializationCompleteCallbackContext *event)
    {
        if (event->error) {
            printf("Error during the initialisation\r\n");
            return;
        }

        /* print device address */
        Gap::AddressType_t addr_type;
        Gap::Address_t addr;
        _ble.gap().getAddress(&addr_type, addr);
        printf("Device address: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
               addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);

        /* all calls are serialised on the user thread through the event queue */
        _event_queue.call(this, &GwDevice::scan_mode_start);
    };

    void scan_mode_start() {
        _ble.gap().onConnection(this, &GwDevice::on_connect);
        _ble.gap().onDisconnection(this, &GwDevice::on_disconnect);
        _ble.gattClient().onHVX(
            makeFunctionPointer(this, &GwDevice::hvx_handler));


        _ble.gattClient().onDataRead(
            makeFunctionPointer(this, &GwDevice::data_read_handler));

         _ble.gattClient().onDataWritten(
            makeFunctionPointer(this, &GwDevice::data_written_handler));
        _event_queue.call(this, &GwDevice::scan2);
        _is_connecting = false;
    }

    void scan2() {
        // timeout: 0->disabled
        ble_error_t error = _ble.gap().setScanParams(800, 400, 0, true);
        if (error) {
            printf("Error during Gap::setScanParams\r\n");
            return;
        }

        /* start scanning and attach a callback that will handle advertisements
         * and scan requests responses */
        error = _ble.gap().startScan(this, &GwDevice::on_scan2);

        if (error) {
            printf("Error during Gap::startScan\r\n");
            return;
        }

        printf("Scanning started (interval: %dms, window: %dms, timeout: %ds).\r\n",
               400, 400, 0);           
    }

    void on_scan2(const Gap::AdvertisementCallbackParams_t *params)
    {
        /* keep track of scan events for performance reporting */
        _scan_count++;

        printf(".");
        if(_scan_count%50 == 0)
            printf("\n");
        if(params->peerAddr[0]!=0xcc)
            return;
        print_address(params->peerAddr);

        /* don't bother with analysing scan result if we're already connecting */
        if (_is_connecting) {
            printf("_is_connecting..\n");
            return;
        }
        ble_error_t error = _ble.gap().connect( params->peerAddr, BLEProtocol::AddressType_t::PUBLIC, nullptr, nullptr);
        if (error) {
            printf("Error during Gap::connect\r\n");
            return;
                    
        }
        _is_connecting = true;
    }

    
    void print_address(const BLEProtocol::AddressBytes_t peerAddr) {
        printf("peer_addr:");
        for(int i=BLEProtocol::ADDR_LEN-1; i>=0; --i)
            printf("%02x:",peerAddr[i]);
        printf("\n");
    }

/*
peer_addr:a1:80:e1:00:00:cc:
...Connected to:peer_addr:a1:80:e1:00:00:cc:
Service Short UUID-1800 attrs[1 7]
  Char UUID-2a00 valueAttr[3] props[0]
  Char UUID-2a01 valueAttr[5] props[0]
  Char UUID-2a04 valueAttr[7] props[0]
Service Short UUID-1801 attrs[8 11]
Scanning started (interval: 400ms, window: 400ms, timeout: 0s).
  Char UUID-2a05 valueAttr[10] props[0]
Service Short UUID-a000 attrs[12 65535]
...........................................
.............................  Char UUID-a001 valueAttr[14] props[0]
terminated SD for handle 0
*/
    
    /** This is called by Gap to notify the application we connected,
     *  in our case it immediately disconnects */
    void on_connect(const Gap::ConnectionCallbackParams_t *connection_event)
    {
        // print_performance();

        // printf("Connected in %dms\r\n", _demo_duration.read_ms());
        printf("connection handle %x, Connected to:", connection_event->handle);
        print_address(connection_event->peerAddr);

        _is_connecting = false;
        _event_queue.call_in(1000, this, &GwDevice::scan2);
        _event_queue.call(makeFunctionPointer(this, &GwDevice::start_service_disc), 
            connection_event->handle);

    }

    void start_service_disc(GattAttribute::Handle_t h) {
        BLE &ble = BLE::Instance();
        ble.gattClient().onServiceDiscoveryTermination(
            makeFunctionPointer(this,  &GwDevice::discoveryTerminationCallback));
        ble.gattClient().launchServiceDiscovery(h, 
            makeFunctionPointer(this, &GwDevice::serviceDiscoveryCallback), 
            makeFunctionPointer(this, &GwDevice::characteristicDiscoveryCallback));

    }

    void serviceDiscoveryCallback(const DiscoveredService *service) {
        if (service->getUUID().shortOrLong() == UUID::UUID_TYPE_SHORT) {
            printf("Service Short UUID-%x attrs[%u %u]\r\n", service->getUUID().getShortUUID(), service->getStartHandle(), service->getEndHandle());
        } 
        else {
            printf("Service Long UUID-");
            const uint8_t *longUUIDBytes = service->getUUID().getBaseUUID();
            for (unsigned i = 0; i < UUID::LENGTH_OF_LONG_UUID; i++) {
                printf("%02x", longUUIDBytes[i]);
            }
            printf(" attrs[%u %u]\r\n", service->getStartHandle(), service->getEndHandle());
        }
    }


    void characteristicDiscoveryCallback(const DiscoveredCharacteristic *characteristicP) {
        
        auto charid = characteristicP->getUUID().getShortUUID();
        printf("  Char UUID-%x valueAttr[%u] props[%x]\r\n", 
            charid, 
            characteristicP->getValueHandle(), 
            (uint8_t)characteristicP->getProperties().broadcast());
        
        if(charid == 0xa001) {
            this->notifyList[charid] = characteristicP;

            // printf ("calling discoverDescriptors..\n");
            // characteristicP->discoverDescriptors(
            //     makeFunctionPointer(this, &GwDevice::charDescDiscCb), 
            //     makeFunctionPointer(this, &GwDevice::charDescDiscTermCb));         
        }

        // if (characteristicP->getUUID().getShortUUID() == 0xa001) { 
        //     ledCharacteristic        = *characteristicP;
        //     triggerLedCharacteristic = true;
        // }
    }

    void charDescDiscCb(const CharacteristicDescriptorDiscovery::DiscoveryCallbackParams_t* p) {
        printf("checking _CCCD\n");
        
        if (p->descriptor.getUUID() == BLE_UUID_DESCRIPTOR_CLIENT_CHAR_CONFIG) { 
            _CCCD = p->descriptor.getAttributeHandle();
            printf("_CCCD found: %02x\n", _CCCD);
            // p->characteristic.getGattClient()->terminateCharacteristicDescriptorDiscovery(p->characteristic);
            // auto client = p->descriptor.getGattClient();
            // client->terminateCharacteristicDescriptorDiscovery(p->characteristic);
        }
    }

    void charDescDiscTermCb(const CharacteristicDescriptorDiscovery::TerminationCallbackParams_t* p) {
        printf("in charDescDiscTermCb\n");
        
        uint16_t notification_enabled = 1; 
        _ble.gattClient().write(
             GattClient::GATT_OP_WRITE_CMD,
            p->characteristic.getConnectionHandle(),
            _CCCD,
            sizeof(notification_enabled),
            reinterpret_cast<const uint8_t*>(&notification_enabled)
        );
        // ble_error_t err = p->characteristic.getGattClient()->write( 
        //     GattClient::GATT_OP_WRITE_REQ,
        //     p->characteristic.getConnectionHandle(),
        //     _CCCD,
        //     sizeof(notification_enabled),
        //     reinterpret_cast<const uint8_t*>(&notification_enabled)
        // );
        // if(err) {
        //     printf("charDescDiscTermCb err: %d\n", err);
        // }
    }



    void hvx_handler(const GattHVXCallbackParams* p) {
        printf("hvx_handler: conn:%x attr:%x data:%x\n", p->connHandle, p->handle, p->data[0]);
        
    }

    void data_read_handler(const GattReadCallbackParams* p) {
        printf("data_read_handler: conn:%x attr:%x\n", p->connHandle, p->handle);
    }

    void data_written_handler(const GattWriteCallbackParams* p) {
        printf("data_written_handler: conn:%x attr:%x, status:%d\n", p->connHandle, p->handle, p->status);
    }

// connection handle passed?
    void discoveryTerminationCallback(Gap::Handle_t connectionHandle) {
        printf("terminated SD for handle %u\r\n", connectionHandle);
        // if (triggerLedCharacteristic) {
        //     triggerLedCharacteristic = false;
        //     eventQueue.call(updateLedCharacteristic);
        // }

        printf ("calling discoverDescriptors..\n");
        this->notifyList[0xa001]->discoverDescriptors(
                makeFunctionPointer(this, &GwDevice::charDescDiscCb), 
                makeFunctionPointer(this, &GwDevice::charDescDiscTermCb));         

        
    }



    /** This is called by Gap to notify the application we disconnected,
     *  in our case it calls demo_mode_end() to progress the demo */
    void on_disconnect(const Gap::DisconnectionCallbackParams_t *event)
    {
        printf("Disconnected\r\n");

        /* we have successfully disconnected ending the demo, move to next mode */
        // _event_queue.call(this, &GwDevice::demo_mode_end);
    };

    /** called if timeout is reached during advertising, scanning
     *  or connection initiation */
    void on_timeout(const Gap::TimeoutSource_t source)
    {
        // _demo_duration.stop();

        switch (source) {
            case Gap::TIMEOUT_SRC_ADVERTISING:
                printf("Stopped advertising early due to timeout parameter\r\n");
                break;
            case Gap::TIMEOUT_SRC_SCAN:
                printf("Stopped scanning early due to timeout parameter\r\n");
                break;
            case Gap::TIMEOUT_SRC_CONN:
                printf("Failed to connect after scanning %d advertisements\r\n", _scan_count);
                // _event_queue.call(this, &GwDevice::print_performance);
                // _event_queue.call(this, &GwDevice::demo_mode_end);
                break;
            default:
                printf("Unexpected timeout\r\n");
                break;
        }
    };


    /** Schedule processing of events from the BLE middleware in the event queue. */
    void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context)
    {
        _event_queue.call(mbed::callback(&context->ble, &BLE::processEvents));
    };

    /** Blink LED to show we're running */
    void blink(void)
    {
        _led1 = !_led1;
    };

private:
    BLE                &_ble;
    events::EventQueue  _event_queue;
    DigitalOut          _led1;
    std::map<UUID::ShortUUIDBytes_t, const DiscoveredCharacteristic*> notifyList;
    GattAttribute::Handle_t _CCCD;

    /* Keep track of our progress through demo modes */
    bool                _is_connecting;

    /* Measure performance of our advertising/scanning */
    size_t              _scan_count;
};

#endif