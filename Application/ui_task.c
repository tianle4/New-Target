#include "ui_task.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static void Show_UI_Init(void);
static void Show_UI(void);

static void Layer1(uint8_t operate);
static void Layer2(uint8_t operate);
static void Layer3(uint8_t operate);
static void Layer4(uint8_t operate);
static void Layer5(uint8_t operate);
static void Layer6(uint8_t operate);
static void Layer7(uint8_t operate);

graphic_data_struct_t graphic[8];
client_frame_t Char;
client_frame_t Char2;
client_frame_t Char3;
client_frame_t Char4;
client_frame_t Char5;
client_frame_t Char6;
client_char_t Char7;

static float dt = 0, t = 0;
float uiCostTime = 0, uiTimeStamp = 0;
uint32_t UI_DWT_Count = 0, ui_DWT_Cost = 0;

int8_t flg = 0;

uint16_t remain_HP;
uint8_t hunt_buff;
uint8_t client_send_buff[30];
uint8_t client_send_buff_0[30];
uint8_t client_send_warning_buff[30];
uint16_t x[3], y[3], end_x[2], end_y[2];
uint16_t width;
uint8_t color;
uint16_t rect_startX = 706;
uint16_t rect_startY = 820;
uint16_t rect_endX = 1214;
uint16_t rect_endY = 840;
uint16_t HP_startX = 710;
uint16_t HP_startY = 830;
uint16_t HP_endX = 890;
uint16_t HP_endY = 830;
uint16_t HIT_startX = 890;
uint16_t HIT_startY = 830;
uint16_t HIT_endX = 890 - 100;
uint16_t HIT_endY = 830;
uint8_t HP_Width = 12;
uint8_t HIT_Width = 8;

#if ROBOT_ID == 0
uint16_t rect_Aim_startX = 930;
uint16_t rect_Aim_startY = 460;
uint16_t rect_Aim_endX = 990;
uint16_t rect_Aim_endY = 490;
uint16_t rect_AimRange_startX = 620;
uint16_t rect_AimRange_startY = 790;
uint16_t rect_AimRange_endX = 1170;
uint16_t rect_AimRange_endY = 327;
#elif ROBOT_ID == 1
uint16_t rect_Aim_startX = 930;
uint16_t rect_Aim_startY = 460;
uint16_t rect_Aim_endX = 990;
uint16_t rect_Aim_endY = 490;
uint16_t rect_AimRange_startX = 620;
uint16_t rect_AimRange_startY = 790;
uint16_t rect_AimRange_endX = 1170;
uint16_t rect_AimRange_endY = 327;
#elif ROBOT_ID == 3
uint16_t rect_Aim_startX = 920;
uint16_t rect_Aim_startY = 495;
uint16_t rect_Aim_endX = 970;
uint16_t rect_Aim_endY = 525;
uint16_t rect_AimRange_startX = 620;
uint16_t rect_AimRange_startY = 790;
uint16_t rect_AimRange_endX = 1170;
uint16_t rect_AimRange_endY = 327;
#endif

uint16_t robot_state_startX[5] = {1550, 1550, 1550, 0, 0};
uint16_t robot_state_startY[5] = {590, 560, 530, 0, 0};
uint16_t robot_state_endX[5] = {1620, 1600, 1580, 0, 0};
uint16_t robot_state_endY[5] = {590, 560, 530, 0, 0};
uint8_t robot_width[3] = {0, 0, 0};

uint16_t Start_x1[7] = {850, 850, 900, 945, 1000, 1050, 0};
uint16_t Start_y1[7] = {400, 400, 400, 400, 400, 400, 0};
uint16_t End_x1[7] = {1050, 850, 900, 945, 1000, 1050, 0};
uint16_t End_y1[7] = {400, 410, 410, 410, 410, 410, 0};

uint16_t Start_x2[7] = {750, 750, 850, 945, 1050, 1150, 0};
uint16_t Start_y2[7] = {350, 350, 350, 350, 350, 350, 0};
uint16_t End_x2[7] = {1150, 750, 850, 945, 1050, 1150, 0};
uint16_t End_y2[7] = {350, 360, 360, 360, 360, 360, 0};

