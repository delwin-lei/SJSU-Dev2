#pragma once 

#include <cstdint>
#include <cstdio>
#include "L0_LowLevel/LPC40xx.h"
#include "L1_Drivers/pin_configure.hpp"

//a stub until we get the system clock driver files
uint32_t getSystemClock()
{
	return 48'000'000;
}

class PWMInterface
{
	public:
		virtual void Initialize(uint32_t frequencyHz = 100) = 0;
		virtual void SetDutyCycle(float dutyCycle) = 0;
		virtual float GetDutyCycle() = 0;
		virtual void SetFrequency(uint32_t frequencyHz) = 0;
		virtual uint32_t GetFrequency() = 0;
		virtual void SetAsDoubleEdge(bool vIsDoubleEdge = true) = 0;
		virtual bool IsDoubleEdge() = 0;
};

class PWM : public PWMInterface
{
	public:

		static constexpr uint8_t pwmFunction = 0b001;

		static constexpr uint8_t defaultFunction = 0b000;

		PWM(uint8_t vPPort, uint8_t vPPin):
			pwm_(pwm_pin_),
			pwm_pin_(vPPort, vPPin)
		{
		}

		~PWM()
		{
			//0b000 is GPIO
			pwm_pin_.SetPinFunction(defaultFunction);
		}

		void Initialize(uint32_t frequencyHz = 100) final override
		{
			//DIRECT MANIUPULATION OF SYSTEM CLOCK REGISTERS; WILL REPLACE WITH FUNCTIONS LATER
			//Enables PWM1 power/clock control bit
			LPC_SC->PCONP |= (1 << 6);
			//input clock is divided by 1
			LPC_SC->PCLKSEL = (1 << 0);
			//END OF DIRECT MANIPULATION

			matchValue = getSystemClock()/frequencyHz;

			//Resets PWMTC on Match with MR0
			LPC_PWM1->MCR |= (1 << 1);
			LPC_PWM1->MR0 = matchValue;

			//Enables PWM TC and PC for counting and enables PWM mode
			LPC_PWM1->TCR = (1<< 0) | (1 << 3);
			LPC_PWM1->CTCR &= ~(0xF << 0);

			//sets port to proper function and get pwm channel
			if(pwm_pin_.getPort() == 2)
			{
				//0b010 is PWM
				pwm_pin_.SetPinFunction(pwmFunction);
				port = 1;
				channel = pwm_pin_.setPin() + 1;		//pwm.getPin()	
			}

			//Enables PWM[channel] output
			LPC_PWM1->PCR |= (1 << (channel + 8));
		}

		void SetDutyCycle(float percentage) final override
		{
			if(percentage < 0 || percentage > 100)
			{
				return;
			}
			dutyCycle = (percentage * matchValue) / 100;

			switch(channel)
			{
				case 1: LPC_PWM1->MR1 = dutyCycle;
				case 2: LPC_PWM1->MR2 = dutyCycle;
				case 3: LPC_PWM1->MR3 = dutyCycle;
				case 4: LPC_PWM1->MR4 = dutyCycle;
				case 5: LPC_PWM1->MR5 = dutyCycle;
				case 6: LPC_PWM1->MR6 = dutyCycle;
				default: return;
			}
			LPC_PWM1->LER |= (1 << channel);
		}

		float GetDutyCycle() final override
		{
			return dutyCycle;
		}

		void SetFrequency(uint32_t frequencyHz) final override
		{
			//disables PWM mode; this will reset all counters to 0 and allow us to update MR0
			LPC_PWM1->TCR &= ~(1 << 3);

			matchValue = getSystemClock() / frequencyHz;
			LPC_PWM1->MR0 = matchValue;

			//re-enable PWM mode
			LPC_PWM1->TCR |= (1 << 3);
		}

		uint32_t GetFrequency() final override
		{
			return getSystemClock() / matchValue;
		}

		void SetAsDoubleEdge(bool vIsDoubleEdge = true) final override
		{
			//TODO: Set related registers to double edge mode
		}

		bool IsDoubleEdge() final override
		{
			return isDoubleEdge;
		}

	private:
		PinConfigureInterface &pwm_;

		PinConfigure pwm_pin_;

		static uint32_t matchValue;
		bool isDoubleEdge;
		
		//will signify current PWM0/1 and channel
		bool port;
		uint8_t channel;

		float dutyCycle;	//duty cycle of pwm 
};
