#include "pwm.hpp"




uint32_t PWM::matchValue = 0;

/*
	*Currently not done*
	If PWM is set to double edge, it needs to turn on the next PWM channel by calling IOCON
*/
void PWM::setAsDoubleEdge(bool vIsDoubleEdge)
{
	isDoubleEdge = vIsDoubleEdge;

	if(port == 0 && channel > 1 && channel <= 6)	//should I have a private variable that tells us if double edge can be set instead?
	{
		if(isDoubleEdge) //may need a lookup table to activate the pins to PWM mode (for LPC_IOCON)
		{
			LPC_PWM0->PCR |= (1 << channel);
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