uint16_t Center_x_15[5] = {960, 960, 960, 960, 960};
uint16_t Center_y_15[5] = {430, 420, 400, 290, 245};
uint16_t Radius_15[5] = {3, 3, 3, 3, 3}; // 3

uint16_t Center_x_30[5] = {945, 945, 945, 945, 945};
uint16_t Center_y_30[5] = {430, 420, 400, 290, 245};
uint16_t Radius_30[5] = {3, 3, 3, 3, 3}; // 3

void UI_Task(void)
{
	dt = DWT_GetDeltaT(&UI_DWT_Count);
	t += dt;
	uiTimeStamp = DWT_GetDeltaT(&ui_DWT_Cost);

	static uint8_t state = 0;

	switch (state)
	{
	case 0:
		state++;
		Show_UI_Init();
		break;

	case 1:
		state = 0;
		Show_UI();
		break;

	default:
		state = 0;
		break;
	}

	uiTimeStamp = DWT_GetDeltaT(&ui_DWT_Cost);
	uiCostTime = uiTimeStamp;
}

static void Show_UI_Init(void)
{
	static uint8_t state = 0;

	switch (state)
	{
	case 0:
		state++;
		Layer1(GRAPHIC_ADD);
		break;

	case 1:
		state++;
		Layer2(GRAPHIC_ADD);
		break;

	case 2:
		state++;
		Layer3(GRAPHIC_ADD);
		break;

	case 3:
		state++;
		Layer4(GRAPHIC_ADD);
		break;

	case 4:
		state++;
		Layer5(GRAPHIC_ADD);
		break;

	case 5:
		state++;
		Layer6(GRAPHIC_ADD);
		break;
	case 6:
		state = 0;
		Layer7(GRAPHIC_ADD);

		break;

	default:
		state = 0;
		break;
	}
}

static void Show_UI(void)
{
	static uint8_t state = 0;
	switch (state)
	{
	case 0:
		state++;
		Layer1(GRAPHIC_MODIFY);
		break;

	case 1:
		state++;
		Layer2(GRAPHIC_MODIFY);
		break;

	case 2:
		state++;
		Layer3(GRAPHIC_MODIFY);
		break;
	case 3:
		state++;
		Layer4(GRAPHIC_MODIFY);

		break;
	case 4:
		state++;
		Layer5(GRAPHIC_MODIFY);
		break;
	case 5:
		state++;
		Layer6(GRAPHIC_MODIFY);
		break;
	case 6:
		state = 0;
		Layer7(GRAPHIC_MODIFY);
		break;

	default:
		state = 0;
		break;
	}
}

static void Layer1(uint8_t operate)
{
	memset(client_send_buff, 0, sizeof(client_send_buff));
	if (Power_Control.Is_Cap_Used == TRUE)
		memcpy(client_send_buff, "12          18           24", strlen("12          18           24"));
	else if (Power_Control.Is_Cap_Used == FALSE)
		memcpy(client_send_buff, "          CAP OFF           ", strlen("          CAP OFF           "));
	client_draw_char_2(JudgeUSART, &Char2, 2, operate, 706, 815, 20, robot_state.robot_id, robot_state.robot_id + 256, client_send_buff, 30, CLIENT_GREEN);
}

