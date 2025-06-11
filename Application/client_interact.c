#include "client_interact.h"
#include "judgement_info.h"
#include "string.h"
#include "stm32f4xx_hal.h"
#include "usart.h"

extern const uint8_t CRC8_INIT;
extern uint16_t CRC_INIT;

uint8_t UI_SendBuf[120] = {0};

void client_draw_char(
    UART_HandleTypeDef *huart, client_char_t *client_char,
    uint8_t layer, uint8_t operate,
    uint16_t start_x, uint16_t start_y, uint8_t size,
    uint16_t sender, uint16_t receiver, uint8_t *buff, uint8_t buff_len, uint8_t color)
{
    static uint16_t crc_16;
    if (buff_len > 30)
        return;
    client_char->frame_header.Sof = 0xa5;
    client_char->frame_header.data_length = 51;
    client_char->frame_header.seq = 2;
    memcpy(UI_SendBuf, client_char, 4);
    client_char->frame_header.CRC8 = Get_CRC8_Check_Sum((unsigned char *)UI_SendBuf, 4, CRC8_INIT);
    client_char->frame_header.cmd_id = ROBOT_COMMUICATE;     
    client_char->frame_header.content_id = ROBOT_DRAW_CHARA; 
    client_char->frame_header.sender_id = sender;           
    client_char->frame_header.receiver_id = receiver;        

    client_char->graphic.operate_tpye = operate;    
    client_char->graphic.graphic_tpye = CLIENT_CHAR; 
    client_char->graphic.layer = layer;              
    client_char->graphic.color = color;             
    client_char->graphic.graphic_name[0] = '0';     
    client_char->graphic.graphic_name[1] = '0' + 6;
    client_char->graphic.graphic_name[2] = '0'+ layer;
    client_char->graphic.width = 3;        
    client_char->graphic.start_x = start_x; 
    client_char->graphic.start_y = start_y;
    client_char->graphic.end_x = 0;
    client_char->graphic.end_y = 0;
    client_char->graphic.start_angle = size;
    client_char->graphic.end_angle = buff_len;
    memcpy(UI_SendBuf, client_char, sizeof(client_char_t));
    memcpy(client_char->data, buff, buff_len);
    crc_16 = Get_CRC16_Check_Sum(UI_SendBuf, 58, CRC_INIT);
    client_char->frame_tail = crc_16;
    memcpy(UI_SendBuf, client_char, sizeof(client_char_t));
    if (HAL_UART_GetState(huart) != HAL_UART_STATE_BUSY_TX && (__HAL_UART_GET_FLAG(huart, UART_FLAG_TC) == SET))
        HAL_UART_Transmit_DMA(huart, UI_SendBuf, sizeof(client_char_t));
}

void Line_Generate(graphic_data_struct_t *line, uint8_t layer, uint8_t name, uint8_t operate, uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y, uint8_t width, uint8_t color)
{
    line->operate_tpye = operate;     
    line->graphic_tpye = CLIENT_LINE; 
    line->layer = layer;             
    line->color = color;              
    line->graphic_name[0] = '0';     
    line->graphic_name[1] = '0' + layer;
    line->graphic_name[2] = '0' + name;
    line->width = width;   
    line->start_x = start_x; 
    line->start_y = start_y; 
    line->end_x = end_x;     
    line->end_y = end_y;    
}

void circular_Generate(graphic_data_struct_t *circular, uint8_t layer, uint8_t name, uint8_t operate, uint16_t center_x, uint16_t center_y, uint16_t radius, uint8_t width, uint8_t color)
{
    circular->operate_tpye = operate;         
    circular->graphic_tpye = CLIENT_CIRCULAR; 
    circular->layer = layer;                 
    circular->color = color;                  
    circular->graphic_name[0] = '0';         
    circular->graphic_name[1] = '0' + layer;
    circular->graphic_name[2] = '0' + name;
    circular->width = width;      
    circular->start_x = center_x; 
    circular->start_y = center_y; 
    circular->radius = radius;    
}


