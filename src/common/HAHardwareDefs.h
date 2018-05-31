
#ifndef HAHardwareDefs_h_
#define HAHardwareDefs_h_

#include "PinNames.h"

#define STATUS_LED	P0_9
#define TEST_BTN		P0_10

// buttons
#define BTN_BTN1		P0_30
#define BTN_BTN2		P0_31

// gateway nrf52832 & esp32
#define UART2_TX		P0_6
#define UART2_RX		P0_8


// For dimmer & plug. STPM01 pins
#define STPM01_SDA P0_11 
#define STPM01_SCL P0_12 
#define STPM01_SCS P0_14
#define STPM01_SYN P0_15

// LEDs
#define LED_WHITE_PWM P0_8
#define LEDPWR_SDA	P0_12
#define LEDPWR_SCL	P0_11
#define LEDPWR_ADDR 0x40


#endif
