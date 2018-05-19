#ifndef EspConn_h_
#define EspConn_h_

#include <Serial.h>
// use mbed::Callback instead of std::function
// #include <functional>
#include <mbed_events.h>
#include <mbed.h>
#include <cstdarg>
#include <memory>
#include <string>
#include "SEGGER_RTT.h"

using namespace mbed;

class EspConn {
private:
    Serial& conn;
    char buff[64];
    int buffi;
    mbed::Callback<void(std::shared_ptr<const char*>)> cb;
    EventQueue& evq;    
public:

EspConn(Serial& serial, EventQueue& eventQueue): 
    conn(serial), buffi(0), cb(nullptr), evq(eventQueue) { }

void init(mbed::Callback<void(std::shared_ptr<const char*>)> callback) {
    this->conn.set_flow_control(mbed::SerialBase::Flow::Disabled, NC, NC);
    this->conn.format(8, SerialBase::None, 1);
    this->conn.set_blocking(false);
    this->cb = callback;
    this->conn.attach(this, &EspConn::update);
}

void send(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(conn, fmt, args);
    va_end(args);
    conn.printf("\n");
}

void update() {
    char c;
    while(conn.readable()) {
        c = conn.getc();
        if(c=='\n') {
            // buff[buffi<64?buffi:63]=0;
            buff[buffi]=0;
            buffi=0;
            DPRN("[info] new data: %s", buff);
            if(cb!=nullptr) {
                DPRN("[info] callback for send", buff);
                string s(buff);
                std::shared_ptr<const char*> data = std::make_shared<const char*>(s.c_str());
                // evq.call(cb, data);
                cb(data);
            }
        }
        else if(buffi<64) {
            buff[buffi++]=c;
        }
    }
}



};


#endif