//
// Created by ShenJunjie on 2021/8/25.
//
#include "bmp.h"
FATFS spi_fs;
FIL fil;
unsigned char work[4096] = {0};
unsigned int count = 0;
BMP_INFO bmpinfo;
FRESULT retSD;
uint8_t ImgReadHeader(BITMAPFILEHEADER*Header, FIL* fp)
{
    uint32_t byteswritten;
    uint8_t buffer[100];
    f_read(fp,buffer,sizeof(BITMAPFILEHEADER),&byteswritten);
    //fread(buffer, sizeof(BITMAPFILEHEADER), 1, fp);
    Header->bfType =buffer[0]<<8| buffer[1];
    Header->bfSize = buffer[5] << 24 | buffer[4] << 16 | buffer[3] << 8 | buffer[4];
    Header->bfReserved1 = buffer[7] << 8 | buffer[6];
    Header->bfReserved2 = buffer[9] << 8 | buffer[8];
    Header->bfOffBits= buffer[13]<<24 | buffer[12] << 16 | buffer[11] << 8 | buffer[10];
    if (Header->bfType == 0x424D)
        return 1;
    return 0;
}
void ImgReadInfo(BMP_INFOHEADER* INFO, FIL* fp)
{
    uint32_t byteswritten;
    //fseek(fp, 14L, SEEK_SET);
    f_lseek(fp,14);
    uint8_t buffer[100];
    f_read(fp,buffer,sizeof(BMP_INFOHEADER),&byteswritten);
   // fread(buffer, sizeof(BMP_INFOHEADER), 1, fp);
    INFO->biSize= buffer[3] << 24 | buffer[2] << 16 | buffer[1] << 8 | buffer[0];
    INFO->biWidth= buffer[7] << 24 | buffer[6] << 16 | buffer[5] << 8 | buffer[4];
    INFO->biHeight = buffer[11] << 24 | buffer[10] << 16 | buffer[9] << 8 | buffer[8];
    INFO->biPlanes =buffer[13] << 8 | buffer[12];
    INFO->biBitCount =  buffer[15] << 8 | buffer[14];
    INFO->biCompression = buffer[19] << 24 | buffer[18] << 16 | buffer[17] << 8 | buffer[16];
    INFO->biSizeImage = buffer[23] << 24 | buffer[22] << 16 | buffer[21] << 8 | buffer[20];
    INFO->biXPelsPerMeter = buffer[27] << 24 | buffer[26] << 16 | buffer[25] << 8 | buffer[24];
    INFO->biYPelsPerMeter = buffer[31] << 24 | buffer[30] << 16 | buffer[29] << 8 | buffer[28];
    INFO->biClrUsed = buffer[35] << 24 | buffer[34] << 16 | buffer[33] << 8 | buffer[32];
    INFO->biClrImportant = buffer[39] << 24 | buffer[38] << 16 | buffer[37] << 8 | buffer[36];
}
uint8_t ImgReadData(FIL* fp, BITMAPFILEHEADER* Header, BMP_INFOHEADER* INFO, BMP_8 bmp8[][IMG_WIDTH],BMP_24 bmp24[][IMG_WIDTH])
{
    if (ImgReadHeader(Header, fp) == 0)
        return 0;
    ImgReadInfo(INFO,fp);
    //fseek(fp, Header->bfOffBits, SEEK_SET);
    f_lseek(fp,Header->bfOffBits);
    uint32_t byteswritten;
    if (INFO->biBitCount == 24)
    {
        uint8_t buffer[3];
        for (int i = INFO->biHeight - 1; i >= 0; i--)
        {
            for (int j = 0; j < INFO->biWidth; j++)
            {
                f_read(fp,buffer,3*sizeof(uint8_t),&byteswritten);
               // fread(buffer, sizeof(uint8_t), 3, fp);
                bmp24[i][j].r_val = buffer[2];
                bmp24[i][j].g_val = buffer[1];
                bmp24[i][j].b_val = buffer[0];
            }
        }

    }
    else if (INFO->biBitCount == 8)
    {

        uint8_t buffer;
        for (int i = INFO->biHeight-1; i>=0; i--)
        {
            for (int j = 0; j < INFO->biWidth; j++)
            {
                //fread(&buffer, sizeof(uint8_t), 1, fp);
                f_read(fp,&buffer,sizeof(uint8_t),&byteswritten);
                bmp8[i][j].gray_val = buffer;
            }
        }
    }
    else
    {
        return 0;
    }
    return 1;
}

