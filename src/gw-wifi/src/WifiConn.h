#ifndef WifiConn_h_
#define WifiConn_h_


#include <functional>
#include <WiFi.h>
// #include <WiFiMulti.h>

using WifiConnectedCallbackT = std::function<void(void)>;

class WifiConn {
private:
    WifiConnectedCallbackT cb;
    bool pending;
    unsigned long resetTimout;
    const char* ssid;
    const char* pwd;
public:

    WifiConn(const char* ssid, const char* pwd, WifiConnectedCallbackT f=nullptr): 
            cb(f), pending(true), ssid(ssid), pwd(pwd) { 
        WiFi.mode(WIFI_OFF);
        delay(2000);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, pwd);
        this->resetTimout = millis()+10000;
    } 

    void reset() {
        Serial.print("\n[info] WiFi reset \n");
        WiFi.disconnect(true);
        delay(2000);
        WiFi.mode(WIFI_OFF);
        delay(2000);
        WiFi.persistent(true);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, pwd);
        this->resetTimout = 0;
        Serial.print("\n[info] WiFi reset completed \n");
    }
    // void addAP(const char* ssid, const char* pwd) {
    //     WifiConn::wifiMulti.addAP(ssid, pwd);
    // }

    void setConnectedCallback(WifiConnectedCallbackT f) {
        this->cb = f;
    }

    // bool isConnected() const { 
    //     return WiFi.status() == WL_CONNECTED; 
    // }

    void update() {
        if(this->pending) {
            delay(50);

            if(WiFi.status() == WL_CONNECTED) {
                this->pending=false;
                if(this->cb!=nullptr)
                    this->cb();
            }
            else if(this->resetTimout && this->resetTimout<millis()) {
                this->reset();
            }
        }
    }

};

#endif