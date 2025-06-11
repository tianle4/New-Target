#include "chassis_task.h"

Chassis_t Chassis = {0};

float dt = 0;
float t = 0;

uint8_t flag = 0;
int16_t ConstantValue = 300;

void Chassis_Control(void)
{
    dt = DWT_GetDeltaT(&Chassis.Chassis_DWT_Count);
    t += dt;
    //Get the angle of Chassis and Gimbal
    Chassis_Get_Theta();
    //Set Chassis's mode
    Chassis_Set_Mode();
    //Handle the data from keyboard and remote control
    Chassis_Get_CtrlValue();
    //Chassis calcutation
    Chassis_Set_Control();
    //Send Chassis current to Motor
    Send_Chassis_Current();
}

void Chassis_Init(void)
{   
    /*Chassis struct init*/
    Chassis.VX_k = 2000;
    Chassis.VY_k = 2000;
    Chassis.VR_k = 500;
    Chassis.VelocityRatio = VELOCITY_RATIO;
    Chassis.rcStickRotateRatio = 3.0f;
    Chassis.rcMouseRotateRatio = 0;
    Chassis.YawCorrectionScale = 0;
    Chassis.HeadingFlag = 0;
    Chassis.Heading = 0;
    Chassis.GravityCenter_Adjustment = 1;
    Chassis.Is_Attitude_Control_On = FALSE;
    Chassis.Mode = Silence_Mode;
    for(uint8_t i=0;i<4;i++)
        Chassis.Attitude_adjustment[i] = 1.0f;

    /*TD Init*/
    TD_Init(&Chassis.ChassisVxTD, 1000, 0.01f);
    TD_Init(&Chassis.ChassisVyTD, 1000, 0.01f);
    
    /*Findheading*/
    for(uint8_t i=0;i<4;i++)
    {
        Chassis.TotalTheta = (Gimbal.YawMotor.Total_angle - Gimbal.YawMotor.zero_offset)*ENCODERCOEF*YAW_REDUCTION_RATIO + i*YAW_REDUCTION_CORRECTION_ANGLE;
        Chassis.Theta = loop_float_constrain((Chassis.TotalTheta + Gimbal.YawMotor.Angle_offset*ENCODERCOEF*YAW_REDUCTION_RATIO),-180,180);
    }

    /*PID Init*/
    //PID_Velocity
    for(uint8_t i=0;i<4;i++)
    {
        PID_Init(&Chassis.ChassisMotor[i].PID_Velocity, 16384, 16384, 0, 15, 30, 0, 500, 100, 0.005, 0, 1, Integral_Limit | OutputFilter);
        Chassis.ChassisMotor[i].Max_Out = 16384;
    }
    //PID_Follow
    PID_Init(&Chassis.RotateFollow, 300, 100, 0, 6, 0, 0, 0,0, 0, 0, 5,Integral_Limit | Derivative_On_Measurement | OutputFilter | DerivativeFilter);
    //PID_Vr
    PID_Init(&Chassis.ChassisVr, 500, 100, 0, 5, 0, 0, 300,100, 0, 0.005, 5,Integral_Limit | Derivative_On_Measurement | Trapezoid_Intergral | DerivativeFilter);

    /*Power control init*/
		Chassis.Spinning_direction = 1;
    Power_Control.Is_Cap_On = FALSE;
		Power_Control.Is_Cap_Used = TRUE;
}

void Chassis_Get_Theta(void)
{
    /*Get the angle of Chassis and Gimbal*/
    Chassis.TotalTheta = (Gimbal.YawMotor.Total_angle - Gimbal.YawMotor.zero_offset)*ENCODERCOEF + Chassis.YawCorrectionScale * YAW_REDUCTION_CORRECTION_ANGLE;
    Chassis.Theta = loop_float_constrain((Chassis.TotalTheta + Gimbal.YawMotor.Angle_offset * ENCODERCOEF),-180, 180);

    /*Finding head*/
    if(Chassis.Fly_Mode == TRUE)
    {
        Chassis.HeadingFlag = 2;//When in fly mode, only use the head with guide whells.
        Chassis.Theta = loop_float_constrain((Chassis.Theta + Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE),-180, 180);
    }
	else
		Find_heading_2heads_H();

    //When Gimbal is abnormal,reset the angle 
    if (is_TOE_Error(GIMBAL_YAW_MOTOR_TOE))
    {
        Chassis.TotalTheta = 0;
        Chassis.Theta = 0;
    }

    Chassis.FollowTheta = float_deadband(Chassis.Theta, -0.005f, 0.005f);
}

