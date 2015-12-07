// evan kahn
// turns the motor off

#include "easyPIO.h"

int main() {
	pioInit();
	pinMode(22, OUTPUT);
	digitalWrite(22, 0);
}