void bmp24_to_rgb565(BMP_24 bmp24[][IMG_WIDTH],RGB565 rgb565[][IMG_WIDTH],uint16_t img[][IMG_WIDTH])
{
    for (int i = 0; i < IMG_HEIGHT; i++)  //BMP24->RGB565
    {
        for (int j = 0; j < IMG_WIDTH; j++)
        {
           rgb565[i][j].r_bit =bmp24[i][j].r_val>>3;
           rgb565[i][j].g_bit = bmp24[i][j].g_val>>2;
           rgb565[i][j].b_bit = bmp24[i][j].b_val>>3;
        }
    }
    for (int i = 0; i < IMG_HEIGHT; i++)
    {
        for (int j = 0; j < IMG_WIDTH; j++)
        {
            img[i][j] =(rgb565[i][j].r_bit << 11 |rgb565[i][j].g_bit << 5 |rgb565[i][j].b_bit);
            img[i][j] =( img[i][j] >> 8) | ((img[i][j] & 0xFF) << 8);
        }
    }
}

void bmp8_to_bmp1(BMP_8 bmp8[][IMG_WIDTH],uint8_t img[][IMG_WIDTH/8],uint8_t thre)
{
    uint8_t after_thre[IMG_HEIGHT*IMG_WIDTH];
    for(int i=0;i<IMG_HEIGHT;i++)
    {
        for(int j=0;j<IMG_WIDTH;j++)
        {
            after_thre[i*IMG_WIDTH+j]=bmp8[i][j].gray_val>=thre;
        }
    }
    for(int i=0;i<IMG_HEIGHT*IMG_WIDTH;i+=8)
    {
        for(int j=0;j<8;j++)
        {
            **img= **img<<1|after_thre[i+j];
        }
        img++;
    }
}

uint8_t temp_to_bmp8(BMP_8 bmp8[],const float temp_mat[],int size,float *max_temp,float *min_temp,uint8_t revese)
{
    float low_threshold=1000;
    float high_threshold=-1000;
    int max_index=0;
    for(int i=0;i<size;i++)
    {
        if(temp_mat[i]<low_threshold)
            low_threshold=temp_mat[i];
        if(temp_mat[i]>high_threshold)
            high_threshold=temp_mat[i];
    }
    *max_temp=high_threshold;
    *min_temp=low_threshold;
    //温度异常退出重新获取
    if(high_threshold>300||low_threshold<-40)
        return 1;
    uint8_t max_gray=0;
    for(int i=0;i<size;i++)
    {
        if(!revese)
            bmp8[i].gray_val=(int)(temp_mat[i]*255.0f/(high_threshold-low_threshold)+-1*low_threshold*255.0f/(high_threshold-low_threshold));
        else
            bmp8[i].gray_val=(int)(temp_mat[(i / IMG_INITIAL_WIDTH + 1) * IMG_INITIAL_WIDTH - i - 1+ (i / IMG_INITIAL_WIDTH ) * IMG_INITIAL_WIDTH]*255.0f/(high_threshold-low_threshold)+-1*low_threshold*255.0f/(high_threshold-low_threshold));
        if(bmp8[i].gray_val>max_gray)
        {
            max_gray=bmp8[i].gray_val;
            max_index=i;
        }
    }
    max_index/IMG_INITIAL_WIDTH*IMG_HEIGHT+5;

    int max_y=max_index/IMG_INITIAL_WIDTH*IMG_HEIGHT/IMG_INITIAL_HEIGHT+(240-IMG_HEIGHT)/2+IMG_HEIGHT/IMG_INITIAL_HEIGHT/2;
    int max_x=(max_index%IMG_INITIAL_WIDTH)*IMG_WIDTH/IMG_INITIAL_WIDTH+(320-IMG_WIDTH)/2+IMG_WIDTH/IMG_INITIAL_WIDTH/2;
    ST7789_DrawFilledRectangle(max_x-6,max_y-1,12,2,BLACK);
    ST7789_DrawFilledRectangle(max_x-1,max_y-6,2,12,BLACK);
    return 0;
}

