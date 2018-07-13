#include "lpc_pwm.h"

//setMode(1, 5, 0b101);
void setMode(uint8_t port, uint8_t pin, uint8_t mode)
{
  io_con_map->con[port][pin] = (io_con_map->con[port][pin] & (0b111)) | (mode & 0b111);
}

PWM::PWM(bool vPPort, uint8_t vPPin, uint32_t frequencyHz)
{

	//this saved value will be used to during destructor
	pport = vPPort;	
	ppin = vPPin;

	//Enables PWM1 power/clock control bit
	LPC_SC->PCONP |= (1 << 6);
	//input clock is divided by 1
	LPC_SC->PCLKSEL = (1 << 0);

	matchValue = getSystemClock()/frequencyHz;

	//Resets PWMTC on Match with MR0
	LPC_PWM1->MCR |= (1 << 1);
	LPC_PWM1->MR0 = matchValue;

	//Enables PWM TC and PC for counting and enables PWM mode
	LPC_PWM1->TCR = (1<< 0) | (1 << 3);
	LPC_PWM1->CTCR &= ~(0xF << 0);

	//sets port to proper function and get pwm channel
	if(vPPort == 2)
	{
		setMode(vPPort, vPPin, 0b001);
		port = vPPort - 1;
		channel = vPPin + 1;		
	}
	else
	{
		setMode(vPPort, vPPin, 0b010);
		if(vPPort == 3 && vPPin <= 21)
		{
			port = 0;
			channel = vPPin - 15;
		}
		else if( vPPort == 3 && vPPin >= 24)
		{
			port = 1;
			channel = vPPin - 23;
		}
	}

	//Enables PWM[channel] output
	LPC_PWM1->PCR |= (1 << (channel + 8));
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
	setMode(pport, ppin, 0b000);
}

void PWM::setDutyCycle(float percentage)	//will set the respective match register to [percentage]*MR0
{
	if(percentage < 0 || percentage > 100)
	{
		return;
	}
	uint32_t dutyValue = (percentage * matchValue) / 100;

	switch(channel)
	{
		case 1: LPC_PWM1->MR1 = dutyValue;
		case 2: LPC_PWM1->MR2 = dutyValue;
		case 3: LPC_PWM1->MR3 = dutyValue;
		case 4: LPC_PWM1->MR4 = dutyValue;
		case 5: LPC_PWM1->MR5 = dutyValue;
		case 6: LPC_PWM1->MR6 = dutyValue;
		default: return;
	}
	LPC_PWM1->LER |= (1 << channel);
}

float PWM::getDutyCycle()		//will store in a variable? vs calculate it using match register
{

}

void PWM::setFrequency(uint32_t frequencyHz)	//this will set PWMEN = 0, set MR0, and set PWMEN = 1 
{
	//disables PWM mode; this will reset all counters to 0 and allow us to update MR0
	LPC_PWM1->TCR &= ~(1 << 3);

	matchValue = getSystemClock() / frequencyHz;
	LPC_PWM1->MR0 = matchValue;

	//re-enable PWM mode
	LPC_PWM1->TCR |= (1 << 3);
}

uint32_t PWM::getFrequency()		//stored vs calculate
{

}

/*
	If PWM is set to double edge, it needs to turn on the next PWM channel by calling IOCON

*/
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
