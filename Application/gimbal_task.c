#include "gimbal_task.h"
#include "remote_control.h"
#include "includes.h"
#include "math.h"

Gimbal_t Gimbal = {0};
Gimbal_Date_t Gimbal_Date = {0};
void Gimbal_Init(void)
{
	Gimbal.YawMotor.zero_offset = YAW_MOTOR_ZERO_OFFSET;
	Gimbal.PitchMotor.zero_offset = PITCH_MOTOR_ZERO_OFFSET;

	Gimbal.YawMotor.Direction = NEGATIVE;
}

void Callback_Gimbal_Handle(Gimbal_Date_t *Gimbal_Date, uint8_t *buff)
{
	uint8_t buff_temp[3];
	if (Gimbal_Date == NULL || buff == NULL)
	{
		return;
	}
	Gimbal_Date->Gimbal_mode = buff[0] & 0x01;
	Gimbal_Date->MiniPC_state = buff[0] >> 1 & 0x01;
	Gimbal_Date->isFricOn = buff[0] >> 2 & 0x01;
	Gimbal_Date->Real_Fric = buff[0] >> 3 & 0x01;
	Gimbal_Date->Aimassist_mode = buff[0] >> 4 & 0x03;
	Gimbal_Date->Is_ShootEvaluation_On = buff[0] >> 6 & 0x01;
	Gimbal_Date->Is_LidOpen = buff[0] >> 7 & 0x01;
	Gimbal_Date->Aimassist_target = buff[1];
	Gimbal_Date->Aimassist_debug_show = (buff[2] << 8) | buff[3];
}