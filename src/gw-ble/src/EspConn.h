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
// #include "SEGGER_RTT.h"

#define SERIAL_BUFF_SIZE 128

using namespace mbed;

class EspConn {
private:
    Serial& conn;
    char buff[SERIAL_BUFF_SIZE];
    int buffi;
    mbed::Callback<void(const char*)> cb;
    EventQueue& evq; 
   
public:

    EspConn(Serial& serial, EventQueue& eventQueue): 
        conn(serial), buffi(0), cb(nullptr), evq(eventQueue) { }
    
    void init(mbed::Callback<void(const char*)> callback);
    void send(const char* fmt, ...);
    void update();
    
};


void EspConn::init(mbed::Callback<void(const char*)> callback) {
    this->conn.set_flow_control(mbed::SerialBase::Flow::Disabled, NC, NC);
    this->conn.format(8, SerialBase::None, 1);
    this->conn.set_blocking(false);
    this->cb = callback;
    this->conn.attach(this, &EspConn::update);
}

void EspConn::send(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(conn, fmt, args);
    va_end(args);
    conn.printf("\n");
}

void EspConn::update() {
    char c;
    while(conn.readable()) {
        c = conn.getc();
        if(c=='\n') {
            // buff[buffi<SERIAL_BUFF_SIZE?buffi:63]=0;
            buff[buffi]=0;
            buffi=0;
            // DPRN("[info] new data: %s", buff);
            if(cb!=nullptr) {
                // DPRN("[info] callback for send", buff);
                string s(buff);
                // std::shared_ptr<const char*> data = std::make_shared<const char*>(s.c_str());
                // evq.call(cb, data);
                cb(s.c_str());
            }
        }
        else if(buffi<SERIAL_BUFF_SIZE) {
            buff[buffi++]=c;
        }
    }
}

#endif