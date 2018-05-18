#ifndef WifiConn_h_
#define WifiConn_h_


#include <functional>
#include <WiFi.h>

using WifiConnectedCallbackT = std::function<void(void)>;

class WifiConn {
private:
    WifiConnectedCallbackT cb;
    bool pending;
public:

WifiConn(WifiConnectedCallbackT f=nullptr): 
    cb(f), pending(true) { } 

void init(const char* ssid, const char* pwd) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pwd);
    // Serial.print("\nWiFi connecting ");
}

void setConnectedCallback(WifiConnectedCallbackT f) {
    this->cb = f;
}

bool isConnected() const { 
    return WiFi.status() == WL_CONNECTED; 
}

void reconnect() { 
    this->pending = WiFi.status() != WL_CONNECTED;
}

void update() {
    if(pending) {
        if(WiFi.status() == WL_CONNECTED) {
            this->pending=false;
            if(this->cb!=nullptr)
                this->cb();
        }
    }
}

};

#endif