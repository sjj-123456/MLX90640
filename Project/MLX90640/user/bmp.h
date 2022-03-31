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
    uint16_t bfType;        //2Bytes������Ϊ"BM"����0x424D ����Windowsλͼ�ļ�
    uint32_t bfSize;         //4Bytes������BMP�ļ��Ĵ�С
    uint16_t bfReserved1;  //2Bytes��������Ϊ0
    uint16_t bfReserved2;  //2Bytes��������Ϊ0
    uint32_t bfOffBits;     //4Bytes���ļ���ʼλ�õ�ͼ���������ݵ��ֽ�ƫ����
} BITMAPFILEHEADER;
typedef struct BMP_INFOHEADER
{
    uint32_t  biSize;    //4Bytes��INFOHEADER�ṹ���С�����������汾I NFOHEADER����������
    uint32_t   biWidth;    //4Bytes��ͼ���ȣ�������Ϊ��λ��
    uint32_t   biHeight;    //4Bytes��ͼ��߶ȣ�+��ͼ��洢˳��ΪBottom2Top��-��Top2Bottom
    uint16_t biPlanes;    //2Bytes��ͼ������ƽ�棬BMP�洢RGB���ݣ������Ϊ1
    uint16_t   biBitCount;         //2Bytes��ͼ������λ��
    uint32_t  biCompression;     //4Bytes��0����ѹ����1��RLE8��2��RLE4
    uint32_t  biSizeImage;       //4Bytes��4�ֽڶ����ͼ�����ݴ�С
    uint32_t   biXPelsPerMeter;   //4 Bytes��������/�ױ�ʾ��ˮƽ�ֱ���
    uint32_t   biYPelsPerMeter;   //4 Bytes��������/�ױ�ʾ�Ĵ�ֱ�ֱ���
    uint32_t  biClrUsed;          //4 Bytes��ʵ��ʹ�õĵ�ɫ����������0��ʹ�����еĵ�ɫ������
    uint32_t biClrImportant;     //4 Bytes����Ҫ�ĵ�ɫ����������0�����еĵ�ɫ����������Ҫ
}BMP_INFOHEADER;
typedef struct RGBQUAD
{
    uint8_t rgbBlue;       //ָ����ɫǿ��
    uint8_t  rgbGreen;      //ָ����ɫǿ��
    uint8_t  rgbRed;        //ָ����ɫǿ��
    uint8_t  rgbReserved;  //����������Ϊ0
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
