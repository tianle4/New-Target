#ifndef POWER_CONTROL_H
#define POWER_CONTROL_H
#include "includes.h"

#define MAX_POWER_BUFF 60.0f
#define SAFE_POWER_BUFF 20.0f
#define WARNING_POWER_BUFF 10.0f

#define CAP_CURRENT 10.0f
#define CAP_MAX_VOLTAGE 24.0f

#define MOTOR_POWER_K1 0.000091f
#define MOTOR_POWER_K2 454.29f
#define MOTOR_POWER_OFFSET 0.7f

#if ROBOT_ID == 0
#define POWER_LIMIT_SCALE_CAP 0.18f
#define POWER_LIMIT_SCALE 0.15f
#define POWER_FLY_OFFSET 0.11f
#elif ROBOT_ID == 1
#define POWER_LIMIT_SCALE_CAP 0.18f
#define POWER_LIMIT_SCALE 0.15f
#define POWER_FLY_OFFSET 0.11f
#elif ROBOT_ID == 3
#define POWER_LIMIT_SCALE_CAP 0.18f
#define POWER_LIMIT_SCALE 0.15f
#define POWER_FLY_OFFSET 0.11f
#endif
#define DOUBLE_CAP


/*Struct of Power_Control*/
typedef struct
{
  float Power_Scale;
  float Second_Power_Scale;
  float Power_Limit;
  float Power_Ref;
  float Power_Calculation;
  uint8_t Is_Cap_On;
	uint8_t Is_Cap_Used;
} Chassis_PowerControl_t;

extern Chassis_PowerControl_t Power_Control;
extern PID_t Power_Control_PID;

void Chassis_Power_Cal(void);//Calculate Chassis Power
void Chassis_Power_Control(void);
void Chassis_Power_Control_Fly(void);
void Chassis_Power_Control_Without_Cap(void);
void Chassis_Power_Control_New(void);
#endif