//邻近插值法
void Resize_draw(BMP_8 bmp8[],uint8_t method)
{
    BMP_24 input_bmp24[IMG_INITIAL_HEIGHT][IMG_INITIAL_WIDTH];
    gray_to_rgb(bmp8,input_bmp24,method);
    RGB565 TEMP_RGB565;

    ST7789_SetAddressWindow((320-IMG_WIDTH)/2, (240-IMG_HEIGHT)/2, IMG_WIDTH+(320-IMG_WIDTH)/2-1, IMG_HEIGHT+(240-IMG_HEIGHT)/2-1);
    ST7789_Select();
    ST7789_DC_Set();
    uint16_t data[IMG_WIDTH*IMG_HEIGHT/4];
    for(int i=0;i<IMG_INITIAL_HEIGHT;i++)
    {
        for(int j=0;j<IMG_INITIAL_WIDTH;j++)
        {
            TEMP_RGB565.r_bit =input_bmp24[i][j].r_val>>3;
            TEMP_RGB565.g_bit =input_bmp24[i][j].g_val>>2;
            TEMP_RGB565.b_bit =input_bmp24[i][j].b_val>>3;
            uint16_t color;
            color =(TEMP_RGB565.r_bit << 11 |TEMP_RGB565.g_bit << 5 |TEMP_RGB565.b_bit);
            color =( color >> 8) | ((color & 0xFF) << 8);
            if(i%6==0)
            {
                for(int a=0;a<IMG_WIDTH/IMG_INITIAL_WIDTH;a++)
                {
                    for(int b=0;b<IMG_WIDTH/IMG_INITIAL_WIDTH;b++)
                        data[a*IMG_WIDTH+j*IMG_WIDTH/IMG_INITIAL_WIDTH+b]=color;
                }
            }
            else if((i+5)%6==0)
            {
                for(int a=IMG_WIDTH/IMG_INITIAL_WIDTH;a<2*IMG_WIDTH/IMG_INITIAL_WIDTH;a++)
                {
                    for(int b=0;b<IMG_WIDTH/IMG_INITIAL_WIDTH;b++)
                        data[a*IMG_WIDTH+j*IMG_WIDTH/IMG_INITIAL_WIDTH+b]=color;
                }
            }
            else if((i+4)%6==0)
            {
                for(int a=2*IMG_WIDTH/IMG_INITIAL_WIDTH;a<3*IMG_WIDTH/IMG_INITIAL_WIDTH;a++)
                {
                    for(int b=0;b<IMG_WIDTH/IMG_INITIAL_WIDTH;b++)
                        data[a*IMG_WIDTH+j*IMG_WIDTH/IMG_INITIAL_WIDTH+b]=color;
                }
            }
            else if((i+3)%6==0)
            {
                for(int a=3*IMG_WIDTH/IMG_INITIAL_WIDTH;a<4*IMG_WIDTH/IMG_INITIAL_WIDTH;a++)
                {
                    for(int b=0;b<IMG_WIDTH/IMG_INITIAL_WIDTH;b++)
                        data[a*IMG_WIDTH+j*IMG_WIDTH/IMG_INITIAL_WIDTH+b]=color;
                }
            }
            else if((i+2)%6==0)
            {
                for(int a=4*IMG_WIDTH/IMG_INITIAL_WIDTH;a<5*IMG_WIDTH/IMG_INITIAL_WIDTH;a++)
                {
                    for(int b=0;b<IMG_WIDTH/IMG_INITIAL_WIDTH;b++)
                        data[a*IMG_WIDTH+j*IMG_WIDTH/IMG_INITIAL_WIDTH+b]=color;
                }
            }
            else if((i+1)%6==0)
            {
                for(int a=5*IMG_WIDTH/IMG_INITIAL_WIDTH;a<6*IMG_WIDTH/IMG_INITIAL_WIDTH;a++)
                {
                    for(int b=0;b<IMG_WIDTH/IMG_INITIAL_WIDTH;b++)
                        data[a*IMG_WIDTH+j*IMG_WIDTH/IMG_INITIAL_WIDTH+b]=color;
                }
            }
        }
        if(!((i+1)%6))
        {
            HAL_SPI_Transmit(hspi, (uint8_t *)data, sizeof(uint16_t) * IMG_WIDTH*IMG_HEIGHT/4, HAL_MAX_DELAY);
        }
    }

    ST7789_UnSelect();
}
// 双线性插值
uint8_t Besize_draw(BMP_8 input_bmp8[],uint8_t method,uint8_t ifsavefile)
{//注释的是自己的插值，现在使用dsp库中提供的
//    BMP_8 bmp8[IMG_INITIAL_HEIGHT][IMG_INITIAL_WIDTH];
    BMP_8 temp;
    BMP_24 temp_24;
    RGB565 TEMP_RGB565;
    uint16_t color[1];
    char filename[20];
    int i, j;
    int w = IMG_INITIAL_WIDTH;
    int h = IMG_INITIAL_HEIGHT;
    int iw = IMG_WIDTH;
    int ih = IMG_HEIGHT;
    float dx=((float)IMG_INITIAL_WIDTH-2.0f)*1.0f/(IMG_WIDTH-1);
    float dy=((float)IMG_INITIAL_HEIGHT-2.0f)*1.0f/(IMG_HEIGHT-1);
//    for(int a=0;a<IMG_INITIAL_HEIGHT;a++)
//    {
//        for(int b=0;b<IMG_INITIAL_WIDTH;b++)
//            bmp8[a][b].gray_val=input_bmp8[a*IMG_INITIAL_WIDTH+b].gray_val;
//    }
    arm_bilinear_interp_instance_f32 bilinear_data;
    bilinear_data.numCols=IMG_INITIAL_WIDTH;
    bilinear_data.numRows=IMG_INITIAL_HEIGHT;
    float data[IMG_INITIAL_WIDTH*IMG_INITIAL_HEIGHT];
    for(int a=0;a<IMG_INITIAL_HEIGHT*IMG_INITIAL_WIDTH;a++)
    {
        data[a]=(float)input_bmp8[a].gray_val;
    }
    bilinear_data.pData=data;
    if(ifsavefile)
    {
        sprintf(filename,"photo%d.bmp",get_filenum());
        retSD = f_mount(&spi_fs, "", 1);
        if(retSD != FR_OK)
        {
            if(retSD == FR_NO_FILESYSTEM)
            {
                //f_mount 没有文件系统,开始格式化spi-flash
                retSD = f_mkfs("",FM_FAT32,0,work,sizeof(work));
                if(retSD != FR_OK)
                {
                //f_mkfs 格式化失败
                    return 1;
                }
                else
                {
                    //格式化成功，开始重新挂载spi-flash
                    retSD = f_mount(&spi_fs, "", 1);
                    if(retSD != FR_OK)
                    {
                        return 1;
                        ;//f_mount 发生错误
                    }
                    else
                    {
                        save_bmp_header(&fil,&bmpinfo);
                        f_lseek(&fil,66);
                    }//spi-flash文件系统挂载成功
                }
            }
            else
            {
                return 1;
            }
        }
        else
        {//spi-flash文件系统挂载成功
            retSD = f_open(&fil, filename, FA_OPEN_ALWAYS | FA_WRITE);
            if(retSD == FR_OK)
            {
                save_bmp_header(&fil,&bmpinfo);
                f_lseek(&fil,66);
            }
            else
            {
                return 1;
            }
        }
    }
    ST7789_SetAddressWindow((320-IMG_WIDTH)/2, (240-IMG_HEIGHT)/2, IMG_WIDTH+(320-IMG_WIDTH)/2-1, IMG_HEIGHT+(240-IMG_HEIGHT)/2-1);
    ST7789_Select();
    ST7789_DC_Set();
    uint16_t imgdata[IMG_WIDTH*IMG_HEIGHT/4];
    uint32_t index=0;
    for (j = 0; j < ih; j++)
    {
//        float fy = ((float)j + 0.5f) * dy - 0.5f;
        float fy=(float)j  * dy+1 ;
        for (i = 0; i < iw; i++)
        {
//            float fx = ((float)i + 0.5f) * dx - 0.5f;
            float fx = (float)i  * dx+1 ;
            temp.gray_val=(uint8_t)arm_bilinear_interp_f32(&bilinear_data,fx,fy);
//            // 四邻域
//            int Lx = (int)fx;
//            int Rx = Lx + 1;
//            int Ly = (int)fy;
//            int Ry = Ly + 1;
//            Lx = Lx > w - 2 ? w - 2 : Lx < 0 ? 0 : Lx;
//            Rx = Rx > w - 1 ? w - 1 : Rx < 0 ? 0 : Rx;
//            Ly = Ly > h - 2 ? h - 2 : Ly < 0 ? 0 : Ly;
//            Ry = Ry > h - 1 ? h - 1 : Ry < 0 ? 0 : Ry;
//            float u = (float)Rx - fx;
//            float v = (float)Ry - fy;
//            float r1=u*(float)bmp8[Ly][Lx].gray_val+(1-u)*(float)bmp8[Ly][Rx].gray_val;
//            float r2=u*(float)bmp8[Ry][Lx].gray_val+(1-u)*(float)bmp8[Ry][Rx].gray_val;
//            temp.gray_val=(uint8_t)(v * r1 + (1 - v) * r2);
            bmp8_to_color(method,temp.gray_val,&temp_24);
            TEMP_RGB565.r_bit =temp_24.r_val>>3;
            TEMP_RGB565.g_bit =temp_24.g_val>>2;
            TEMP_RGB565.b_bit =temp_24.b_val>>3;
            color[0] =(TEMP_RGB565.r_bit << 11 |TEMP_RGB565.g_bit << 5 |TEMP_RGB565.b_bit);
            color[0] =( color[0] >> 8) | ((color[0] & 0xFF) << 8);
            if(index<IMG_WIDTH*(IMG_HEIGHT/4))
                imgdata[index++]=color[0];
            else
            {
                index=0;
                imgdata[index++]=color[0];
            }
        }
        if((j+1)%(IMG_HEIGHT/4)==0)
        {
            HAL_SPI_Transmit(hspi, (uint8_t*)imgdata, sizeof(uint16_t)*IMG_WIDTH*(IMG_HEIGHT/4), HAL_MAX_DELAY);
            if(ifsavefile)
            {
                for(int y=0;y<IMG_HEIGHT/4;y++)
                {
                    for(int x=0;x<IMG_WIDTH;x++)
                    {
                        imgdata[y*IMG_WIDTH+x] =( imgdata[y*IMG_WIDTH+x] >> 8) | ((imgdata[y*IMG_WIDTH+x] & 0xFF) << 8);
                    }
                }
                f_write(&fil,(uint8_t*)imgdata,sizeof(uint16_t)*IMG_WIDTH*(IMG_HEIGHT/4),&count);
               // UART_printf(&huart1,"count=%d\n",count);
            }
        }
    }
    ST7789_UnSelect();
    if(ifsavefile)
    {
        if(retSD != FR_OK)
        {
            f_close(&fil);
            return 1;
        }
        f_close(&fil);
    }
    return 0;
}


