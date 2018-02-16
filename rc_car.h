
#define LIGHT_THRESHOLD			500
#define LIGHT_THRESHOLD_HIST	10

typedef struct
{
	unsigned char MotorPWM1;
	unsigned char MotorPWM2;
	unsigned char MotorEnabled1;
	unsigned char MotorEnabled2;
} OutputBuffer;

enum IO_Command
{
	Input_No_Command=0x00,
	Input_Accelerate,
	Input_Break,
	Input_Turn_Left,
	Input_Turn_Right,
	Input_Low_Beam_on = 0x10,
	Input_Low_Beam_off
};

enum Flash_Command
{
	Flash_Stop=0x00,
	Flash_Right,
	Flash_Left
};

void Input_Task(void);
void LSensor_Task(void);
void Flasher_Task(void);
void Controller_Task(void);
