#include "lpc_pwm.h"

PWM::PWM(bool vPort, uint8_t vChannel, uint32_t frequencyHz)
{

}

PWM::~PWM()	//might need the private variable for double edge to turn off properly
{
	if(port == 0)
	{
		LPC_PWM0->PCR &= ~(1 << (channel+8));
	}
	else if(port == 1)
	{
		LPC_PWM1->PCR &= ~(1 << (channel+8));
	}
}

void PWM::setDutyCycle(float percentage)
{

}

float PWM::getDutyCycle()
{

}

void PWM::setFrequency(uint32_t frequencyHz)
{

}

uint32_t PWM::getFrequency()
{

}

void PWM::setAsDoubleEdge(bool vIsDoubleEdge = true)
{
	isDoubleEdge = vIsDoubleEdge;

	if(port == 0 && channel > 1 && channel <= 6)	//should I have a private variable that tells us if double edge can be set instead?
	{
		if(isDoubleEdge) //may need a lookup table to activate the pins to PWM mode (for LPC_IOCON)
		{
			LPC_PWM0->PCR |= (1 << channel);
			LPC_IOCON
		}	
		else
		{
			LPC_PWM0->PCR |= (0 << channel);
		}
	}

	else if(port == 1 && channel > 1 && channel <= 6)	//should I have a private variable that tells us if double edge can be set instead?
	{
		if(isDoubleEdge)
		{
			LPC_PWM1->PCR |= (1 << channel);
		}	
		else
		{
			LPC_PWM1->PCR |= (0 << channel);
		}
	}
}

bool PWM::getEdge()
{
	return isDoubleEdge;
}
