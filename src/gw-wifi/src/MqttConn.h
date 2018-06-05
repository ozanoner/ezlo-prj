#ifndef MqttConn_h_
#define MqttConn_h_


#include <WiFi.h>
#include <functional>
#include <PubSubClient.h>

using namespace std::placeholders;


// void mqttCb(char* topic, byte* payload, unsigned int length)

using MqttConnectedCallbackT = std::function<void(void)>;
using MqttDisconnectedCallbackT = std::function<void(void)>;


typedef struct MqttConnConfig {
    MqttConnectedCallbackT connectedCallback; 
    MqttDisconnectedCallbackT disconnectedCallback;
    const char* clientId;
    const char* commandTopic;
    const char* statusTopic;
    const char* serverAddress;
    int serverPort;
} MqttConnConfigT;

class MqttConn {
private:
    WiFiClient wifi;
    PubSubClient* mqtt;

    MqttConnConfigT* config;

    bool enabled;
    unsigned long nextConn;
    
public:

MqttConn(): enabled(false), nextConn(0) { 
    this->mqtt = new PubSubClient(wifi);
}

~MqttConn() {
    this->mqtt->disconnect();
    delete mqtt;
}

void init(MqttConnConfigT& cfg) {
    this->config = &cfg;
    this->mqtt->setServer(cfg.serverAddress, cfg.serverPort);
    // Serial.printf("[info] mqtt server: %s %d\n", cfg.serverAddress, cfg.serverPort);
}

void setMqttCallback(MQTT_CALLBACK_SIGNATURE) {
    mqtt->setCallback(callback);
}

void connect(bool force=false) {
    if(force)
        this->mqtt->disconnect();
    if(this->mqtt->connected()) 
        return;
    if(this->nextConn>millis()) 
        return;

    this->enabled = true;
    if(mqtt->connect(config->clientId)) {
        mqtt->subscribe(config->commandTopic);
        if(config->connectedCallback != nullptr)
            config->connectedCallback();
        mqtt->loop();
    }
    else 
    {
        nextConn = millis()+5000;
    }
}

void disconnect() {
    enabled = false;
    mqtt->disconnect();
    if(config->disconnectedCallback!=nullptr) {
        config->disconnectedCallback();
    }
}

void update() {
    if(enabled && !mqtt->loop()) {
        this->connect();
    }
}

void publish(const char* data) {
    if(mqtt->connected())
        mqtt->publish(config->statusTopic, data);
}

};

#endif