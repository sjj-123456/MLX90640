//
// Created by 21312 on 2021/12/12.
//

#ifndef MLX90640_USER_H
#define MLX90640_USER_H
#include "stdio.h"
#include "usart.h"
#include "stdarg.h"
typedef struct ENCODER{
    int32_t cnt;
    int32_t last_cnt;
    int direction;
}ENCODER;
typedef struct menu{
    char color_method[4][30];
    int chose_color_method_index;
    char resize_or_binsize[2][30];
    int resize_or_binsize_index;
    char screenshot[4][30];
    int screenshot_index;
    char voltage_info[30];
    char min_temp_info[30];
    char max_temp_info[30];
    char center_temp_info[30];
}MENU;
int UART_printf(UART_HandleTypeDef *huart, const char *fmt, ...);
#endif //MLX90640_USER_H
