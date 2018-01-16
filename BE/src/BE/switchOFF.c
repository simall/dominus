/*switchOFF.c
*2teint une diode branchée à la PIN choisie
*Utilisation switchOFF <NumPin>
*/

#include <stdio.h>
#include <stdlib.h>
#include "GPIO.h"

int main(int argc, char* argv[])
{
	int pin;


	if(argc != 2)
	{
		printf("switchOFF <NumPinGPIO>\n");
	}
	else
	{
		pin = atoi(argv[1]);
		if(pin == 0)
		{
			printf("Unknown pin number...");
		}
		else
		{
			//On éteint la LED
			GPIOWrite(pin, 0);

			//On ferme proprement la PIN
			GPIOUnexport(pin);
		}
	}

	return 0;
}