void Chassis_Set_Mode(void)
{
    static uint16_t LastKeyCode = 0;
	static uint16_t LastRightSwitch = 0;
    static uint16_t LastLeftSwich = 0;

    /*Set mode by key_code*/
    //Keep EF, Silence_Mode
    if((remote_control.key_code & Key_E) && (remote_control.key_code & Key_F))
        Chassis.Mode = Silence_Mode;

    //F, switch Spinning_Mode and Follow_Mode
    if ((remote_control.key_code & Key_F) && !(LastKeyCode & Key_F))
    {
        if (Chassis.Mode != Spinning_Mode)
            Chassis.Mode = Spinning_Mode;
        else
            Chassis.Mode = Follow_Mode;
				
				Chassis.Spinning_direction *= -1;
    }

    //Z, switch Silence_Mode and Follow_Mode
    if ((remote_control.key_code & Key_Z) && !(LastKeyCode & Key_Z))
    {
        if (Chassis.Mode != Silence_Mode)
            Chassis.Mode = Silence_Mode;
        else
            Chassis.Mode = Follow_Mode;
    }

    //If motor lost, Silence_Mode
    if((is_TOE_Error(CHASSIS_MOTOR1_TOE)&&is_TOE_Error(CHASSIS_MOTOR2_TOE)&&is_TOE_Error(CHASSIS_MOTOR3_TOE)&&is_TOE_Error(CHASSIS_MOTOR4_TOE)))
        Chassis.Mode = Silence_Mode;

    /*Set mode by switch*/	
		if(remote_control.switch_right == Switch_Middle)
			Chassis.Mode = Follow_Mode;

        if(remote_control.switch_right == Switch_Up)
            Chassis.Mode = Sin_Mode;
		
		if(remote_control.switch_right == Switch_Down && LastRightSwitch != Switch_Down)
		{
			// if(Chassis.Mode != Silence_Mode)
			// 	Chassis.Mode = Silence_Mode;
			// else
			// {
				Chassis.Mode = Spinning_Mode;
				Chassis.Spinning_direction *= -1;
			// }
		}

        if (remote_control.switch_left == Switch_Up && LastLeftSwich != Switch_Up)
            flag = (flag + 1) % 4;
        else if (remote_control.switch_right == Switch_Down && LastLeftSwich != Switch_Down)
            flag = (flag - 1 + 5) % 5;

        switch (flag)
        {
            case 0:
                ConstantValue = 100;
                break;
            case 1:
                ConstantValue = 300;
                break;
            case 2:
                ConstantValue = 500;
                break;
            case 3:
                ConstantValue = 700;
                break;
        }

    /*Cap Switch*/
    if((remote_control.switch_left==Switch_Up||remote_control.key_code&Key_SHIFT||remote_control.key_code&Key_CTRL) && (Cap.Voltage>15.0f))
		{
        Power_Control.Is_Cap_On = TRUE;
			//Chassis.Fly_Mode = TRUE;
		}
		else
		{
			Power_Control.Is_Cap_On = FALSE;
			//Chassis.Fly_Mode = FALSE;
		}

    /*Fly_Control*/
    if(remote_control.key_code & Key_CTRL)
    {
        Chassis.Is_Attitude_Control_On = TRUE;
        Chassis.Fly_Mode = TRUE;
    }
    else
    {
        Chassis.Is_Attitude_Control_On = FALSE;
        Chassis.Fly_Mode = FALSE;
    }

    if(remote_control.key_code & Key_V && LastKeyCode != (remote_control.key_code & Key_V))
        Power_Control.Is_Cap_Used = !(Power_Control.Is_Cap_Used);

    /*Refresh*/
    LastKeyCode = remote_control.key_code;
		LastRightSwitch = remote_control.switch_right;
}

