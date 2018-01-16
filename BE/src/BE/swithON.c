/*switchON.c
*Allume une diode branchée à la PIN choisie
*Utilisation switchON <NumPin>
*/

#include <stdio.h>
#include <stdlib.h>
#include "GPIO.h"

int main(int argc, char* argv[])
{
	int pin;


	if(argc != 2)
	{
		printf("switchON <NumPinGPIO>\n");
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
			//initialistation de la pin GPIO
			GPIOExport(pin);
			GPIODirection(pin, OUT);

			//On enoie le courant dans la PIN (et donc on allume la LED)
			GPIOWrite(pin, 30);
		}
	}

	return 0;
}
