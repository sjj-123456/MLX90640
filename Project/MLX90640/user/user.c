//
// Created by 21312 on 2021/12/12.
//
#include "user.h"

int UART_printf(UART_HandleTypeDef *huart, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int length;
    char buffer[128];
    length = vsnprintf(buffer, 128, fmt, ap);
    HAL_UART_Transmit(huart,buffer,length,HAL_MAX_DELAY);
    va_end(ap);
    return length;
}