void Chassis_Get_CtrlValue(void)
{
    static float Press_Timestamp_W,Press_Timestamp_S,Press_Timestamp_A,Press_Timestamp_D;
    static uint16_t LastKeyCode = 0;

    static float Temp_Vx, Temp_Vy;//Velocity_Ref
    static float tempVal;
    static float RC = 0.000001f;
	
		//If Judge_lost
		if(is_TOE_Error(JUDGE_TOE))
		{
			robot_state.chassis_power_limit = 45.0f;
			robot_state.shooter_barrel_cooling_value = 10.0f;
			robot_state.shooter_barrel_heat_limit = 50.0f;
			
			power_heat_data.buffer_energy = 30.0f;
		}

    //When the power_limit is different from real data
    if (robot_state.chassis_power_limit >= 10240)//max of data_packet
        robot_state.chassis_power_limit /= 256;
    //Power max limit
    if (robot_state.chassis_power_limit > 120)//min of data_packet
        robot_state.chassis_power_limit = 120;
    //Power min limit
    if (robot_state.chassis_power_limit == 0)
        robot_state.chassis_power_limit = 45;

    Chassis.VelocityRatio = float_constrain(robot_state.chassis_power_limit / 10.0f + 2, 5, 10); //adjust the ratio

    /*Key_code WSAD*/
    if (remote_control.key_code & Key_A || remote_control.key_code & Key_D)
    {
        if(!(LastKeyCode & Key_A) && (remote_control.key_code & Key_A))
            Press_Timestamp_A = USER_GetTick();
        if(remote_control.key_code & Key_A)
        {
            if(USER_GetTick() - Press_Timestamp_A > 750)
                Temp_Vx = -660.0f;
            else
                Temp_Vx = -220.0f;
        }

        if(!(LastKeyCode & Key_D) && (remote_control.key_code & Key_D))
            Press_Timestamp_D = USER_GetTick();
        if(remote_control.key_code & Key_D)
        {
            if(USER_GetTick() - Press_Timestamp_D > 750)
                Temp_Vx = 660.0f;
            else
                Temp_Vx = 220.0f;
        }
    }
    else
        Temp_Vx = 0;

    if (remote_control.key_code & Key_W || remote_control.key_code & Key_S) 
    {
        if (!(LastKeyCode & Key_W) && (remote_control.key_code & Key_W))
            Press_Timestamp_W = USER_GetTick();
        if(remote_control.key_code & Key_W)
        {
            if (USER_GetTick() - Press_Timestamp_W > 750)
                Temp_Vy = 660.0f;
            else
                Temp_Vy = 220.0f;
        }

        if (!(LastKeyCode & Key_S) && (remote_control.key_code & Key_S))
            Press_Timestamp_S = USER_GetTick();
        if(remote_control.key_code & Key_S)
        {
        if (USER_GetTick() - Press_Timestamp_S > 750)
            Temp_Vy = -660.0f;
        else
            Temp_Vy = -220.0f;
        }
    }
    else
        Temp_Vy = 0;

    LastKeyCode = remote_control.key_code;//Refresh LastKeyCode

    // Get remote_control's data
    Temp_Vx += remote_control.ch3;
    Temp_Vy += remote_control.ch4;

    //Cap_mode Velocity_Ref*2
    if (Power_Control.Is_Cap_On == TRUE)
    {
#if ROBOT_ID == 0
	Temp_Vx *= 2.0f;
	Temp_Vy *= 2.0f;
#elif ROBOT_ID == 1
    Temp_Vx *= 2.0f;
    Temp_Vy *= 2.0f;
#elif ROBOT_ID == 3
	Temp_Vx *= 2.0f;
	Temp_Vy *= 2.0f;
#endif
    }
    //Slow_mode Velocity_Ref/5
    if ((remote_control.key_code & Key_C))
    {
        Chassis.Mode = Follow_Mode;
        Temp_Vx *= 0.2f;
        Temp_Vy *= 0.2f;
    }

    /*Slow Start*/
    if (fabsf(Temp_Vx) > 1e-3f)
    {
        if (Chassis.Vx * Temp_Vx < 0)
        {
            Chassis.Vx = 0;
        }
        tempVal = (Temp_Vx - Chassis.Vx) / (RC + dt);
        if (tempVal > Chassis.VX_k)
            tempVal = Chassis.VX_k;
        else if (tempVal < -Chassis.VX_k)
            tempVal = -Chassis.VX_k;
        Chassis.Vx += tempVal * dt;
    }
    else
    {
        tempVal = (Temp_Vx - Chassis.Vx) / (0.01f + dt);
        Chassis.Vx += tempVal * dt;
    }

    if (fabsf(Temp_Vy) > 1e-3f)
    {
        if (Chassis.Vy * Temp_Vy < 0)
            Chassis.Vy = 0;
        tempVal = (Temp_Vy - Chassis.Vy) / (RC + dt);
        if (tempVal > Chassis.VY_k)
            tempVal = Chassis.VY_k;
        else if (tempVal < -Chassis.VY_k)
            tempVal = -Chassis.VY_k;
        Chassis.Vy += tempVal * dt;
    }
    else
    {
        tempVal = (Temp_Vy - Chassis.Vy) / (0.01f + dt);
        Chassis.Vy += tempVal * dt;
    }
}

