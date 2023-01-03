#ifndef VOLTMETER_H
#define VOLTMETER_H

#include <Arduino.h>

class Voltmeter {

public:
	Voltmeter(unsigned int sensorPin, unsigned int maxVoltage, unsigned int lowerBound = 0, unsigned int upperBoundB = 1023);
	float getVoltage();
	void initialize();
protected:

private:
	unsigned int maxVoltage = 0;
	unsigned int sensorPin;
	unsigned int lowerBound;
	unsigned int upperBound;
};

#endif
