#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#define CONTROLLER_REG_4016 0x4016 // kontroller 1
#define CONTROLLER_REG_4017 0x4017 // kontroller 2

#define CONTROLLER_BIT_OF_A		 0
#define CONTROLLER_BIT_OF_B		 1
#define CONTROLLER_BIT_OF_SELECT 2
#define CONTROLLER_BIT_OF_START	 3
#define CONTROLLER_BIT_OF_UP	 4
#define CONTROLLER_BIT_OF_DOWN	 5
#define CONTROLLER_BIT_OF_LEFT	 6
#define CONTROLLER_BIT_OF_RIGHT	 7

// most csak kontroller 1-et támogatja
typedef struct Controller {
	int a;
	int b;
	int select;
	int start;
	int up;
	int down;
	int left;
	int right;
	bool strobeBit;
	int nextBitToRead;
} Controller;

uint8_t ReadingFromControllerReg(Controller* controller);
void WritingToControllerReg(Controller* controller, uint8_t value); // csak CONTROLLER_REG_4016-re lehet írni
Controller CreateController();