void Chassis_Set_Control(void)
{
    Chassis.Heading = 0.0f;

    switch(Chassis.Mode)
    {
        case Follow_Mode:
        FOLLOW:
            PID_Init(&Chassis.RotateFollow, 300, 100, 0, 12, 0, 0.15, 0,0, 0, 0, 5,Integral_Limit | Derivative_On_Measurement | OutputFilter | DerivativeFilter);
            Chassis.GravityCenter_Adjustment = GRAVITYCENTER_ADJUSTMENT;
            if(Chassis.Fly_Mode == TRUE)
            {
                    PID_Init(&Chassis.RotateFollow, 300, 100, 0, 25, 0, 0.055, 0,0, 0, 0, 5,Integral_Limit | Derivative_On_Measurement | OutputFilter | DerivativeFilter);
                    Chassis.GravityCenter_Adjustment = GRAVITYCENTER_ADJUSTMENT;
            }

            if (fabsf(Chassis.FollowTheta) < 0.005f)//If the Gimbal is directly face the Chassis
                Chassis.Vr = remote_control.ch1 * Chassis.rcStickRotateRatio + remote_control.mouse.x * Chassis.rcMouseRotateRatio;
            else
            {
                if (Power_Control.Is_Cap_On == TRUE)//Cap_Mode
                    Chassis.Vr = -PID_Calculate(&Chassis.RotateFollow, Chassis.FollowTheta, Chassis.Heading);// No Feedforward
                else
                    Chassis.Vr = -PID_Calculate(&Chassis.RotateFollow, Chassis.FollowTheta, Chassis.Heading) + remote_control.ch1 * Chassis.rcStickRotateRatio + remote_control.mouse.x * Chassis.rcMouseRotateRatio;
            }
        //Transform Gimbal's angle to Chassis
        Chassis.VxTransfer = user_cos((Chassis.FollowTheta - Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE) / RADIAN_COEF) * Chassis.Vx + user_sin((Chassis.FollowTheta - Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE) / RADIAN_COEF) * Chassis.Vy;
        Chassis.VyTransfer = -user_sin((Chassis.FollowTheta - Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE) / RADIAN_COEF) * Chassis.Vx + user_cos((Chassis.FollowTheta - Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE) / RADIAN_COEF) * Chassis.Vy;
        break;

        case Silence_Mode:
            Chassis.Vr = 0;
            Chassis.VxTransfer = 0;
            Chassis.VyTransfer = 0;

        //Press W|S|A|D in Silence_Mode, set Follow_Mode
        if ((remote_control.key_code & Key_W) || (remote_control.key_code & Key_A) || (remote_control.key_code & Key_S) || (remote_control.key_code & Key_D))
            Chassis.Mode = Follow_Mode;
        break;

        case Spinning_Mode:
            srand(t);
            static float rand_amp;
            rand_amp = rand()%25 + 100;
		    if(Power_Control.Is_Cap_On==TRUE) 
			    Chassis.Vr = (int16_t)(CAP_SPINNING_B+rand_amp*user_sin(CAP_SPINNING_OMEGA*t));
		    else
           Chassis.Vr = (int16_t)(SPINNING_B+rand_amp*user_sin(SPINNING_OMEGA*t));

            //Press W|S|A|D in Spinning Mode, set constant spinning speed
            if ((remote_control.key_code & Key_W) || (remote_control.key_code & Key_A) || (remote_control.key_code & Key_S) || (remote_control.key_code & Key_D) || 
							(remote_control.ch1 != 0) || (remote_control.ch2 != 0) || (remote_control.ch3 != 0) || (remote_control.ch4 != 0))
                Chassis.Vr = SPINNING_SPEED;
						
						Chassis.Vr *= Chassis.Spinning_direction;
            Chassis.DeflectionAngle = (Chassis.FollowTheta - Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE) / RADIAN_COEF;
            Chassis.VxTransfer = user_cos(Chassis.DeflectionAngle) * Chassis.Vx +
                            user_sin(Chassis.DeflectionAngle) * Chassis.Vy;
            Chassis.VyTransfer = -user_sin(Chassis.DeflectionAngle) * Chassis.Vx +
                            user_cos(Chassis.DeflectionAngle) * Chassis.Vy;

            Chassis.VxTransfer *= 0.5f;
            Chassis.VyTransfer *= 0.5f;
						
						Chassis.GravityCenter_Adjustment = 1.0f;
        break;
            
    }

    if (is_TOE_Error(GIMBAL_YAW_MOTOR_TOE)) //If Gimbal Yaw lost, Control Chassis
        Chassis.Vr = remote_control.ch1 * Chassis.rcStickRotateRatio + remote_control.mouse.x * Chassis.rcMouseRotateRatio;
    if (Chassis.Mode == Spinning_Mode)
    {
        Chassis.Vr = ConstantValue;
        Chassis.Vr *= Chassis.Spinning_direction;
    }
    if (Chassis.Mode == Sin_Mode)
    {
        Chassis.Vr = 300 * sin(3 * t);
    }
    

    if (Chassis.Is_Attitude_Control_On == TRUE)//Flying slope
    {
        Chassis.Attitude_adjustment[0] = 1.5;
        Chassis.Attitude_adjustment[1] = 1;
        Chassis.Attitude_adjustment[2] = 1;
        Chassis.Attitude_adjustment[3] = 1.5;
    }
    else
    {
        Chassis.Attitude_adjustment[0] = 1;
        Chassis.Attitude_adjustment[1] = 1;
        Chassis.Attitude_adjustment[2] = 1;
        Chassis.Attitude_adjustment[3] = 1;
    }

    /*Calculation*/
    Chassis.V1 = -(-Chassis.VxTransfer - Chassis.VyTransfer) * Chassis.VelocityRatio * Chassis.Attitude_adjustment[0] + Chassis.Vr * 10;
    Chassis.V2 = -(Chassis.VxTransfer - Chassis.VyTransfer) * Chassis.VelocityRatio * Chassis.Attitude_adjustment[1] + Chassis.Vr * 10;
    Chassis.V3 = -(Chassis.VxTransfer + Chassis.VyTransfer) * Chassis.VelocityRatio * Chassis.Attitude_adjustment[2] + Chassis.Vr * 10;
    Chassis.V4 = -(-Chassis.VxTransfer + Chassis.VyTransfer) * Chassis.VelocityRatio * Chassis.Attitude_adjustment[3] + Chassis.Vr * 10;

    //Adjust Gimbal's Gracity_Center
			Chassis.V3 *= Chassis.GravityCenter_Adjustment;
			Chassis.V4 *= Chassis.GravityCenter_Adjustment;

    Velocity_MAXLimit();

    //Motor speed calculation
    Motor_Speed_Calculate(&Chassis.ChassisMotor[0], Chassis.ChassisMotor[0].Velocity_RPM, Chassis.V1);
    Motor_Speed_Calculate(&Chassis.ChassisMotor[1], Chassis.ChassisMotor[1].Velocity_RPM, Chassis.V2);
    Motor_Speed_Calculate(&Chassis.ChassisMotor[2], Chassis.ChassisMotor[2].Velocity_RPM, Chassis.V3);
    Motor_Speed_Calculate(&Chassis.ChassisMotor[3], Chassis.ChassisMotor[3].Velocity_RPM, Chassis.V4);

    /*Power Control*/
	Chassis_Power_Cal();
	if(Power_Control.Is_Cap_Used == TRUE)
		{
        if(Chassis.Fly_Mode == TRUE)
            Chassis_Power_Control_Fly();
        else
            Chassis_Power_Control();
        }
	else if(Power_Control.Is_Cap_Used == FALSE)
		Chassis_Power_Control_Without_Cap();

    Cap_unused_correct();
}

