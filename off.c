// evan kahn
// turns the power supply off

#include "easyPIO.h"

int main() {
	pioInit();
	pinMode(20, OUTPUT);
	digitalWrite(20, 0);
}
