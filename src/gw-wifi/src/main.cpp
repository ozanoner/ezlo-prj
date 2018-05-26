

#include <NrfConn.h>
#include <WifiConn.h>
#include <MqttConn.h>

using namespace std::placeholders;


// const char* SSID = "breakingBad";
// const char* SSID_PWD = "@@oy108-33";
// const char* SSID = "ozanopo";
// const char* SSID_PWD = "baa533f161fc";

// const char* SSID = "eraltd";
// const char* SSID_PWD = "1001934448";


const char* SSID = "eZlo_Smart_House";
const char* SSID_PWD = "smart16_inHouse";


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


// gpio 16/17
// https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/HardwareSerial.cpp

HardwareSerial Serial2(2);
NrfConn nrfConn(Serial2);

WifiConn wifiConn;
MqttConnConfigT mqttCfg {
    mqttConnectedCb, mqttDisconnectedCb, 
    MQTT_CL_ID, MQTT_TOPIC_CMD, MQTT_TOPIC_STATUS, MQTT_SRV, MQTT_PORT
};
MqttConn mqttConn;



void setup() {
    delay(5000);
    Serial2.begin(9600, SERIAL_8N1);
    Serial.begin(9600, SERIAL_8N1);

    wifiConn.init(SSID, SSID_PWD);
    wifiConn.setConnectedCallback(std::bind(wifiConnectedCb));

    nrfConn.setNrfCallback(std::bind(nrfReceiveCallback, _1));

    mqttConn.init(mqttCfg);
    mqttConn.setMqttCallback(mqttCb);

    Serial.printf("[info] started");
}


void loop() {

    nrfConn.update();
    wifiConn.update();
    mqttConn.update();
    

    switch(status) {
        case StatusEnum::HA_WIFI_PENDING:
            if(!(millis()%500))
                Serial.print(".");
        break;

        case StatusEnum::HA_MQTT_PENDING:
            if(!(millis()%500))
                Serial.print(".");
        break;

        case StatusEnum::HA_READY:

        default:
        break;
    }
}


void mqttCb(char* topic, byte* payload, unsigned int length) {
    if(length>=64) // unexpected mess len
        return;

    char mqttReceiveBuffer[64];
    int i;
    for(i=0; i<length; i++)
        mqttReceiveBuffer[i] = (char)payload[i];
    mqttReceiveBuffer[i]=0;

    nrfConn.write(mqttReceiveBuffer);
    Serial.printf("[from mqtt] %s\n", mqttReceiveBuffer);
}

void nrfReceiveCallback(const char* data) {
    Serial.printf("[from nrf] %s\n", data);
    mqttConn.publish(data);
}

void wifiConnectedCb() {
    status = StatusEnum::HA_MQTT_PENDING;
    Serial.printf("[info] MQTT connecting\n");
    mqttConn.connect(true);
}

void mqttConnectedCb() {
    status = StatusEnum::HA_READY;
    Serial.printf("[info] MQTT connected\n");
}
void mqttDisconnectedCb() {
    status = StatusEnum::HA_MQTT_PENDING;
    Serial.printf("[info] MQTT disconnected\n");
}