void gray_to_rgb(BMP_8 bmp8[],BMP_24 bmp24[][IMG_INITIAL_WIDTH],uint8_t method)
{
    for(int i=0;i<IMG_INITIAL_HEIGHT;i++)
    {
        for(int j=0;j<IMG_INITIAL_WIDTH;j++)
        {
            bmp8_to_color(method,bmp8[i*IMG_INITIAL_WIDTH+j].gray_val,&bmp24[i][j]);
        }
    }
}

void bmp8_to_color(uint8_t method,uint8_t gray_val,BMP_24*bmp24)
{
    switch(method)
    {
        case GCM_Pseudo1:
            bmp24->r_val=abs(0-gray_val);
            bmp24->g_val=abs(127-gray_val);
            bmp24->b_val=abs(255-gray_val);
            break;

        case GCM_Pseudo2:
            if( (gray_val>=0) && (gray_val<=63) )
            {
                bmp24->r_val=0;
                bmp24->g_val=0;
                bmp24->b_val=(gray_val*255/64);
            }
            else if( (gray_val>=64) && (gray_val<=127) )
            {
                bmp24->r_val=0;
                bmp24->g_val=((gray_val-64)*255/64);
                bmp24->b_val=((127-gray_val)*255/64);
            }
            else if( (gray_val>=128) && (gray_val<=191) )
            {
                bmp24->r_val=((gray_val-128)*255/64);
                bmp24->g_val=255;
                bmp24->b_val=0;
            }
            else if( (gray_val>=192) && (gray_val<=255) )
            {
                bmp24->r_val=255;
                bmp24->g_val=((255-gray_val)*255/64);
                bmp24->b_val=0;
            }
            break;

        case GCM_Metal1:
            if( (gray_val>=0) && (gray_val<=63) )
            {
                bmp24->r_val=0;
                bmp24->g_val=0;
                bmp24->b_val=(gray_val*255/64);
            }
            else if( (gray_val>=64) && (gray_val<=95) )
            {
                bmp24->r_val=((gray_val-63)*127/32);
                bmp24->g_val=((gray_val-63)*127/32);
                bmp24->b_val=255;
            }
            else if( (gray_val>=96) && (gray_val<=127) )
            {
                bmp24->r_val=((gray_val-95)*127/32)+128;
                bmp24->g_val=((gray_val-95)*127/32)+128;
                bmp24->b_val=((127-gray_val)*255/32);
            }
            else if( (gray_val>=128) && (gray_val<=191) )
            {
                bmp24->r_val=255;
                bmp24->g_val=255;
                bmp24->b_val=0;
            }
            else if( (gray_val>=192) && (gray_val<=255) )
            {
                bmp24->r_val=255;
                bmp24->g_val=255;
                bmp24->b_val=((gray_val-192)*255/64);
            }
            break;

        case GCM_Metal2:
            bmp24->r_val=0; bmp24->b_val=0; bmp24->b_val=0;
            if( (gray_val>=0) && (gray_val<=16) )
            {
                bmp24->r_val=0;
            }
            else if( (gray_val>=17) && (gray_val<=140) )
            {
                bmp24->r_val=((gray_val-16)*255/(140-16));
            }
            else if( (gray_val>=141) && (gray_val<=255) )
            {
                bmp24->r_val=255;
            }

            if( (gray_val>=0) && (gray_val<=101) )
            {
                bmp24->g_val=0;
            }
            else if( (gray_val>=102) && (gray_val<=218) )
            {
                bmp24->g_val=((gray_val-101)*255/(218-101));
            }
            else if( (gray_val>=219) && (gray_val<=255) )
            {
                bmp24->g_val=255;
            }

            if( (gray_val>=0) && (gray_val<=91) )
            {
                bmp24->b_val=28+((gray_val-0)*100/(91-0));
            }
            else if( (gray_val>=92) && (gray_val<=120) )
            {
                bmp24->b_val=((120-gray_val)*128/(120-91));
            }
            else if( (gray_val>=129) && (gray_val<=214) )
            {
                bmp24->b_val=0;
            }
            else if( (gray_val>=215) && (gray_val<=255) )
            {
                bmp24->b_val=((gray_val-214)*255/(255-214));
            }
            break;

        case GCM_Rainbow1:
            if( (gray_val>=0) && (gray_val<=31) )
            {
                bmp24->r_val=0;
                bmp24->g_val=0;
                bmp24->b_val=(gray_val*255/32);
            }
            else if( (gray_val>=32) && (gray_val<=63) )
            {
                bmp24->r_val=0;
                bmp24->g_val=((gray_val-32)*255/32);
                bmp24->b_val=255;
            }
            else if( (gray_val>=64) && (gray_val<=95) )
            {
                bmp24->r_val=0;
                bmp24->g_val=255;
                bmp24->b_val=((95-gray_val)*255/32);
            }
            else if( (gray_val>=96) && (gray_val<=127) )
            {
                bmp24->r_val=((gray_val-96)*255/32);
                bmp24->g_val=255;
                bmp24->b_val=0;
            }
            else if( (gray_val>=128) && (gray_val<=191) )
            {
                bmp24->r_val=255;
                bmp24->g_val=((191-gray_val)*255/64);
                bmp24->b_val=0;
            }
            else if( (gray_val>=192) && (gray_val<=255) )
            {
                bmp24->r_val=255;
                bmp24->g_val=((gray_val-192)*255/64);//0
                bmp24->b_val=((gray_val-192)*255/64);
            }
            break;

        case GCM_Rainbow2:
            if( (gray_val>=0) && (gray_val<=63) )
            {
                bmp24->r_val=0;
                bmp24->g_val=((gray_val-0)*255/64);
                bmp24->b_val=255;
            }
            else if( (gray_val>=64) && (gray_val<=95) )
            {
                bmp24->r_val=0;
                bmp24->g_val=255;
                bmp24->b_val=((95-gray_val)*255/32);
            }
            else if( (gray_val>=96) && (gray_val<=127) )
            {
                bmp24->r_val=((gray_val-96)*255/32);
                bmp24->g_val=255;
                bmp24->b_val=0;
            }
            else if( (gray_val>=128) && (gray_val<=191) )
            {
                bmp24->r_val=255;
                bmp24->g_val=((191-gray_val)*255/64);
                bmp24->b_val=0;
            }
            else if( (gray_val>=192) && (gray_val<=255) )
            {
                bmp24->r_val=255;
                bmp24->g_val=((gray_val-192)*255/64);
                bmp24->b_val=((gray_val-192)*255/64);
            }
            break;

        case GCM_Rainbow3:
            if( (gray_val>=0) && (gray_val<=51) )
            {
                bmp24->r_val=0;
                bmp24->g_val=gray_val*5;
                bmp24->b_val=255;
            }
            else if( (gray_val>=52) && (gray_val<=102) )
            {
                bmp24->r_val=0;
                bmp24->g_val=255;
                bmp24->b_val=255-(gray_val-51)*5;
            }
            else if( (gray_val>=103) && (gray_val<=153) )
            {
                bmp24->r_val=(gray_val-102)*5;
                bmp24->g_val=255;
                bmp24->b_val=0;
            }
            else if( (gray_val>=154) && (gray_val<=204) )
            {
                bmp24->r_val=255;
                bmp24->g_val=(255-128*(gray_val-153)/51);
                bmp24->b_val=0;
            }
            else if( (gray_val>=205) && (gray_val<=255) )
            {
                bmp24->r_val=255;
                bmp24->g_val=(127-127*(gray_val-204)/51);
                bmp24->b_val=0;
            }
            break;

        case GCM_Zhou:
            if( (gray_val>=0) && (gray_val<=63) )
            {
                bmp24->r_val=0;
                bmp24->g_val=((64-gray_val)*255/64);
                bmp24->b_val=255;
            }
            else if( (gray_val>=64) && (gray_val<=127) )
            {
                bmp24->r_val=0;
                bmp24->g_val=((gray_val-64)*255/64);
                bmp24->b_val=((127-gray_val)*255/64);
            }
            else if( (gray_val>=128) && (gray_val<=191) )
            {
                bmp24->r_val=((gray_val-128)*255/64);
                bmp24->g_val=255;
                bmp24->b_val=0;
            }
            else if( (gray_val>=192) && (gray_val<=255) )
            {
                bmp24->r_val=255;
                bmp24->g_val=((255-gray_val)*255/64);
                bmp24->b_val=0;
            }
            break;

        case GCM_Ning:
            if ((gray_val>=0) && (gray_val<=63))
            {
                bmp24->r_val=0;
                bmp24->g_val=254-4*gray_val;
                bmp24->b_val=255;
            }
            else if ((gray_val>=64) && (gray_val<=127))
            {
                bmp24->r_val=0;
                bmp24->g_val=4*gray_val-254;
                bmp24->b_val=510-4*gray_val;
            }
            else if ((gray_val>=128) && (gray_val<=191))
            {
                bmp24->r_val=4*gray_val-510;
                bmp24->g_val=255;
                bmp24->b_val=0;
            }
            else if ((gray_val>=192) && (gray_val<=255))
            {
                bmp24->r_val=255;
                bmp24->g_val=1022-4*gray_val;
                bmp24->b_val=0;
            }
            break;

        case GCM_Gray:
            bmp24->r_val=gray_val;
            bmp24->g_val=gray_val;
            bmp24->b_val=gray_val;
            break;

        default:
            break;
    }
}

