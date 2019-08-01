/*

	PID Controller...

	based off wikipedia's entry on PID Controllers.
*/

#include <unistd.h>
#include <iostream>

int main()
{
	srand(time(NULL));

	int setpoint = rand();

	int measured_value = 0;
	int previous_error = 0;
	int integral = 0;
	int dt = 1;
	double Kp = 1.0;
	double Ki = 1.2;
	double Kd = 3.0 / 40.0; // setting this to 0 is actually better.

	while (1) {
		std::cout << "Target: " << setpoint << " Current: " << measured_value << std::endl;

		int error = setpoint - measured_value;
		integral = integral + error * dt;
		int derivative = (error - previous_error) / dt;
		int output = Kp * error + Ki * integral + Kd * derivative;
		previous_error = error;

		measured_value += output;

		sleep(dt);
	}
}
