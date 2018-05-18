#ifndef NrfConn_h_
#define NrfConn_h_

#include <Stream.h>
#include <functional>

using NrfCallbackT = std::function<void(const char*)>;


class NrfConn {
private:
    Stream& nrfConn;
    NrfCallbackT cb;
    char buff[64];
    int buffi;
public:

    NrfConn(Stream& s): nrfConn(s), cb(nullptr), buffi(0) {  }

    void setNrfCallback(NrfCallbackT cbFunc) {
        this->cb = cbFunc;
    }

    void update() {
        while(this->nrfConn.available()) {
            char c = (char)this->nrfConn.read();
            bool rts = c=='\n';
            if(rts) {
                buff[buffi]=0;
                buffi=0;
                if(this->cb!=nullptr)
                    this->cb(this->buff);
            }
            else {
                if(buffi<64)
                    buff[buffi++]=c;
                else {
                    // buffer overflow
                }
            }
        }
    }

    void write(const char* data) {
        nrfConn.print(data);
        nrfConn.print("\n");
    }

};


#endif