void draw_spectrum(uint8_t method)
{
    uint16_t color;
    for(int i=255;i>0;i-=2)
    {
        BMP_24 temp24;
        RGB565 temp565;
        bmp8_to_color(method,i,&temp24);
        temp565.r_bit =temp24.r_val>>3;
        temp565.g_bit =temp24.g_val>>2;
        temp565.b_bit =temp24.b_val>>3;
        color =(temp565.r_bit << 11 |temp565.g_bit << 5 |temp565.b_bit);
        ST7789_DrawFilledRectangle(310,228-((int)((i/2)*1.6875))-2,8,2,color);

    }
}
void bmp_header_init(BMP_INFO *bmp_info)
{
    bmp_info->bitmapfileheader.bfType = ((uint16_t)'M'<<8)+'B';			//bmp类型  "BM"
    bmp_info->bitmapfileheader.bfSize= 54+12 + IMG_HEIGHT*IMG_WIDTH*2;	//文件大小（信息结构体+像素数据）
    bmp_info->bitmapfileheader.bfReserved1 = 0x0000;		//保留，必须为0
    bmp_info->bitmapfileheader.bfReserved2 = 0x0000;
    bmp_info->bitmapfileheader.bfOffBits=54+12;				//位图信息结构体所占的字节数

    //填写位图信息头信息
    bmp_info->bmpInfoheader.biSize=sizeof(BMP_INFOHEADER);  			//位图信息头的大小
    bmp_info->bmpInfoheader.biWidth=IMG_WIDTH;  		//位图的宽度
    bmp_info->bmpInfoheader.biHeight=-1*IMG_HEIGHT;  		//图像的高度
    bmp_info->bmpInfoheader.biPlanes=1;  			//目标设别的级别，必须是1
    bmp_info->bmpInfoheader.biBitCount=16;        //每像素位数
    bmp_info->bmpInfoheader.biCompression=3;  	//RGB555格式
    bmp_info->bmpInfoheader.biSizeImage=IMG_WIDTH*IMG_WIDTH*2;//实际位图所占用的字节数（仅考虑位图像素数据）
    bmp_info->bmpInfoheader.biXPelsPerMeter=0;	//水平分辨率
    bmp_info->bmpInfoheader.biYPelsPerMeter=0; 	//垂直分辨率
    bmp_info->bmpInfoheader.biClrImportant=0;   	//说明图像显示有重要影响的颜色索引数目，0代表所有的颜色一样重要
    bmp_info->bmpInfoheader.biClrUsed=0;  		//位图实际使用的彩色表中的颜色索引数，0表示使用所有的调色板项

//    //RGB565格式掩码
    bmp_info->RGB_MASK[0]=0X00F800;	 		//红色掩码
    bmp_info->RGB_MASK[1]=0X0007E0;	 		//绿色掩码
    bmp_info->RGB_MASK[2]=0X00001F;	 		//蓝色掩码
}


