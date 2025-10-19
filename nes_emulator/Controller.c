#include "Controller.h"

void CheckStrobe(Controller* controller)
{
	if (controller->strobeBit)
		controller->nextBitToRead = 0;
}

uint8_t ReadingFromControllerReg(Controller* controller)
{
	uint8_t value;
	switch (controller->nextBitToRead)
	{
		case CONTROLLER_BIT_OF_A: {
			value = controller->a;
			break;
		}
		case CONTROLLER_BIT_OF_B: {
			value = controller->b;
			break;
		}
		case CONTROLLER_BIT_OF_SELECT: {
			value = controller->select;
			break;
		}
		case CONTROLLER_BIT_OF_START: {
			value = controller->start;
			break;
		}
		case CONTROLLER_BIT_OF_UP: {
			value = controller->up;
			break;
		}
		case CONTROLLER_BIT_OF_DOWN: {
			value = controller->down;
			break;
		}
		case CONTROLLER_BIT_OF_LEFT: {
			value = controller->left;
			break;
		}
		case CONTROLLER_BIT_OF_RIGHT: {
			value = controller->right;
			break;
		}
		default: {
			value = 0;
			break;
		}
	}
	controller->nextBitToRead++;
	CheckStrobe(controller);
	return value;
}

void WritingToControllerReg(Controller* controller, uint8_t value)
{
	controller->strobeBit = value & 0b1;
	CheckStrobe(controller);
}

Controller CreateController()
{
	Controller controller = {0};
	return controller;
}