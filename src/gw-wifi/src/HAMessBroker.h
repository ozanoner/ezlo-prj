#ifndef HAMessBroker_h_
#define HAMessBroker_h_

#include <map>
#include <string>
#include <inttypes.h>
#include "HADev.h"
#include "HABleEvents.h"
#include "HABleServiceDefs.h"
#include <Print.h>
#include <Arduino.h>
#include <ArduinoJson.h>

// https://arduinojson.org/example/

using namespace std;

class HAMessBroker {
private:
    std::map<uint8_t, shared_ptr<HADev_t>> byId;
    std::map<uint16_t, shared_ptr<HADev_t>> byConn;
    static bool isTrackedChar(uint16_t);
    // device, char-uuid, val-handle
    void addAttrib(std::shared_ptr<HADev_t>, uint16_t, uint16_t);
    // device, val-handle, returns charUuid
    uint16_t getCharU(std::shared_ptr<HADev_t>, uint16_t valH) const;

public:
    const char* fromNrf(String&);
    const char* fromMqtt(String&);
};

const char* HAMessBroker::fromNrf(String& buff) {
    StaticJsonBuffer<256> inputBuff;
    JsonObject& inputJ = inputBuff.parseObject(buff);
    if (!inputJ.success()) {
        Serial.println("[error] HAMessBroker::fromNrf: parseObject() failed");
        // buff="{\"err_code\":1}";
        // return buff.c_str();
        return "";
    }

    if(inputJ.containsKey("ble_debug")) {
        // Serial.printf("[ble_debug] %s\n", inputJ["ble_debug"]);
        return "";
    }

    if(!inputJ.containsKey("evt")) {
        Serial.println("[error] HAMessBroker::fromNrf: evt key expected ");
        // buff="{\"err_code\":1}";
        // return buff.c_str();
        return "";
    }
    int event = inputJ["evt"];

    StaticJsonBuffer<256> outputBuff;
    JsonObject& outputJ = outputBuff.createObject();

    bool skip=false;
    
    switch (event)
    {
        case BLE_EVENT_onConnected:
        {
            uint8_t devId = inputJ["peer"];
            uint16_t connH = inputJ["conn"];
            if(this->byId.find(devId)==this->byId.end()) {
                this->byId[devId] = std::make_shared<HADev_t>();
                this->byId[devId]->devId = devId;
                this->byId[devId]->connHandle = connH;
            }
            else {
                uint16_t oldConnH = this->byId[devId]->connHandle;
                if(oldConnH != connH && this->byConn.find(oldConnH)!=this->byConn.end()) {
                    this->byConn.erase(oldConnH);
                    if(this->byConn.find(oldConnH)==this->byConn.end())
                        Serial.printf("[info] oldConnH deleted (%d)\n", oldConnH);
                    else
                        Serial.printf("[error] oldConnH delete failed (%d)\n", oldConnH);
                }
            }
            
            this->byId[devId]->connected = true;
            this->byConn[connH] = this->byId[devId];
            outputJ["peer"] = devId;
        }
        break;

        case BLE_EVENT_onDisconnected:
        {
            uint16_t connH = inputJ["conn"];
            auto it = this->byConn.find(connH); 
            if(it == this->byConn.end()) {
                Serial.printf("[info] old disconnected message, skip (%d)\n", connH);
                skip =true;
            }
            else {
                it->second->connected = false;
                outputJ["peer"] = it->second->devId;
            }
        }
        break;

        case BLE_EVENT_onCharacteristicDiscovery:
        {
            uint16_t connH = inputJ["conn"];
            if(this->byConn.find(connH) == this->byConn.end()) {
                outputJ["err_code"] = 2;
            }
            else {
                uint16_t charU = inputJ["char"];
                uint16_t valH = inputJ["valh"];
                if(HAMessBroker::isTrackedChar(charU)) {
                    this->addAttrib(this->byConn[connH], charU, valH);
                    outputJ["peer"] = this->byConn[connH]->devId;
                    outputJ["char"] = charU;
                }
                else {
                    skip=true;
                }
            }
        }
        break;

        case BLE_EVENT_onHVX:
        case BLE_EVENT_onDataRead:
        {
            uint16_t connH = inputJ["conn"];
            if(this->byConn.find(connH) == this->byConn.end()) {
                outputJ["err_code"] = 3;
            }
            else {
                uint32_t val = inputJ["val"];
                uint16_t valH = inputJ["valh"];
                uint16_t charU = this->getCharU(this->byConn[connH], valH);
                if(HAMessBroker::isTrackedChar(charU)) {
                    outputJ["peer"] = this->byConn[connH]->devId;
                    outputJ["char"] = charU;
                    outputJ["val"] = val;
                }
                else {
                    skip=true;
                    Serial.printf("[error] invalid value handle (%d)\n", valH);
                }
            }
        }
        break;

        default:
            return buff.c_str();
    }

    if(skip)
        return "";
    buff="";
    // if(!skip) {
    //     outputJ["evt"] = event;
    //     outputJ.printTo(buff);
    // }
    outputJ["evt"] = event;
    outputJ.printTo(buff);
    return buff.c_str();
}


