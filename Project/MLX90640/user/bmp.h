//
// Created by ShenJunjie on 2021/8/25.
//

#ifndef SD_TEST_BMP_H
#define SD_TEST_BMP_H
#include<stdio.h>
#include<stdint.h>
#include<string.h>
#include "st7789.h"
#include "math.h"
#include "fatfs.h"
#include "arm_math.h"
#include "user.h"
#define abs(s) (s)>0?(s):-1*(s)
#define IMG_WIDTH 288
#define IMG_HEIGHT 216
#define IMG_INITIAL_WIDTH 32
#define IMG_INITIAL_HEIGHT 24
#define savefile 0x01
#define notsavefile 0x00
#define GCM_Pseudo1 0x01
#define GCM_Pseudo2 0x02
#define GCM_Rainbow1 0x03
#define GCM_Rainbow2 0x04
#define GCM_Ning 0x05
#define GCM_Rainbow3 0x06
#define GCM_Zhou 0x07
#define GCM_Gray 0x08
#define GCM_Metal1 0x09
#define GCM_Metal2 0x0A
typedef struct BITMAPFILEHEADER
{
    uint16_t bfType;        //2Bytes，必须为"BM"，即0x424D 才是Windows位图文件
    uint32_t bfSize;         //4Bytes，整个BMP文件的大小
    uint16_t bfReserved1;  //2Bytes，保留，为0
    uint16_t bfReserved2;  //2Bytes，保留，为0
    uint32_t bfOffBits;     //4Bytes，文件起始位置到图像像素数据的字节偏移量
} BITMAPFILEHEADER;
typedef struct BMP_INFOHEADER
{
    uint32_t  biSize;    //4Bytes，INFOHEADER结构体大小，存在其他版本I NFOHEADER，用作区分
    uint32_t   biWidth;    //4Bytes，图像宽度（以像素为单位）
    uint32_t   biHeight;    //4Bytes，图像高度，+：图像存储顺序为Bottom2Top，-：Top2Bottom
    uint16_t biPlanes;    //2Bytes，图像数据平面，BMP存储RGB数据，因此总为1
    uint16_t   biBitCount;         //2Bytes，图像像素位数
    uint32_t  biCompression;     //4Bytes，0：不压缩，1：RLE8，2：RLE4
    uint32_t  biSizeImage;       //4Bytes，4字节对齐的图像数据大小
    uint32_t   biXPelsPerMeter;   //4 Bytes，用象素/米表示的水平分辨率
    uint32_t   biYPelsPerMeter;   //4 Bytes，用象素/米表示的垂直分辨率
    uint32_t  biClrUsed;          //4 Bytes，实际使用的调色板索引数，0：使用所有的调色板索引
    uint32_t biClrImportant;     //4 Bytes，重要的调色板索引数，0：所有的调色板索引都重要
}BMP_INFOHEADER;
typedef struct RGBQUAD
{
    uint8_t rgbBlue;       //指定蓝色强度
    uint8_t  rgbGreen;      //指定绿色强度
    uint8_t  rgbRed;        //指定红色强度
    uint8_t  rgbReserved;  //保留，设置为0
} RGBQUAD;
typedef struct BMP_INFO
{
    BITMAPFILEHEADER bitmapfileheader;
    BMP_INFOHEADER bmpInfoheader;
    uint32_t RGB_MASK[3];
}BMP_INFO;
typedef struct BMP_8
{
    uint8_t gray_val;
}BMP_8;
typedef struct BMP_24
{
    uint8_t r_val;
    uint8_t g_val;
    uint8_t b_val;

}BMP_24;
typedef struct RGB565
{
    uint8_t r_bit;
    uint8_t g_bit;
    uint8_t b_bit;
}RGB565;

uint8_t ImgReadHeader(BITMAPFILEHEADER*Header, FIL* fp);
void ImgReadInfo(BMP_INFOHEADER* INFO, FIL* fp);
uint8_t ImgReadData(FIL* fp, BITMAPFILEHEADER* Header, BMP_INFOHEADER* INFO, BMP_8 bmp8[][IMG_WIDTH],BMP_24 bmp24[][IMG_WIDTH]);
void bmp24_to_rgb565(BMP_24 bmp24[][IMG_WIDTH],RGB565 rgb565[][IMG_WIDTH],uint16_t img[][IMG_WIDTH]);
void bmp8_to_bmp1(BMP_8 bmp8[][IMG_WIDTH],uint8_t img[][IMG_WIDTH/8],uint8_t thre);
uint8_t temp_to_bmp8(BMP_8 bmp8[],const float temp_mat[],int size,float *max_temp,float *min_temp,uint8_t revese);
void Resize_draw(BMP_8 bmp8[],uint8_t method);
uint8_t Besize_draw(BMP_8 input_bmp8[],uint8_t method,uint8_t ifsavefile);
void gray_to_rgb(BMP_8 bmp8[],BMP_24 bmp24[][IMG_INITIAL_WIDTH],uint8_t method);
void bmp8_to_color(uint8_t method,uint8_t gray_val,BMP_24*bmp24);
void draw_spectrum(uint8_t method);
void bmp_header_init(BMP_INFO *bmp_info);
void save_bmp_header(FIL *file,BMP_INFO *bmp_info);
uint8_t get_filenum();
#endif //SD_TEST_BMP_H
