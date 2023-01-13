#include "Voltmeter.h"

Voltmeter::Voltmeter(unsigned int sensorPin, unsigned int maxVoltage, unsigned int lowerBound = 0, unsigned int upperBound = 1023)
{
	this->maxVoltage = maxVoltage;
	this->sensorPin = sensorPin;
	this->lowerBound = lowerBound;
	this->upperBound = upperBound;
}

void Voltmeter::initialize()
{
	pinMode(this->sensorPin, INPUT);
}

float Voltmeter::getVoltage()
{
	unsigned long value = ((analogRead(this->sensorPin) + (5 / 2)) / 5) * 5;
	long mapped = map(value, this->lowerBound, this->upperBound, 0, (long)(this->maxVoltage * 1000));
	float result = (float)(mapped) / (float)1000;
	return result > 0 ? result : 0;
}