void Send_Chassis_Current(void)
{
    /*Send Judgement's data to Cap*/
    static uint8_t count=0;
    if(count%10 == 0)
    {	
        Send_Power_Data(&hcan1, robot_state.chassis_power_limit, power_heat_data.buffer_energy);
        count = 0;
    }
    count ++;

    /*Send Chassis current to motors*/
    //If RC and VTM lost, stop the robot
    if (is_TOE_Error(RC_TOE) && is_TOE_Error(VTM_TOE))
    {
        if (Send_Motor_Current_1_4(&hcan1, 0, 0, 0, 0) == HAL_OK)
            HAL_IWDG_Refresh(&hiwdg);
    }
    else
    {
        if (Send_Motor_Current_1_4(&hcan1,Chassis.ChassisMotor[0].Output, Chassis.ChassisMotor[1].Output,Chassis.ChassisMotor[2].Output, Chassis.ChassisMotor[3].Output) == HAL_OK)
            HAL_IWDG_Refresh(&hiwdg);
    }
}

void Find_heading_4heads(void)
{
//Search the direction of Gimbal
    if (Chassis.Heading < 0.01f)
    {
        if ((Chassis.Mode != Silence_Mode) && ((Chassis.Theta > 45 + Chassis.Heading) && (Chassis.Theta < 135 + Chassis.Heading)))
        {
            Chassis.HeadingFlag = -1;

            Chassis.Theta = loop_float_constrain((Chassis.Theta + Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE),
                                                 -180, 180);
        }
        else if (((Chassis.Mode != Silence_Mode) && (fabsf(Chassis.Theta) > 135) && (Chassis.Heading == 0)) || ((Chassis.Mode != Silence_Mode) && (Chassis.Heading != 0) && (Chassis.Theta < -135 + Chassis.Heading)))
        {

            Chassis.HeadingFlag = 2;

            Chassis.Theta = loop_float_constrain((Chassis.Theta + Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE),
                                                 -180, 180);
        }
        else if ((Chassis.Mode != Silence_Mode) && (Chassis.Theta > -135 + Chassis.Heading) && (Chassis.Theta < -45 + Chassis.Heading))
        {

            Chassis.HeadingFlag = 1;

            Chassis.Theta = loop_float_constrain((Chassis.Theta + Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE),
                                                 -180, 180);
        }
        else if ((Chassis.Mode != Silence_Mode) && (Chassis.Theta < 45 + Chassis.Heading) && (Chassis.Theta > -45 + Chassis.Heading))
        {
            Chassis.HeadingFlag = 0;

            Chassis.Theta = loop_float_constrain((Chassis.Theta + Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE),
                                                 -180, 180);
        }
    }
    else
    {
        if ((Chassis.Mode != Silence_Mode) && ((Chassis.Theta > 45 + Chassis.Heading) && (Chassis.Theta < 135 + Chassis.Heading)))
        {
            Chassis.HeadingFlag = -1;

            Chassis.Theta = loop_float_constrain((Chassis.Theta + Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE),
                                                 -180, 180);
        }
        else if ((Chassis.Mode != Silence_Mode) && (Chassis.Theta < -135 + Chassis.Heading))
        {

            Chassis.HeadingFlag = 2;

            Chassis.Theta = loop_float_constrain((Chassis.Theta + Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE),
                                                 -180, 180);
        }
        else if ((Chassis.Mode != Silence_Mode) && (Chassis.Theta > -135 + Chassis.Heading) && (Chassis.Theta < -45 + Chassis.Heading))
        {

            Chassis.HeadingFlag = 1;

            Chassis.Theta = loop_float_constrain((Chassis.Theta + Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE),
                                                 -180, 180);
        }
        else if ((Chassis.Mode != Silence_Mode) && (Chassis.Theta < 45 + Chassis.Heading) && (Chassis.Theta > -45 + Chassis.Heading))
        {
            Chassis.HeadingFlag = 0;

            Chassis.Theta = loop_float_constrain((Chassis.Theta + Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE),
                                                 -180, 180);
        }
    }
}

