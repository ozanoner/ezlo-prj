

// #include <WiFi.h>
#include <PubSubClient.h>
#include <NrfConn.h>
#include <WifiConn.h>
#include <MqttConn.h>

using namespace std::placeholders;

// WiFiClient wifi;
// PubSubClient mqtt(wifi);


// const char* SSID = "breakingBad";
// const char* SSID_PWD = "@@oy108-33";
const char* SSID = "ozanopo";
const char* SSID_PWD = "baa533f161fc";

// const char* SSID = "eraltd";
// const char* SSID_PWD = "1001934448";


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

StatusEnum status;

char nrfReceiveBuffer[64];
int nrfRBI=0;
bool rts=false; // if c==\n then ready to send
char nrfSendBuffer[64];
char mqttReceiveBuffer[64];
char mqttSendBuffer[64];

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

unsigned long nextLoop;




void setup() {
    delay(5000);
    Serial2.begin(9600, SERIAL_8N1);
    Serial.begin(9600, SERIAL_8N1);

    // WiFi.mode(WIFI_STA);
    // WiFi.begin(SSID, SSID_PWD);
    // Serial.print("\nWiFi connecting ");
    wifiConn.init(SSID, SSID_PWD);
    wifiConn.setConnectedCallback(std::bind(wifiConnectedCb));
    status = StatusEnum::HA_WIFI_PENDING;

    nrfConn.setNrfCallback(std::bind(nrfReceiveCallback, _1));

    // mqttCfg.messageCallback = mqttCb;
    // mqttCfg.clientId = MQTT_CL_ID;
    // mqttCfg.commandTopic = MQTT_TOPIC_CMD;
    // mqttCfg.serverAddress = MQTT_SRV;
    // mqttCfg.serverPort = MQTT_PORT;
    // mqttCfg.statusTopic = MQTT_TOPIC_STATUS;

    mqttConn.init(mqttCfg);
    mqttConn.setMqttCallback(mqttCb);

    // mqtt.setServer(MQTT_SRV, MQTT_PORT); 
    // mqtt.setCallback(mqttCb);


    Serial.printf("[info] started");
}

int i=0;
int tick =0;

void loop() {
    
    // while(Serial2.available()) {
    //     char c = (char)Serial2.read();
    //     rts = c=='\n';
    //     if(rts) {
    //         nrfReceiveBuffer[nrfRBI]=0;
    //         sprintf(mqttSendBuffer, "%s", nrfReceiveBuffer);
    //         nrfRBI = 0;
    //     }
    //     else {
    //         if(nrfRBI<64)
    //             nrfReceiveBuffer[nrfRBI++]=c;
    //         else {
    //             // buffer overflow
    //         }
                
    //     }

    // }

    nrfConn.update();
    wifiConn.update();
    mqttConn.update();
    

    switch(status) {
        case StatusEnum::HA_WIFI_PENDING:
        /*
            if(WiFi.status() == WL_CONNECTED) {
                status = StatusEnum::HA_MQTT_PENDING;
                mqtt.connect(MQTT_CL_ID);
                nextLoop = millis()+5000; // next mqtt connect
                Serial.print("\nMQTT connecting ");
            }
            else {
                if(!(millis()%500))
                    Serial.print(".");
            }
        */
            if(!(millis()%500))
                Serial.print(".");
        break;

        case StatusEnum::HA_MQTT_PENDING:
            // if(mqtt.connected()) {
            //     Serial.print("\nReady\n");
            //     mqtt.loop();
            //     mqtt.subscribe(MQTT_TOPIC_CMD);
            //     status = StatusEnum::HA_READY;
            //     nextLoop = millis()+10000; // next bme280 read
            // }
            // else {
            //     if(nextLoop < millis()) {
            //         mqtt.connect(MQTT_CL_ID);
            //         nextLoop = millis()+5000; // next mqtt connect
            //         Serial.print("\nMQTT connecting ");
            //     }
            //     else {
            //         if(!(millis()%500))
            //             Serial.print(".");
            //     }
            // }
            if(!(millis()%500))
                Serial.print(".");
        break;

        case StatusEnum::HA_READY:
            // if(mqtt.connected()) {
            //     mqtt.loop();

            //     if(rts) {
            //         mqtt.publish(MQTT_TOPIC_STATUS, mqttSendBuffer);
            //         rts = false;
            //     }

              
            // }
            // else {
            //     status = StatusEnum::HA_MQTT_PENDING;
            //     mqtt.connect(MQTT_CL_ID);
            //     nextLoop = millis()+5000; // next mqtt connect
            //     Serial.print("\nMQTT connecting ");
            // }
            

        break;

        default:
        break;
    }
}


void mqttCb(char* topic, byte* payload, unsigned int length) {

    if(length>=64) // unexpected mess len
        return;
    int i;
    for(i=0; i<length; i++)
        mqttReceiveBuffer[i] = (char)payload[i];
    mqttReceiveBuffer[i]=0;
    
    // Serial2.print(mqttReceiveBuffer);
    // Serial2.print("\n");
    Serial.printf("[from mqtt] %s\n", mqttReceiveBuffer);
    nrfConn.write(mqttReceiveBuffer);
}

void nrfReceiveCallback(const char* data) {
    Serial.printf("[from nrf] %s\n", data);
    mqttConn.publish(data);
    // if(mqtt.connected()) {
    //     mqtt.publish(MQTT_TOPIC_STATUS, data);
    // } 
    // else {

    // }

}

void wifiConnectedCb() {
    status = StatusEnum::HA_MQTT_PENDING;
    // mqtt.connect(MQTT_CL_ID);
    Serial.printf("[info] MQTT connecting\n");
    mqttConn.connect(true);
    // nextLoop = millis()+5000; // next mqtt connect
}

void mqttConnectedCb() {
    status = StatusEnum::HA_READY;
    Serial.printf("[info] MQTT connected\n");
}
void mqttDisconnectedCb() {
    status = StatusEnum::HA_MQTT_PENDING;
    Serial.printf("[info] MQTT disconnected\n");
}