#include "L5_Testing/testing_frameworks.hpp"

#include "lpc_pwm.h"

TEST_CASE("Testing PWM initialization", "[pwm]")
{
	PWM pwm2_0(2, 0, 100);
	REQUIRE(pwm2_0.pport == 2);
	REQUIRE(pwm2_0.ppin == 0);
	REQUIRE(pwm2_0.port == 1);
	REQUIRE(pwm2_0.channel == 1);
}