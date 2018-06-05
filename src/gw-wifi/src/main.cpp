

#include "NrfConn.h"
#include "WifiConn.h"
#include "MqttConn.h"
#include "StatusLed.h"
#include "HAMessBroker.h"
#include "ProgressDisp.h"

#include <cstring>

using namespace std::placeholders;


const char* SSID1 = "breakingBad";
const char* SSID_PWD1 = "@@oy108-33";
const char* SSID2 = "ozanopo";
const char* SSID_PWD2 = "baa533f161fc";

// const char* SSID = "eraltd";
// const char* SSID_PWD = "1001934448";


// const char* SSID = "eZlo_Smart_House";
// const char* SSID_PWD = "smart16_inHouse";


static StatusLed statusLed;

const char* MQTT_SRV = "34.241.70.227";
const int MQTT_PORT  = 1883;
const char* MQTT_CL_ID = "ha11111";
// topic to receive commands from frontend apps, subscribe
const char* MQTT_TOPIC_CMD = "ha/ha11111/cmd";
// device status topic, publish
const char* MQTT_TOPIC_STATUS = "ha/ha11111/status";

enum class StatusEnum {
    HA_WIFI_PENDING,
    HA_MQTT_PENDING,
    HA_READY
};
StatusEnum status = StatusEnum::HA_WIFI_PENDING;


void mqttCb(char* topic, byte* payload, unsigned int length);
void nrfReceiveCallback(const char* data);
void wifiConnectedCb();
void mqttConnectedCb();
void mqttDisconnectedCb();


MqttConnConfigT mqttCfg {
    mqttConnectedCb, mqttDisconnectedCb, 
    MQTT_CL_ID, MQTT_TOPIC_CMD, MQTT_TOPIC_STATUS, MQTT_SRV, MQTT_PORT
};


// gpio 16/17
// https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/HardwareSerial.cpp

HardwareSerial Serial2(2);
NrfConn nrfConn(Serial2);

ProgressDisp progress;
WifiConn wifiConn(SSID2, SSID_PWD2);

MqttConn mqttConn;

HAMessBroker messBroker;


void setup() {

    delay(5000);
    Serial2.begin(9600, SERIAL_8N1);
    Serial.begin(9600, SERIAL_8N1);

    statusLed.setBlinkPeriod(1000);
    wifiConn.setConnectedCallback(std::bind(wifiConnectedCb));

    nrfConn.setNrfCallback(std::bind(nrfReceiveCallback, _1));

    mqttConn.init(mqttCfg);
    mqttConn.setMqttCallback(mqttCb);

    Serial.printf("[info] started. waiting wifi..\n");
    progress.enable();
}


void loop() {
    statusLed.update();
    wifiConn.update();
    progress.update();
    mqttConn.update();
    nrfConn.update();
}


void mqttCb(char* topic, byte* payload, unsigned int length) {
    if(length>=SERIAL_BUFF_SIZE) { // unexpected mess len
        Serial.printf("[error] too long message from MQTT\n");
        return;
    }

    char data[length+1];
    memcpy(data, payload, length);
    data[length]=0;

    String buff = String(data);
    const char* ret = messBroker.fromMqtt(buff);
    if(ret[0]!=0) {
        nrfConn.write(ret);
        Serial.printf("[info] nrfConn.write: %s\n", ret);
    }
    // nrfConn.write(mqttReceiveBuffer);
    // Serial.printf("[info] mqtt: %s\n", mqttReceiveBuffer);

    // nrfConn.write(messBroker.fromMqtt(std::string(mqttReceiveBuffer)).c_str());
}

void nrfReceiveCallback(const char* data) {
    Serial.printf("[info] nrf: %s\n", data);

    String buff = String(data);
    const char* ret = messBroker.fromNrf(buff);
    if(ret[0]!=0)
        mqttConn.publish(ret);
}

void wifiConnectedCb() {
    status = StatusEnum::HA_MQTT_PENDING;
    Serial.printf("[info] MQTT connecting\n");
    progress.enable();
    mqttConn.connect(true);
}

void mqttConnectedCb() {
    status = StatusEnum::HA_READY;
    Serial.printf("[info] MQTT connected\n");
    progress.disable();
}
void mqttDisconnectedCb() {
    status = StatusEnum::HA_MQTT_PENDING;
    Serial.printf("[info] MQTT disconnected\n");
}