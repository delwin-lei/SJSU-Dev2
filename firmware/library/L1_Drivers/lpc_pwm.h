#ifndef LPC_PWM_H_
#define LPC_PWM_H_

#include "LPC40xx.h"

class PWM
{
public:
	PWM(bool vPort, uint8_t vChannel, uint32_t frequencyHz);
	~PWM();
	void setDutyCycle(float percentage);
	float getDutyCycle();
	void setFrequency(uint32_t frequencyHz);
	uint32_t getFrequency();
	void setAsDoubleEdge(bool vIsDoubleEdge = true);
	bool getEdge();

private:
	bool isDoubleEdge;
	bool port;
	uint8_t channel;
};

#endif LPC_PWM_H_