bool HAMessBroker::isTrackedChar(uint16_t charU) {
    // Serial.printf("(%x)-(%x)-(%x)-(%d) \n", 
    //     charU, charU & 0xA100, charU & 0xA200, (charU & 0xA100)==0xA100 || (charU & 0xA200)==0xA200);
    return (charU & 0xA100)==0xA100 || (charU & 0xA200)==0xA200 ;
}

void HAMessBroker::addAttrib(std::shared_ptr<HADev_t> rec, uint16_t charU, uint16_t valH) {
    auto it = rec->attribs.begin();
    while(it!=rec->attribs.end() && it->charUuid!=charU) it++;
    if(it == rec->attribs.end()) {
        HAAttrib_t attrib {charU, valH, 0};
        rec->attribs.push_back(attrib);
    }
    else {
        it->valHandle = valH;
    }
}

uint16_t HAMessBroker::getCharU(std::shared_ptr<HADev_t> rec, uint16_t valH) const {
    auto it = rec->attribs.begin();
    while(it!=rec->attribs.end() && it->valHandle!=valH) it++;
    return it==rec->attribs.end() ? 0: it->charUuid;
}


// peer, char, val
const char* HAMessBroker::fromMqtt(String& buff) {
    StaticJsonBuffer<256> inputBuff;
    JsonObject& inputJ = inputBuff.parseObject(buff);
    if (!inputJ.success()) {
        Serial.println("[error] fromMqtt: parseObject() failed");
        return "";
    }

    uint16_t devId = inputJ["peer"];
    uint16_t charU = inputJ["char"];
    bool read=true;

    uint16_t connH;
    if(this->byId.find(devId)==this->byId.end()) {
        Serial.printf("[error] no such device (%d)\n", devId);
        return "";
    }
    else {
        connH = this->byId[devId]->connHandle;
    }

    auto it = this->byId[devId]->attribs.begin();
    while(it!=this->byId[devId]->attribs.end() && it->charUuid!=charU) it++;
    if(it == this->byId[devId]->attribs.end()) {
        Serial.printf("[error] no such charUuuid (%d)/(%x)\n", devId, charU);
        return "";
    }

    if(inputJ.containsKey("val")) {
        if(charU == LED_STATE_CHARACTERISTIC_UUID
                || charU == DIMMER_STATE_CHARACTERISTIC_UUID
                || charU == RGBLED_STATE_CHARACTERISTIC_UUID
                || charU == PLUG_STATE_CHARACTERISTIC_UUID) {
            read=false;
        }
    }

    StaticJsonBuffer<256> outputBuff;
    JsonObject& outputJ = outputBuff.createObject();
    outputJ["conn"] = connH;
    outputJ["handle"] = it->valHandle;
    if(read) {
        outputJ["cmd"]=0;
    }
    else {
        outputJ["cmd"]=1;
        outputJ["val"] = inputJ["val"];
        if(charU == RGBLED_STATE_CHARACTERISTIC_UUID) {
            outputJ["size"] = 4;
        }
        else {
            outputJ["size"] = 1;
        }
    }


    buff = "";
    outputJ.printTo(buff);
    return buff.c_str();
}



#endif