#ifndef _UI_TASK_H
#define _UI_TASK_H

#include "string.h"

#include "judgement_info.h"
#include "bsp_usart_idle.h"
#include "detect_task.h"
#include "includes.h"

#define UI_TASK_PERIOD 33

void UI_Task(void);

//void UI_Buff_Generate(float a, float b, float c, uint8_t *data);//UI_buff生成
uint8_t float_to_char(float floatdata, uint8_t *buffer);//浮点数转字符串

#endif