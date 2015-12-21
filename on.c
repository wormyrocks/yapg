// evan kahn
// turns the motor on

#include "easyPIO.h"

int main() {
	pioInit();
	pinMode(20, OUTPUT);
	digitalWrite(20, 1);
}
