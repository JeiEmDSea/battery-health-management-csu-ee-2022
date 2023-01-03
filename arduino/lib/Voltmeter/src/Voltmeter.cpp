#include "Voltmeter.h"

Voltmeter::Voltmeter(unsigned int sensorPin, unsigned int maxVoltage) {
	this->maxVoltage = maxVoltage;
	this->sensorPin = sensorPin;
}

void Voltmeter::initialize() {
	pinMode(this->sensorPin, INPUT);
}

float Voltmeter::getVoltage() {
	unsigned long value = analogRead(this->sensorPin);
	long mapped = map(value, 0, 1023, 0, (long)(this->maxVoltage * 1000));
	return (float)(mapped) / (float)1000;
}
