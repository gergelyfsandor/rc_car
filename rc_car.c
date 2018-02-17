
#include "rc_car.h"
#include "cmsis_os.h"

OutputBuffer OutputBuffers;
InputBuffer InputBuffers;

extern osMessageQId IO_CommandHandle;
extern osMessageQId Flash_CommandHandle;

/**
 * void Input_Task(void)
 * It gets called in every 100ms and checks the input pins
 * In case any of the input pins is active it sends a command to
 * Controller. It doesn't need to store the previous value, because the
 * controller is updating the PWM values every time it gets called
 */
void Input_Task(void)
{
	enum IO_Command inputCommand = Input_No_Command;

	//check the input pins
	InputBuffers.SID_WifiControlUp = PORTX && PIN_ACCELERATE;
	InputBuffers.SID_WifiControlDown = PORTX && PIN_BREAK;
	InputBuffers.SID_WifiControlRight = PORTX && PIN_RIGHT;
	InputBuffers.SID_WifiControlLeft = PORTX && PIN_LEFT;


	//NOTE: I suppose that only 1 pin can be active at a time
	if (InputBuffers.SID_WifiControlUp)
	{
		inputCommand = Input_Accelerate;
	} else if (InputBuffers.SID_WifiControlDown)
	{
		inputCommand = Input_Break;
	} else if (InputBuffers.SID_WifiControlRight)
	{
		inputCommand = Input_Turn_Right;
	} else if (InputBuffers.SID_WifiControlLeft)
	{
		inputCommand = Input_Turn_Left;
	} else
	{
		inputCommand = Input_No_Command;
	}

	//Command to the queue in order to get the Controller notified
	xQueueCRSend(IO_CommandHandle,&inputCommand,100);
}

/**
 * void LSensor_Task(void)
 * It checks the light sensor and if the input is above / below the
 * threshold then it notifies the Controller using the IO_CommandHandle queue
 */
void LSensor_Task(void)
{
	static unsigned char isLowBeamOn = 0;
	enum IO_Command inputCommand = Input_No_Command;

	//Read the value of the sensor
	InputBuffers.SIA_LightSensor = GET_LIGHT_SENSOR();

	//check if there is dark and the lights are off
	if ((InputBuffers.SIA_LightSensor < (LIGHT_THRESHOLD - LIGHT_THRESHOLD_HIST)) && (isLowBeamOn == 0))
	{
		isLowBeamOn = 1;
		inputCommand = Input_Low_Beam_on;
		//send command
		xQueueCRSend(IO_CommandHandle,&inputCommand,100);
	}
	//check if there is bright and lights are on
	if ((InputBuffers.SIA_LightSensor > (LIGHT_THRESHOLD + LIGHT_THRESHOLD_HIST)) && (isLowBeamOn == 1))
	{
		isLowBeamOn = 0;
		inputCommand = Input_Low_Beam_off;
		//send command
		xQueueCRSend(IO_CommandHandle,&inputCommand,100);
	}
}

/**
 * void Flasher_Task(void)
 * - This function handles the commands to turn on / turn off flashes
 * - It also has a state machine for the correct on/off timings
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
	if (xQueueCRReceive(Flash_CommandHandle,&flashCommand,0))
	{
		switch (flashCommand)
		{
		case Flash_Stop:
			StateMachine = state_flash_switch_off;
			break;
		case Flash_Left:
			StateMachine = state_fleft_on;
			break;
		case Flash_Right:
			StateMachine = state_fright_on;
			break;
		default:
			break;
		}
	}

	//Statemachine - used for handling the timers
	switch (StateMachine)
	{
	case state_inactive:
		break;
	case state_fright_off:
		StateMachine = state_fright_on;
		RIGHT_FLASH(1);
		osDelay(420);
		break;
	case state_fright_on:
		StateMachine = state_fright_off;
		RIGHT_FLASH(1);
		osDelay(320);
		break;
	case state_flash_switch_off:
		RIGHT_FLASH(0);
		LEFT_FLASH(0);
		StateMachine = state_inactive;
		break;
	case state_fleft_on:
		StateMachine = state_fleft_off;
		LEFT_FLASH(1);
		osDelay(320);
		break;
	case state_fleft_off:
		StateMachine = state_fleft_on;
		LEFT_FLASH(1);
		osDelay(420);
		break;
	default:
		break;
	}
}

/**
 * void Controller_Task(void)
 * This task gets called if a command is available in IO_CommandHandle
 * queue. It updates the PWM values for the motors, turns on / off the low beam and
 * if required it calls the flash task
 */
void Controller_Task(void)
{
	enum IO_Command inputCommand = Input_No_Command;
	enum Flash_Command flashCommand = Flash_Stop;
	unsigned char updateMotor = 0;
	unsigned char updateFlash = 0;

	//check if there is any message in the queue
	if (xQueueCRReceive(IO_CommandHandle,&inputCommand,100))
	{
		//process command
		switch (inputCommand)
		{
		case Input_Accelerate:
			if (OutputBuffers.SODPWM_EnableMotor1 < 100) {
				OutputBuffers.SODPWM_EnableMotor1++;
			}
			if (OutputBuffers.SODPWM_EnableMotor2 < 100) {
				OutputBuffers.SODPWM_EnableMotor2++;
			}
			flashCommand = Flash_Stop; //NOTE: I guess it stops steering
			updateMotor = 1;
			updateFlash = 1;
			break;

		case Input_Break:
			if (OutputBuffers.SODPWM_EnableMotor1 > 1) {
				OutputBuffers.SODPWM_EnableMotor1 -= 2;
			} else {
				OutputBuffers.SODPWM_EnableMotor1 = 0;
			}
			if (OutputBuffers.SODPWM_EnableMotor2 > 1) {
				OutputBuffers.SODPWM_EnableMotor2 -= 2;
			} else {
				OutputBuffers.SODPWM_EnableMotor2 = 0;
			}
			flashCommand = Flash_Stop; //NOTE: I guess it stops steering
			updateMotor = 1;
			updateFlash = 1;
			break;

		case Input_Turn_Right:
			//TODO turn right
			flashCommand = Flash_Right;
			updateFlash = 1;
			updateMotor = 1;
			break;

		case Input_Turn_Left:
			//TODO turn left
			flashCommand = Flash_Left;
			updateFlash = 1;
			updateMotor = 1;
			break;

		case Input_Low_Beam_off:
			OutputBuffers.SOD_LowBeam = 0;
			LOW_BEEM_ACTIVE(0);
			break;

		case Input_Low_Beam_on:
			OutputBuffers.SOD_LowBeam = 1;
			LOW_BEEM_ACTIVE(1);
			break;

		case Input_No_Command:
			if (OutputBuffers.SODPWM_EnableMotor1 >= 1) {
				OutputBuffers.SODPWM_EnableMotor1--;
			}
			if (OutputBuffers.SODPWM_EnableMotor2 >= 1) {
				OutputBuffers.SODPWM_EnableMotor2--;
			}
			updateMotor = 1;
			break;

		default:
			break;
		}
	}

	//update PWM values
	if (updateMotor)
	{
		SET_PWM1(OutputBuffers.SODPWM_EnableMotor1);
		SET_PWM2(OutputBuffers.MotorPWM2);
	}

	//send the new command to the Flash task
	if (updateFlash)
	{
		xQueueCRSend(Flash_CommandHandle,&flashCommand,1000);
	}
}