static void Layer2(uint8_t operate)
{
	static int count = 0;
	static uint8_t color;
	rect_Generate(&graphic[1], 2, 1, operate, rect_startX, rect_startY, rect_endX, rect_endY, 3, CLIENT_GREEN); // Cap energy frame

	// Camera Range
	if (Gimbal_Date.MiniPC_state == FALSE)
		rect_Generate(&graphic[2], 2, 2, operate, rect_AimRange_startX, rect_AimRange_startY, rect_AimRange_startX, rect_AimRange_startY, 3, CLIENT_Black); // disappear
	else if (Gimbal_Date.MiniPC_state == TRUE && Gimbal_Date.Gimbal_mode == 0)
		rect_Generate(&graphic[2], 2, 2, operate, rect_AimRange_startX, rect_AimRange_startY, rect_AimRange_endX, rect_AimRange_endY, 3, CLIENT_RedOrBlue);
	else if (Gimbal_Date.MiniPC_state == TRUE && Gimbal_Date.Gimbal_mode == 1)
		rect_Generate(&graphic[2], 2, 2, operate, rect_AimRange_startX, rect_AimRange_startY, rect_AimRange_endX, rect_AimRange_endY, 3, CLIENT_Amaranth);

	if(Gimbal_Date.Is_ShootEvaluation_On == FALSE)// Gun position
		color = CLIENT_Pink;
	else
		color = CLIENT_GREEN;
	
	if(Gimbal_Date.Is_LidOpen==FALSE)//When the lid is open, disappear gun position frame
		rect_Generate(&graphic[6], 2, 6, operate, rect_Aim_startX, rect_Aim_startY, rect_Aim_endX, rect_Aim_endY, 1, color);
	else
		rect_Generate(&graphic[6], 2, 6, operate, rect_Aim_startX, rect_Aim_startY, rect_Aim_startX, rect_Aim_startY, 1, color);
	
	rect_Generate(&graphic[4], 2, 9, operate, 1180, 350, 1230, 750, 1, CLIENT_Yellow);// Bullet number frame

	// Bullet number
	if (projectile_allowance.projectile_allowance_17mm > 100 && projectile_allowance.projectile_allowance_17mm < 10000)
		Line_Generate(&graphic[3], 2, 10, operate, 1205, 350, 1205, 750, 40, CLIENT_GREEN);
	else if (projectile_allowance.projectile_allowance_17mm > 30 && projectile_allowance.projectile_allowance_17mm <= 100)
		Line_Generate(&graphic[3], 2, 10, operate, 1205, 350, 1205, 350 + 4 * projectile_allowance.projectile_allowance_17mm, 40, CLIENT_GREEN);
	else if (projectile_allowance.projectile_allowance_17mm <= 30 && projectile_allowance.projectile_allowance_17mm >= 0)
		Line_Generate(&graphic[3], 2, 10, operate, 1205, 350, 1205, 350 + 4 * projectile_allowance.projectile_allowance_17mm, 40, CLIENT_Pink);
	else if (projectile_allowance.projectile_allowance_17mm > 10000) // Over shoot
		Line_Generate(&graphic[3], 2, 10, operate, 1205, 350, 1205, 350, 40, CLIENT_Pink);

	count++;
	if (count < 3)
		circular_Generate(&graphic[5], 2, 11, operate, 200, 800, 10, 20, CLIENT_GREEN);
	else if (count < 6)
	{
		circular_Generate(&graphic[5], 2, 11, operate, 0, 0, 0, 0, CLIENT_Pink);
	}
	else
		count = 0;

	client_draw(JudgeUSART, 7, graphic, robot_state.robot_id, robot_state.robot_id + 256);
}
static void Layer3(uint8_t operate)
{
	uint8_t lens[3];

	memset(client_send_buff, 0, sizeof(client_send_buff));
	memcpy(client_send_buff, "Sil Fol Spi           S Rob L", strlen("Sil Fol Spi           S Rob L"));
	if (Power_Control.Is_Cap_On == TRUE)
		client_draw_char_2(JudgeUSART, &Char2, 3, operate, 720, 200, 15, robot_state.robot_id, robot_state.robot_id + 256, client_send_buff, 30, CLIENT_Amaranth);
	else
		client_draw_char_2(JudgeUSART, &Char2, 3, operate, 720, 200, 15, robot_state.robot_id, robot_state.robot_id + 256, client_send_buff, 30, CLIENT_RedOrBlue);
}
static void Layer4(uint8_t operate)
{
	static uint16_t end_x_chassis;
	static uint16_t end_y_chassis;
	static uint16_t end_x_shoot;
	static uint16_t end_y_shoot;
	static uint16_t end_x_gimbal;
	static uint16_t end_y_gimbal;
	// 	memset(client_send_buff, 0, sizeof(client_send_buff));

	//   memset(client_send_buff_0, 0, sizeof(client_send_buff_0));
	// 	sprintf((char *)client_send_buff_0, "CAP %.1f\nPit %.1f\nPwr %.1f\n", Cap.Voltage, Gimbal.PitchMotor.AngleInDegree,Cap.Power);

	// client_draw_char_2(JudgeUSART, &Char3, 1, operate, 180, 600, 20, robot_state.robot_id, robot_state.robot_id + 256, client_send_buff_0, 30, CLIENT_RedOrBlue);

	uint8_t cap_color;
	uint16_t end_x_cap;
	if (Cap.Voltage > 18)
		cap_color = CLIENT_GREEN;
	else if (Cap.Voltage > 14)
		cap_color = CLIENT_Yellow;
	else
		cap_color = CLIENT_Pink;

	end_x_cap = 42.33 * Cap.Voltage + 198.04;
	Line_Generate(&graphic[0], 2, 0, operate, rect_startX, HIT_startY, end_x_cap, HIT_endY, HIT_Width, cap_color);

	if (Chassis.Mode == Silence_Mode)
	{
		end_x_chassis = 730;
		end_y_chassis = 180;
	}
	else if (Chassis.Mode == Follow_Mode)
	{
		end_x_chassis = 790;
		end_y_chassis = 180;
	}
	else if (Chassis.Mode == Spinning_Mode)
	{
		end_x_chassis = 850;
		end_y_chassis = 180;
	}

	switch (Gimbal_Date.Aimassist_mode)
	{
	case 1:
		end_x_shoot = 1100;
		end_y_shoot = 180;
		break;
	case 2:
		end_x_shoot = 1050;
		end_y_shoot = 180;
		break;
	case 3:
		end_x_shoot = 1150;
		end_y_shoot = 180;
		break;
	}

	if (Power_Control.Is_Cap_On == TRUE)
	{
		Line_Generate(&graphic[1], 2, 3, operate, 820, 100, end_x_chassis, end_y_chassis, HIT_Width, CLIENT_Amaranth);
		Line_Generate(&graphic[2], 2, 4, operate, 1100, 100, end_x_shoot, end_y_shoot, HIT_Width, CLIENT_Amaranth);
		// Line_Generate(&graphic[3], 2, 5, operate, 1180, 100, end_x_gimbal, end_y_gimbal, HIT_Width, CLIENT_Amaranth);
	}
	else
	{
		Line_Generate(&graphic[1], 2, 3, operate, 820, 100, end_x_chassis, end_y_chassis, HIT_Width, CLIENT_RedOrBlue);
		Line_Generate(&graphic[2], 2, 4, operate, 1100, 100, end_x_shoot, end_y_shoot, HIT_Width, CLIENT_RedOrBlue);
		// Line_Generate(&graphic[3], 2, 5, operate, 1180, 100, end_x_gimbal, end_y_gimbal, HIT_Width, CLIENT_RedOrBlue);
	}

	// Robot Position
	Line_Generate(&graphic[4], 2, 7, operate, 580, 0, 780, 350, 5, CLIENT_RedOrBlue);
	Line_Generate(&graphic[5], 2, 8, operate, 1400, 0, 1180, 350, 5, CLIENT_RedOrBlue);
	client_draw(JudgeUSART, 7, graphic, robot_state.robot_id, robot_state.robot_id + 256);
}
static void Layer5(uint8_t operate)
{
	int16_t load_ui = Gimbal_Date.Aimassist_debug_show;
	memset(client_send_buff, 0, sizeof(client_send_buff));

	for (int i = 0; i < 16; i++)
	{
		if (!isnormal((float)load_ui))
			load_ui = 0;

		client_send_buff[i] = load_ui % 2 + '0';
		load_ui = (uint32_t)load_ui / 2;
	}
	client_draw_char_2(JudgeUSART, &Char5, 5, operate, 200, 700, 13, robot_state.robot_id, robot_state.robot_id + 256, client_send_buff, 30, CLIENT_RedOrBlue);
}
static void Layer6(uint8_t operate)
{
	memset(client_send_buff, 0, sizeof(client_send_buff));

	switch (Gimbal_Date.Aimassist_target)
	{
	case NO:
		memcpy(client_send_buff, "NONE", strlen("NONE"));
		break;
	case HERO:
		memcpy(client_send_buff, "HERO", strlen("HERO"));
		break;
	case ENGINEER:
		memcpy(client_send_buff, "ENGINEER", strlen("ENGINEER"));
		break;
	case INFANTRY_3:
		memcpy(client_send_buff, "INFANTRY_3", strlen("INFANTRY_3"));
	case INFANTRY_4:
		memcpy(client_send_buff, "INFANTRY_4", strlen("INFANTRY_4"));
	case INFANTRY_5:
		memcpy(client_send_buff, "INFANTRY_5", strlen("INFANTRY_5"));
		break;
	case SENTRY:
		memcpy(client_send_buff, "SENTRY", strlen("SENTRY"));
		break;
	case BASE:
		memcpy(client_send_buff, "BASE", strlen("BASE"));
		break;
	case OUTPOST:
		memcpy(client_send_buff, "OUTPOST", strlen("OUTPOST"));
		break;
	default:
		memcpy(client_send_buff, "NONE", strlen("NONE"));
		break;
	}

	client_draw_char_2(JudgeUSART, &Char6, 6, operate, 720, 500, 20, robot_state.robot_id, robot_state.robot_id + 256, client_send_buff, 30, CLIENT_RedOrBlue);
}
static void Layer7(uint8_t operate) // WARNING
{
	uint8_t lens[4];
	memset(client_send_warning_buff, 0, sizeof(client_send_warning_buff));
	if (Chassis.Mode != Spinning_Mode)
	{
		lens[0] = strlen("No Spin\n");
		memcpy(client_send_warning_buff, "No Spin\n", strlen("No Spin\n"));
	}
	else
	{
		lens[0] = strlen("\n");
		memcpy(client_send_warning_buff, "\n", strlen("\n"));
	}

	if (remote_control.switch_left != Switch_Middle || remote_control.switch_right != Switch_Middle)
	{
		lens[1] = strlen("RC Mid\n\n\n");
		memcpy(client_send_warning_buff + lens[0], "RC Mid\n\n\n", strlen("RC Mid\n\n\n"));
	}
	else
	{
		lens[1] = strlen("\n\n\n");
		memcpy(client_send_warning_buff + lens[0], "\n\n\n", strlen("\n\n\n"));
	}

	if (Gimbal_Date.Real_Fric == FALSE)
	{
		lens[2] = strlen("FRIC\n");
		memcpy(client_send_warning_buff + lens[0] + lens[1], "FRIC\n", strlen("FRIC\n"));
	}
	else
	{
		lens[2] = strlen("\n");
		memcpy(client_send_warning_buff + lens[0] + lens[1], "\n", strlen("\n"));
	}

	if (is_TOE_Error(RC_TOE))
	{
		lens[3] = strlen("RC");
		memcpy(client_send_warning_buff + lens[0] + lens[1] + lens[2], "RC", strlen("RC"));
	}
	else
	{
		lens[3] = strlen("");
		memcpy(client_send_warning_buff + lens[0] + lens[1] + lens[2], "", strlen(""));
	}
	

	client_draw_char(JudgeUSART, &Char7, 7, operate, 725, 700, 50, robot_state.robot_id, robot_state.robot_id + 256, client_send_warning_buff, 30, CLIENT_Yellow);
}

uint8_t float_to_char(float floatdata, uint8_t *buffer)
{
	int32_t slope;
	int32_t temp;
	int8_t i, j;
	uint8_t len = 0;

	slope = (int32_t)(floatdata * 100 + 0.5f);
	if (slope < 0)
	{
		buffer[len] = '-';
		slope = -slope;
		len += 1;
	}
	temp = slope;

	for (i = 0; temp != 0; i++)
		temp /= 10;
	temp = slope;

	for (j = i; j >= 0; j--)
	{
		buffer[len + j] = temp % 10 + '0';
		temp /= 10;
		if ((i - j) == 1)
		{
			buffer[len + j - 1] = '.';
			j -= 1;
		}
	}
	len += i + 1;

	buffer[len] = '\n';
	len += 1;
	return len;
}