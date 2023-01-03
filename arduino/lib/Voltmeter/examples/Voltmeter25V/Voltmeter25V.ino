#include "Arduino.h"
#include <Voltmeter.h>

const uint8_t PIN_VOLTMETER = A0;
Voltmeter voltmeter(PIN_VOLTMETER, 25);

void setup() {
	pinMode(PIN_VOLTMETER, INPUT);
	Serial.begin(57600);
	voltmeter.initialize();
}
void loop() {
  float volt = voltmeter.getVoltage();
	Serial.println(volt);
	delay(100);
}
