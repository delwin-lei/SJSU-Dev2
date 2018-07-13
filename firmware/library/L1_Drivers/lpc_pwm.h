#ifndef LPC_PWM_H_
#define LPC_PWM_H_

#include <cstdint>
#include <cstdio>
#include "L0_LowLevel/LPC40xx.h"

class PWM
{
public:
	PWM(uint8_t vPPort, uint8_t vPPin, uint32_t frequencyHz = 50);
	~PWM();
	void setDutyCycle(float percentage);
	float getDutyCycle();
	void setFrequency(uint32_t frequencyHz);
	uint32_t getFrequency();
	void setAsDoubleEdge(bool vIsDoubleEdge = true);
	bool getEdge();

private:
	static uint32_t matchValue;
	bool isDoubleEdge;
	
	//will signify current PWM0/1 and channel
	bool port;
	uint8_t channel;

	//these will be used to identify which pin on the board is used and will activate that pin to pwm
	uint8_t pport;		//port number of pin
	uint8_t ppin;		//pin number

	float dutyCycle;	//duty cycle of pwm 
};

#endif LPC_PWM_H_