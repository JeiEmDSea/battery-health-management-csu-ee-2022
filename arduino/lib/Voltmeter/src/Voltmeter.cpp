#include "Voltmeter.h"

Voltmeter::Voltmeter(unsigned int sensorPin, unsigned int maxVoltage, int offset)
{
	this->maxVoltage = maxVoltage;
	this->sensorPin = sensorPin;
	this->offset = offset;
}

void Voltmeter::initialize()
{
	pinMode(this->sensorPin, INPUT);
}

float Voltmeter::getVoltage()
{
	int sensorValue = analogRead(this->sensorPin);
	double voltage = map(sensorValue, 0, 1023, 0, this->maxVoltage * 100) + offset;
	voltage /= 100;
	voltage -= 0.3;

	return voltage <= 0 ? 0 : voltage;
}
