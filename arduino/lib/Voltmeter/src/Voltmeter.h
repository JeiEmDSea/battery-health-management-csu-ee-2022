#ifndef VOLTMETER_H
#define VOLTMETER_H

#include <Arduino.h>

class Voltmeter
{

public:
	Voltmeter(unsigned int sensorPin, unsigned int maxVoltage, int offset);
	float getVoltage();
	void initialize();

private:
	unsigned int sensorPin;
	unsigned int maxVoltage;
	int offset;
};

#endif
