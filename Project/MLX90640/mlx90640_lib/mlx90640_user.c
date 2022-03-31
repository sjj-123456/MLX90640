//
// Created by 21312 on 2021/12/12.
//
#include "mlx90640_user.h"
uint16_t frame[834];
float emissivity=0.95f;
uint8_t mlx90640_init(uint8_t refresh_rate,paramsMLX90640 *mlx90640)
{
    int status;
    MLX90640_SetRefreshRate(MLX90640_ADDR, refresh_rate);
    MLX90640_SetChessMode(MLX90640_ADDR);
    status = MLX90640_DumpEE(MLX90640_ADDR, eeMLX90640);
    if (status != 0)
        UART_printf(&huart1,"\r\nload system parameters error with code:%d\r\n",status);
    status = MLX90640_ExtractParameters(eeMLX90640, mlx90640);
    if (status != 0)
        UART_printf(&huart1,"\r\nParameter extraction failed with error code:%d\r\n",status);
    return status;
}

uint8_t mlx90640_get_temp_mat(paramsMLX90640 *mlx90640,float*tempmat)
{
    int status;
    status = MLX90640_GetFrameData(MLX90640_ADDR, frame);
    while(status<0)
    {
        status = MLX90640_GetFrameData(MLX90640_ADDR, frame);
        UART_printf(&huart1,"GetFrame Error: %d\r\n",status);
    }
    float vdd = MLX90640_GetVdd(frame, mlx90640);
    float Ta = MLX90640_GetTa(frame, mlx90640);

    float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
    MLX90640_CalculateTo(frame, mlx90640, emissivity , tr, tempmat);
    return 0;
}