//
// Created by 21312 on 2021/12/12.
//

#ifndef MLX90640_MLX90640_USER_H
#define MLX90640_MLX90640_USER_H
#include "MLX90640_API.h"
#include "user.h"
#define  FPS2HZ   0x02
#define  FPS4HZ   0x03
#define  FPS8HZ   0x04
#define  FPS16HZ  0x05
#define  FPS32HZ  0x06

#define  MLX90640_ADDR 0x33
#define	 RefreshRate FPS16HZ
#define  TA_SHIFT 8 //Default shift for MLX90640 in open air

static uint16_t eeMLX90640[832];

uint8_t mlx90640_init(uint8_t refresh_rate,paramsMLX90640 *mlx90640);
uint8_t mlx90640_get_temp_mat(paramsMLX90640 *mlx90640,float*tempmat);
#endif //MLX90640_MLX90640_USER_H
