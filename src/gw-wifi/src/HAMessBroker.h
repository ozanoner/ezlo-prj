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

    std::string fromMqtt(const std::string& mess) {
        return {};
    }
};

const char* HAMessBroker::fromNrf(String& buff) {
    StaticJsonBuffer<256> inputBuff;
    JsonObject& inputJ = inputBuff.parseObject(buff);
    if (!inputJ.success()) {
        Serial.println("[error] fromNrf: parseObject() failed");
        buff="{\"err_code\":1}";
        return buff.c_str();
    }
    if(!inputJ.containsKey("evt")) {
        Serial.println("[error] fromNrf: parseObject() failed");
        buff="{\"err_code\":1}";
        return buff.c_str();
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
            this->byId[devId]->connected = true;
            this->byConn[connH] = this->byId[devId];
            outputJ["peer"] = devId;
        }
        break;

        case BLE_EVENT_onDisconnected:
        {
            uint16_t connH = inputJ["conn"];
            if(this->byConn.find(connH) == this->byConn.end()) {
                outputJ["err_code"] = 1;
            }
            else {
                outputJ["peer"] = this->byConn[connH]->devId;
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
                    Serial.printf("[error] invalid value handle %d\n", valH);
                }
            }
        }
        break;

        default:
            return buff.c_str();
    }

    buff="";
    if(!skip) {
        outputJ["evt"] = event;
        outputJ.printTo(buff);
    }
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
#endif