void save_bmp_header(FIL *file,BMP_INFO *bmp_info)
{
    f_write(file, (uint8_t*)&bmp_info->bitmapfileheader.bfType, sizeof(bmp_info->bitmapfileheader.bfType),&count);
    f_write(file, (uint8_t*)&bmp_info->bitmapfileheader.bfSize, sizeof(bmp_info->bitmapfileheader.bfSize),&count);
    f_write(file, (uint8_t*)&bmp_info->bitmapfileheader.bfReserved1, sizeof(bmp_info->bitmapfileheader.bfReserved1),&count);
    f_write(file, (uint8_t*)&bmp_info->bitmapfileheader.bfReserved2, sizeof(bmp_info->bitmapfileheader.bfReserved2),&count);
    f_write(file,(uint8_t*)&bmp_info->bitmapfileheader.bfOffBits, sizeof(bmp_info->bitmapfileheader.bfOffBits),&count);
    f_write(file, (uint8_t*)&bmp_info->bmpInfoheader, sizeof(bmp_info->bmpInfoheader),&count);
    f_write(file, (uint8_t*)&bmp_info->RGB_MASK, sizeof(bmp_info->RGB_MASK),&count);
    f_lseek(file,66);
}
uint8_t get_filenum()
{
    uint8_t num;
            retSD = f_mount(&spi_fs, "", 1);
    if (retSD != FR_OK)
    {
        if (retSD == FR_NO_FILESYSTEM)
        {
            retSD = f_mkfs("", FM_FAT32, 0, work, sizeof(work));
            retSD = f_mount(&spi_fs, "", 1);
        }
    }
/*----------------------- 文件系统测试：写测试 -----------------------------*/
    retSD = f_open(&fil, "info.bin", FA_OPEN_ALWAYS | FA_READ);

    retSD = f_read(&fil, &num, 1, &count);
    f_close(&fil);

/*------------------- 文件系统测试：读测试 ------------------------------------*/
    num++;
    retSD = f_open(&fil, "info.bin", FA_OPEN_EXISTING | FA_WRITE);
    retSD = f_write(&fil, &num,1,&count);
    f_close(&fil);
    return num;
}

