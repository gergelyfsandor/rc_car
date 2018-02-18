
//Create virtual functions in order to simulate the real driver
#define SET_PWM1(a) //comment
#define SET_PWM2(a) //comment
#define GET_LIGHT_SENSOR() 300
#define RIGHT_FLASH(a) //comment
#define LEFT_FLASH(a) //comment
#define LOW_BEEM_ACTIVE(a) //comment

//Simulate the 4 input pins
#define PIN_ACCELERATE 	0x01
#define PIN_BREAK 		0x02
#define PIN_RIGHT		0x04
#define PIN_LEFT		0x08


//Define Threshold for the light sensor
#define LIGHT_THRESHOLD			500
#define LIGHT_THRESHOLD_HIST	10

typedef struct
{
	unsigned char SOD_Motor12_0;	//TODO - not used
	unsigned char SOD_Motor12_1;	//TODO - not used
	unsigned char SOD_Motor12_2;	//TODO - not used
	unsigned char SOD_Motor12_3;	//TODO - not used
	unsigned char SODPWM_EnableMotor1;
	unsigned char SODPWM_EnableMotor2;
	unsigned char SOD_LeftFlasher;
	unsigned char SOD_RightFlasher;
	unsigned char SOD_LowBeam;
} OutputBuffer;

typedef struct
{
	unsigned int SIA_LightSensor;
	unsigned char SID_WifiControlUp;
	unsigned char SID_WifiControlDown;
	unsigned char SID_WifiControlRight;
	unsigned char SID_WifiControlLeft;
} InputBuffer;

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
