#ifndef CHASSIS_TASK_H
#define CHASSIS_TASK_H

#include "includes.h"

/*System constant*/
#define CHASSIS_TASK_PERIOD 2//Period of the chasssis_task in system

/*Mechanical*/
#define WHEEL_RADIUS 76.0f
#define YAW_REDUCTION_RATIO 1.0f //Yaw_Motor's reduction ratio: 1/1
#define YAW_REDUCTION_CORRECTION_ANGLE 90.0f

#if ROBOT_ID == 0
#define GRAVITYCENTER_ADJUSTMENT 0.9025f
#elif ROBOT_ID == 1

#define GRAVITYCENTER_ADJUSTMENT 0.97f

#elif ROBOT_ID == 3
#define GRAVITYCENTER_ADJUSTMENT 0.9025f
#endif

/*Control*/
#define Kx 0.25f
#define Ky 0.25f
#define VELOCITY_RATIO 5//Spinning mode's velocity ratio
#define MAX_RPM 8000

/*Math calculation*/
#define user_cos arm_cos_f32
#define user_sin arm_sin_f32

/*Spinning*/
#define SPINNING_SPEED 300
#define SPINNING_OMEGA 8
#define SPINNING_B 450
#define CAP_SPINNING_OMEGA 10
#define CAP_SPINNING_B 500

/*Two heads or Four heads*/
#define NUM_OF_HEAD 2//2 or 4

/*Struct of Chassis*/
typedef struct _chassis_t
{
  uint32_t Chassis_DWT_Count;
  int16_t Vx, Vy, Vr;
  float VX_k;
  float VY_k;
  float VR_k;

  float VxTransfer, VyTransfer;

  float VelocityRatio;  /*调整小陀螺在总功率中的占比*/
  float rcStickRotateRatio; /*摇杆运动与电机间的比例系数*/
  float rcMouseRotateRatio; /*鼠标运动与电机间的比例系数*/

  float V1, V2, V3, V4;

  float Vx_is_Chassis, Vy_is_Chassis; /*底盘坐标系下反解出的车速度*/
  float Vx_is_Body, Vy_is_Body; /*云台坐标系下反解出的车速度*/
  float Ax_Chassis, Ay_Chassis; /*底盘坐标系下在X,Y方向上的加速度*/
  float Ax_Body, Ay_Body; /*云台坐标系下在X,Y方向上的加速度*/
  float V1_is, V2_is, V3_is, V4_is; /*反解出四个轮子的速度*/

  int YawCorrectionScale;

  int8_t HeadingFlag; /*0正面 1左 -1右  2背面*/ //步兵车头方向的转换

  float GravityCenter_Adjustment;

  float Heading;
  float Theta;
  float TotalTheta;
  float FollowTheta;
  float DeflectionAngle;

  float cap_energy;
  float energy_percentage;

  float Attitude_adjustment[4];

	int8_t Spinning_direction;
  uint8_t Is_Attitude_Control_On;//差速标志位
  uint8_t Fly_Mode;//飞坡模式

  uint8_t Mode;

  Motor_t ChassisMotor[4];
  PID_t RotateFollow;
  PID_t ChassisVr;

  TD_t ChassisVxTD;
  TD_t ChassisVyTD;
} Chassis_t;
extern Chassis_t Chassis;

/*Mode enum*/
enum
{
  Follow_Mode = 0,  //跟随
  Spinning_Mode,  //小陀螺
  Silence_Mode, //静止模式
  Sin_Mode,
  Add_Mode,
  Recude_Mode
};

/*Declaration of Chassis functions*/
void Chassis_Control(void);
void Chassis_Init(void);
void Chassis_Get_Theta(void);//Get the angle of Chassis and Gimbal
void Chassis_Set_Mode(void);//Set Chassis's mode
void Chassis_Get_CtrlValue(void);//Handle the data from keyboard and remote control
void Chassis_Set_Control(void);//Chassis calcutation
void Send_Chassis_Current(void);//Send Chassis current to Motor
void Find_heading_4heads(void);//Find heading
void Find_heading_2heads_H(void);
void Find_heading_2heads_L(void);
void Velocity_MAXLimit(void);//Max velocity limit
float Max_4(float num1, float num2, float num3, float num4);
#endif