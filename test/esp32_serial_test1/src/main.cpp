#include <WiFi.h>


// gpio 16/17
// https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/HardwareSerial.cpp

HardwareSerial Serial2(2);


void setup() {
    delay(100);
    Serial2.begin(9600);
    Serial.begin(9600);

    Serial.printf("started");
}

int i=0;

void loop() {
    // while(Serial.available()) {
    //     Serial.printf("r0(%c)", (char)Serial.read());
    // }
    char buff[64];
    uint8_t i=0;

    while(Serial2.available()) {
        char c = Serial2.read();
        buff[i++] = c;
        if(c=='\n') {
            buff[--i]=0;
            Serial2.printf("ok\n");
            Serial.printf("%s\n", buff);
            i=0;
        }
    }
}
