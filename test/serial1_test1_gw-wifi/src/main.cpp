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
    while(Serial2.available()) {
        Serial.printf("%c\n", (char)Serial2.read());
    }
    Serial2.printf("%d\n",i++);
    delay(100);
}
