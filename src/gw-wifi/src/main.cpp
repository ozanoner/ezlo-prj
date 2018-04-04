

#include <WiFi.h>


// gpio 16/17
// https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/HardwareSerial.cpp

HardwareSerial Serial2(2);

unsigned long t;

void setup() {
    delay(100);
    Serial2.begin(9600, SERIAL_8N1);
    Serial.begin(960, SERIAL_8N1);

    Serial.printf("started");
    t = millis()+1000;
}

int i=0;

void loop() {
    while(Serial2.available()) {
        Serial.printf("%c\n", (char)Serial2.read());
    }
    while(Serial.available()) {
        Serial.printf("%c\n", (char)Serial.read());
    }
    if(t<millis()) {
        Serial2.printf("%d\n",i++);
        t = millis()+1000;
    }
}
