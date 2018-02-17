
#include "rc_car.h"
#include "cmsis_os.h"

//Simulate the 4 input pins
unsigned int PORTX = 0;
#define PIN_ACCELERATE 	0x01
#define PIN_BREAK 		0x02
#define PIN_RIGHT		0x04
#define PIN_LEFT		0x08

//Simulate the ADC register
unsigned int ADC_IN = 300;

OutputBuffer OutputBuffers;

extern osMessageQId IO_CommandHandle;
extern osMessageQId Flash_CommandHandle;

/**
 *
 */
void Input_Task(void)
{
	enum IO_Command inputCommand = Input_No_Command;

	//check the input pins
	//NOTE: I suppose that only 1 pin can be active at a time
	if (PORTX && PIN_ACCELERATE)
	{
		inputCommand = Input_Accelerate;
	} else if (PORTX && PIN_BREAK)
	{
		inputCommand = Input_Break;
	} else if (PORTX && PIN_RIGHT)
	{
		inputCommand = Input_Turn_Right;
	} else if (PORTX && PIN_LEFT)
	{
		inputCommand = Input_Turn_Left;
	} else
	{
		inputCommand = Input_No_Command;
	}

	//TODO add inputCommand to the queue
	xQueueCRSend(IO_CommandHandle,&inputCommand,100);
}

/**
 *
 */
void LSensor_Task(void)
{
	static unsigned char isLowBeamOn = 0;
	enum IO_Command inputCommand = Input_No_Command;

	//check if there is dark and the lights are off
	if ((ADC_IN < (LIGHT_THRESHOLD - LIGHT_THRESHOLD_HIST)) && (isLowBeamOn == 0))
	{
		isLowBeamOn = 1;
		inputCommand = Input_Low_Beam_on;
		//send command
		xQueueCRSend(IO_CommandHandle,&inputCommand,100);
	}
	//check if there is bright and lights are on
	if ((ADC_IN > (LIGHT_THRESHOLD + LIGHT_THRESHOLD_HIST)) && (isLowBeamOn == 1))
	{
		isLowBeamOn = 0;
		inputCommand = Input_Low_Beam_off;
		//send command
		xQueueCRSend(IO_CommandHandle,&inputCommand,100);
	}
}

/**
 *
 */
void Flasher_Task(void)
{
	enum Flash_Command flashCommand = Flash_Stop;
	static enum sm {state_inactive,
		state_fright_off,
		state_fright_on,
		state_fleft_on,
		state_fleft_off,
		state_flash_switch_off} StateMachine = state_inactive;

	//Check if there is any noew command to execute
	if (xQueueCRReceive(Flash_CommandHandle,&flashCommand,100))
	{
		switch (flashCommand)
		{
		case Flash_Stop:
			StateMachine = state_flash_switch_off;
			break;
		}
	}

	//Statemachine
	switch (StateMachine)
	{
	case state_inactive:
		break;
	case state_fright_off:
		//TODO init timer -
		break;
	case state_fright_on:
		//TODO init timer -
		break;
	case state_flash_switch_off:
		//TODO make sure flash is off
		break;
	case state_fleft_on:
		//TODO init timer -
		break;
	case state_fleft_off:
		//TODO init timer -
		break;
	default:
		break;
	}

}

/**
 *
 */
void Controller_Task(void)
{
	enum IO_Command inputCommand = Input_No_Command;
	enum Flash_Command flashCommand = Flash_Stop;
	unsigned char updateMotor = 0;
	unsigned char updateFlash = 0;

	//check if there is any message in the queue
	xQueueCRReceive(IO_CommandHandle,&inputCommand,100);

	switch (inputCommand)
	{
	case Input_Accelerate:
		if (OutputBuffers.MotorPWM1 < 100) {
			OutputBuffers.MotorPWM1++;
		}
		if (OutputBuffers.MotorPWM2 < 100) {
			OutputBuffers.MotorPWM2++;
		}
		flashCommand = Flash_Stop;
		updateMotor = 1;
		updateFlash = 1;
		break;

	case Input_Break:
		if (OutputBuffers.MotorPWM1 > 1) {
			OutputBuffers.MotorPWM1 -= 2;
		} else {
			OutputBuffers.MotorPWM1 = 0;
		}
		if (OutputBuffers.MotorPWM2 > 1) {
			OutputBuffers.MotorPWM2 -= 2;
		} else {
			OutputBuffers.MotorPWM2 = 0;
		}
		flashCommand = Flash_Stop;
		updateMotor = 1;
		updateFlash = 1;
		break;

	case Input_Turn_Right:
		flashCommand = Flash_Right;
		updateFlash = 1;
		updateMotor = 1;
		break;

	case Input_Turn_Left:
		flashCommand = Flash_Left;
		updateFlash = 1;
		updateMotor = 1;
		break;

	case Input_Low_Beam_off:
		//TODO turn the low beam off
		break;

	case Input_Low_Beam_on:
		//TODO turn the low beam on
		break;

	case Input_No_Command:
		if (OutputBuffers.MotorPWM1 >= 1) {
			OutputBuffers.MotorPWM1--;
		}
		if (OutputBuffers.MotorPWM2 >= 1) {
			OutputBuffers.MotorPWM2--;
		}
		updateMotor = 1;
		break;

	default:
		break;
	}

	if (updateMotor)
	{
		//TODO update PWM values for motor control
	}

	//send the new command to the task
	if (updateFlash)
	{
		xQueueCRSend(Flash_CommandHandle,&flashCommand,100);
	}
}
