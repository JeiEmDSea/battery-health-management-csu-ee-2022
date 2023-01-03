#ifndef VOLTMETER_H
#define VOLTMETER_H

#include <Arduino.h>

class Voltmeter {

public:
	Voltmeter(unsigned int sensorPin, unsigned int maxVoltage);
	float getVoltage();
	void initialize();
protected:

private:
	unsigned int maxVoltage = 0;
	unsigned int sensorPin;
};

#endif
