#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

/////////////////////////////////////////////////////////////////////
// Constants
/////////////////////////////////////////////////////////////////////


// GPIO FSEL Types
#define INPUT  0
#define OUTPUT 1
#define ALT0   4
#define ALT1   5
#define ALT2   6
#define ALT3   7
#define ALT4   3
#define ALT5   2

#define GPFSEL   ((volatile unsigned int *) (gpio + 0))
#define GPSET    ((volatile unsigned int *) (gpio + 7))
#define GPCLR    ((volatile unsigned int *) (gpio + 10))
#define GPLEV    ((volatile unsigned int *) (gpio + 13))

// Physical addresses
#define BCM2836_PERI_BASE        0x3F000000
#define TIMER_BASE 				(BCM2836_PERI_BASE + 0x3000)
#define GPIO_BASE               (BCM2836_PERI_BASE + 0x200000)
#define SPI_BASE 				(BCM2836_PERI_BASE + 0x204000)
#define BLOCK_SIZE (4*1024)

// Pointers that will be memory mapped when pioInit() is called
volatile unsigned int *gpio; //pointer to base of gpio
volatile unsigned int *timer; //pointer to timer base address
volatile unsigned int *spi;  //pointer to SPI base address

void pinMode(int pin, int function) { // sets mode of selected GPIO pin
	int reg = pin/10;
	int offset = (pin%10)*3;
	GPFSEL[reg] &= ~((0b111 & ~function) << offset);
	GPFSEL[reg] |= ((0b111 & function) << offset);
}

void spiInit(int freq){//, int settings) {
	pinMode(8, OUTPUT);  // CEOb
	pinMode(9, ALT0);  // MISO
	pinMode(10, ALT0); // MOSI
	pinMode(11, ALT0); // SCLK
	spi[2] = 250000000/freq; // Set SPI clock divider to desired freq
	spi[0]|= 0x00000080; // set "transfer active" bit
}

char spiSendReceive(char send){
	spi[1] = send; // Send data to slave
	while (!(spi[0] & 0x00010000)); // Wait until SPI complete
	return spi[1]; // Return received data
}

void digitalWrite(int pin, int val) { // writes to selected pin
	int reg = pin / 32;
	int offset = pin % 32;
	if (val) GPSET[reg] = 1 << offset;
	else GPCLR[reg] = 1 << offset;
}

void pioInit() {
	int  mem_fd;
	void *reg_map, *timer_map, *spi_map;

	// /dev/mem is a psuedo-driver for accessing memory in the Linux filesystem
	if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
	      printf("can't open /dev/mem \n");
	      exit(-1);
	}

	reg_map = mmap(
	  NULL,             //Address at which to start local mapping (null means don't-care)
      BLOCK_SIZE,       //Size of mapped memory block
      PROT_READ|PROT_WRITE, // Enable both reading and writing to the mapped memory
      MAP_SHARED,       // This program does not have exclusive access to this memory
      mem_fd,           // Map to /dev/mem
      GPIO_BASE);       // Offset to GPIO peripheral

	if (reg_map == MAP_FAILED) {
      printf("gpio mmap error %d\n", (int)reg_map);
      close(mem_fd);
      exit(-1);
    }
	timer_map = mmap(
	  NULL,
	  BLOCK_SIZE,
	  PROT_READ|PROT_WRITE,
	  MAP_SHARED,
	  mem_fd,
	  TIMER_BASE);

	spi_map = mmap(
	  NULL,
	  BLOCK_SIZE,
	  PROT_READ|PROT_WRITE,
	  MAP_SHARED,
	  mem_fd,
	  SPI_BASE);

	timer = (volatile unsigned *) timer_map; // user accessible timer array
	gpio = (volatile unsigned *) reg_map;    // user accessible GPIO pins
	spi = (volatile unsigned *) spi_map;     // user accessible SPI pins
}
void delayMicros (unsigned int micros){ // time delay in microseconds
	timer[4] = timer[1] + micros;
	timer[0] = 0b0010;
	while(!(timer[0] & 0b0010));
}