void Find_heading_2heads_H(void)
{
    //Search the direction of Gimbal
    if (Chassis.Heading < 0.01f)
    {
        if ((Chassis.Mode != Silence_Mode) && (Chassis.Theta < 90 + Chassis.Heading) && (Chassis.Theta > -90 + Chassis.Heading))
        {
            Chassis.HeadingFlag = 0;

            Chassis.Theta = loop_float_constrain((Chassis.Theta + Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE),
                                                 -180, 180);
        }
        else if (((Chassis.Mode != Silence_Mode) && (fabsf(Chassis.Theta) > 90) && (Chassis.Heading == 0)) /*|| ((Chassis.Mode != Silence_Mode) && (Chassis.Heading != 0) && (Chassis.Theta < -90+ Chassis.Heading))*/)
        {

            Chassis.HeadingFlag = 2;

            Chassis.Theta = loop_float_constrain((Chassis.Theta + Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE),
                                                 -180, 180);
        }
    }
    else
    {
        if ((Chassis.Mode != Silence_Mode) && (Chassis.Theta < 90 + Chassis.Heading) && (Chassis.Theta > -90 + Chassis.Heading))
        {
            Chassis.HeadingFlag = 0;

            Chassis.Theta = loop_float_constrain((Chassis.Theta + Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE),
                                                 -180, 180);
        }

        else if ((Chassis.Mode != Silence_Mode) && (Chassis.Theta < -90 + Chassis.Heading))
        {

            Chassis.HeadingFlag = 2;

            Chassis.Theta = loop_float_constrain((Chassis.Theta + Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE),
                                                 -180, 180);
        }
    }
}