void rect_Generate(graphic_data_struct_t *rect, uint8_t layer, uint8_t name, uint8_t operate, uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y, uint8_t width, uint8_t color)
{
    rect->operate_tpye = operate;    
    rect->graphic_tpye = CLIENT_Rect; 
    rect->layer = layer;             
    rect->color = color;             
    rect->graphic_name[0] = '0';      
    rect->graphic_name[1] = '0' + layer;
    rect->graphic_name[2] = '0' + name;
    rect->width = width;     
    rect->start_x = start_x;
    rect->start_y = start_y; 
    rect->end_x = end_x;     
    rect->end_y = end_y;     
}
void client_draw(UART_HandleTypeDef *huart, uint8_t graphic_num, graphic_data_struct_t *graphic, uint16_t sender, uint16_t receiver)
{
    static uint8_t send_length = 0; 
    static uint16_t crc_16;         

    static frame_header_t frame_header;
    static uint16_t frame_tail;

    switch (graphic_num)
    {
    case 0:
        send_length = 2;
        frame_header.content_id = DEL_CHART_ID;
        break;
    case 1:
        send_length = sizeof(client_frame_t);
        frame_header.content_id = ROBOT_DRAW_CHART_1;
        break;
    case 2:
        send_length = sizeof(client_frame_t_2);
        frame_header.content_id = ROBOT_DRAW_CHART_2;
        break;
    case 5:
        send_length = sizeof(client_frame_t_5);
        frame_header.content_id = ROBOT_DRAW_CHART_5;
        break;
    case 7:
        send_length = sizeof(client_frame_t_7);
        frame_header.content_id = ROBOT_DRAW_CHART_7;
        break;
    default:
        return;
    }

    frame_header.Sof = 0xa5;
    frame_header.data_length = send_length - 9; 
    frame_header.seq = 2;
    memcpy(UI_SendBuf, &frame_header, 4);
    frame_header.CRC8 = Get_CRC8_Check_Sum((unsigned char *)UI_SendBuf, 4, CRC8_INIT);
    frame_header.cmd_id = ROBOT_COMMUICATE;
    frame_header.sender_id = sender;
    frame_header.receiver_id = receiver;
    memcpy(UI_SendBuf, &frame_header, sizeof(frame_header_t));


    memcpy(UI_SendBuf + sizeof(frame_header_t), graphic, send_length - 15);


    crc_16 = Get_CRC16_Check_Sum(UI_SendBuf, send_length - 2, CRC_INIT);
    frame_tail = crc_16;
    memcpy(UI_SendBuf + send_length - 2, &frame_tail, 2);

    if (HAL_UART_GetState(huart) != HAL_UART_STATE_BUSY_TX && (__HAL_UART_GET_FLAG(huart, UART_FLAG_TC) == SET))
        HAL_UART_Transmit_DMA(huart, UI_SendBuf, send_length);
}
void client_draw_char_2(
    UART_HandleTypeDef *huart, client_frame_t *line,
    uint8_t layer, uint8_t operate,
    uint16_t start_x, uint16_t start_y, uint8_t size,
    uint16_t sender, uint16_t receiver, uint8_t *buff, uint8_t buff_len, uint8_t color)
{
    static uint8_t temp_buff[60] = {0};
    static uint16_t crc_16;

    uint8_t *p = temp_buff;

    if (buff_len > 30)
        return;
    line->frame_header.Sof = 0xa5;
    line->frame_header.data_length = 51;
    line->frame_header.seq = 2;
    memcpy(temp_buff, line, 4);
    line->frame_header.CRC8 = Get_CRC8_Check_Sum((unsigned char *)temp_buff, 4, CRC8_INIT);
    line->frame_header.cmd_id = ROBOT_COMMUICATE;     
    line->frame_header.content_id = ROBOT_DRAW_CHARA; 
    line->frame_header.sender_id = sender;            
    line->frame_header.receiver_id = receiver;  

    line->graphic.operate_tpye = operate;     
    line->graphic.graphic_tpye = CLIENT_CHAR; 
    line->graphic.layer = layer;              
    line->graphic.color = color;            
    line->graphic.graphic_name[0] = '0';      
    line->graphic.graphic_name[1] = '0';
    line->graphic.graphic_name[2] = '0' + layer;
    line->graphic.width = 3;       
    line->graphic.start_x = start_x; 
    line->graphic.start_y = start_y;
    line->graphic.end_x = 0;
    line->graphic.end_y = 0;
    line->graphic.start_angle = size;
    line->graphic.end_angle = buff_len;
    memcpy(temp_buff, line, sizeof(client_frame_t));

    p += 28;
    memcpy(p, buff, buff_len);
    crc_16 = Get_CRC16_Check_Sum(temp_buff, 58, CRC_INIT);
    temp_buff[58] = crc_16;
    temp_buff[59] = crc_16 >> 8;

    if (HAL_UART_GetState(huart) != HAL_UART_STATE_BUSY_TX && (__HAL_UART_GET_FLAG(huart, UART_FLAG_TC) == SET))
        HAL_UART_Transmit_DMA(huart, temp_buff, sizeof(temp_buff));
}
void Image_maps(image_maps_t *image_maps,int16_t x,uint16_t y,int16_t z)
{

}