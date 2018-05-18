#include <WiFi.h>


// gpio 16/17
// https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/HardwareSerial.cpp

HardwareSerial Serial2(2);

const int statusLed = 33;
int status = HIGH;

void setup() {
    delay(100);
    Serial2.begin(9600);
    Serial.begin(9600);
    pinMode(statusLed, OUTPUT);
    digitalWrite(statusLed, status);
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
            Serial.printf("%s\n", buff);
            i=0;
            status = !status;
            digitalWrite(statusLed, status);
        }
    }
}