void Find_heading_2heads_L(void)
{
    //Search the direction of Gimbal
    if (Chassis.Heading < 0.01f)
    {
        if ((Chassis.Mode != Silence_Mode) && ((Chassis.Theta > 0 + Chassis.Heading) && (Chassis.Theta < 180 + Chassis.Heading)))
        {
            Chassis.HeadingFlag = -1;

            Chassis.Theta = loop_float_constrain((Chassis.Theta + Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE),
                                                 -180, 180);
        }
        else if ((Chassis.Mode != Silence_Mode) && (Chassis.Theta > -180 + Chassis.Heading) && (Chassis.Theta < 0 + Chassis.Heading))
        {

            Chassis.HeadingFlag = 1;

            Chassis.Theta = loop_float_constrain((Chassis.Theta + Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE),
                                                 -180, 180);
        }
    }
    else
    {
        if ((Chassis.Mode != Silence_Mode) && ((Chassis.Theta > 0 + Chassis.Heading) && (Chassis.Theta < 180 + Chassis.Heading)))
        {
            Chassis.HeadingFlag = -1;

            Chassis.Theta = loop_float_constrain((Chassis.Theta + Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE),
                                                 -180, 180);
        }
        else if ((Chassis.Mode != Silence_Mode) && (Chassis.Theta > -180 + Chassis.Heading) && (Chassis.Theta < 0 + Chassis.Heading))
        {

            Chassis.HeadingFlag = 1;

            Chassis.Theta = loop_float_constrain((Chassis.Theta + Chassis.HeadingFlag * YAW_REDUCTION_CORRECTION_ANGLE),
                                                 -180, 180);
        }
    }
}

float Max_4(float num1, float num2, float num3, float num4)
{
    float max_num;

    max_num = fabs(num1);
    if(fabs(num2) > max_num)
        max_num = fabs(num2);
    if(fabs(num3) > max_num)
        max_num = fabs(num3);
    if(fabs(num4) > max_num)
        max_num = fabs(num4);

    return max_num;
}


void Velocity_MAXLimit(void)
{
    static float temp_max;
    if(Max_4(fabsf(Chassis.V1), fabsf(Chassis.V2), fabsf(Chassis.V3), fabsf(Chassis.V4)) > MAX_RPM)
    {
        temp_max = Max_4(fabsf(Chassis.V1), fabsf(Chassis.V2), fabsf(Chassis.V3), fabsf(Chassis.V4));
        
        Chassis.V1 *= MAX_RPM / temp_max;
        Chassis.V2 *= MAX_RPM / temp_max;
        Chassis.V3 *= MAX_RPM / temp_max;
        Chassis.V4 *= MAX_RPM / temp_max;
    }
}