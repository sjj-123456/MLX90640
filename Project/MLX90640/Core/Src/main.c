/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mlx90640_user.h"
#include "user.h"
#include "bmp.h"
#include "st7789.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
paramsMLX90640 mlx90640;
static float mlx90640To[768];
BMP_8 bmp8[768];
float max_temp;
float min_temp;
float center_temp;
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint32_t voltage_adc[1];
float bat_voltage;
extern BMP_INFO bmpinfo;
void test();
ENCODER my_encoder;
uint8_t img_method_arr[4]={GCM_Pseudo2,GCM_Pseudo1,GCM_Metal2,GCM_Gray};
MENU my_menu={
        {"GCM_Pseudo2   ","GCM_Pseudo1   ","GCM_Metal2   ","GCM_Gray   "},
        0,
        {"Nearest  ","Bilinear  "},
        1,
        {"screenshot","saving    ","success   ","fail      "},
        0
};
uint8_t key1_press,key2_press,button_press;
//menu1 bin_size or resize
//menu2 color_method
//menu3 screenshot
uint8_t chose_menu_index=1;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_FATFS_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */
    HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_1);
    ST7789_SetBackLight(0);
    bmp_header_init(&bmpinfo);
    HAL_TIM_Base_Start_IT(&htim4);
    HAL_TIM_Encoder_Start(&htim2,TIM_CHANNEL_ALL);
    HAL_ADC_Start_DMA(&hadc1,voltage_adc,1);
    ST7789_Init();
    mlx90640_init(FPS8HZ,&mlx90640);
    HAL_Delay(500);
    mlx90640_get_temp_mat(&mlx90640,mlx90640To);
    ST7789_SetBackLight(100);
    draw_spectrum(img_method_arr[my_menu.chose_color_method_index]);
    //test();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      UART_printf(&huart1,"%d,%d,%d\n",chose_menu_index,key1_press,key2_press);
    if(key1_press==1)
        chose_menu_index--;
    if(key2_press)
        chose_menu_index++;
    if(chose_menu_index<1)
        chose_menu_index=3;
    if(chose_menu_index>3)
        chose_menu_index=1;
      mlx90640_get_temp_mat(&mlx90640,mlx90640To);
      while(temp_to_bmp8(bmp8,mlx90640To,768,&max_temp,&min_temp,1))
      {//温度异常重新获取
          mlx90640_get_temp_mat(&mlx90640,mlx90640To);
      }
      center_temp=mlx90640To[768/2];
      if(button_press)
          my_menu.screenshot_index=1;
      ST7789_WriteString(0,0,my_menu.voltage_info,Font_7x10,WHITE,BLACK);
      switch (chose_menu_index) {
          case 1:
              ST7789_WriteString(55,0,my_menu.resize_or_binsize[my_menu.resize_or_binsize_index],Font_7x10,RED,BLACK);
              ST7789_WriteString(135,0,my_menu.color_method[my_menu.chose_color_method_index],Font_7x10,WHITE,BLACK);
              ST7789_WriteString(230,0,my_menu.screenshot[my_menu.screenshot_index],Font_7x10,WHITE,BLACK);
              break;
          case 2:
              ST7789_WriteString(55,0,my_menu.resize_or_binsize[my_menu.resize_or_binsize_index],Font_7x10,WHITE,BLACK);
              ST7789_WriteString(135,0,my_menu.color_method[my_menu.chose_color_method_index],Font_7x10,RED,BLACK);
              ST7789_WriteString(230,0,my_menu.screenshot[my_menu.screenshot_index],Font_7x10,WHITE,BLACK);
              draw_spectrum(img_method_arr[my_menu.chose_color_method_index]);
              break;
          case 3:
              ST7789_WriteString(55,0,my_menu.resize_or_binsize[my_menu.resize_or_binsize_index],Font_7x10,WHITE,BLACK);
              ST7789_WriteString(135,0,my_menu.color_method[my_menu.chose_color_method_index],Font_7x10,WHITE,BLACK);
              ST7789_WriteString(230,0,my_menu.screenshot[my_menu.screenshot_index],Font_7x10,RED,BLACK);
              break;
      }
      ST7789_WriteString(0,229,my_menu.min_temp_info,Font_7x10,BLUE,BLACK);
      ST7789_WriteString(110,229,my_menu.center_temp_info,Font_7x10,YELLOW,BLACK);
      ST7789_WriteString(230,229,my_menu.max_temp_info,Font_7x10,RED,BLACK);

      if(!my_menu.resize_or_binsize_index)
      {
          Resize_draw(bmp8,img_method_arr[my_menu.chose_color_method_index]);
      }
      else
      {
          if(button_press)
          {
              if(Besize_draw(bmp8,img_method_arr[my_menu.chose_color_method_index],savefile))
                  my_menu.screenshot_index=3;
              else
                  my_menu.screenshot_index=2;
          }
          else
              Besize_draw(bmp8,img_method_arr[my_menu.chose_color_method_index],notsavefile);
      }
      HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_5);
     // UART_printf(&huart1,"%d,%d,%d\n",chose_menu_index,key1_press,key2_press);

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==htim4.Instance)
    {
        my_encoder.cnt=TIM2->CNT;
        if(abs(my_encoder.cnt-my_encoder.last_cnt)>2)
        {
            my_encoder.direction=my_encoder.cnt-my_encoder.last_cnt>0?1:-1;
            my_encoder.last_cnt=my_encoder.cnt;
        }
        else
        {
            my_encoder.direction=0;
        }
        switch (chose_menu_index) {
            case 1:
                my_menu.resize_or_binsize_index+=my_encoder.direction;
                if(my_menu.resize_or_binsize_index>1)
                    my_menu.resize_or_binsize_index=0;
                if(my_menu.resize_or_binsize_index<0)
                    my_menu.resize_or_binsize_index=1;
                button_press=0;
                my_menu.screenshot_index=0;
                break;
            case 2:
                my_menu.chose_color_method_index+=my_encoder.direction;
                if(my_menu.chose_color_method_index>3)
                    my_menu.chose_color_method_index=0;
                if(my_menu.chose_color_method_index<0)
                    my_menu.chose_color_method_index=3;
                button_press=0;
                my_menu.screenshot_index=0;
                break;
            case 3:
                button_press= HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13)==0?1:0;
                break;
        }
        key1_press=HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_12)==0?1:0;
        key2_press=HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_13)==0?1:0;
        //UART_printf(&huart1,"a=%d,b=%d\n", HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_12), HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_13));
        bat_voltage=(float)voltage_adc[0]*3.3f/4096.0f/160.0f*630.0f;
        sprintf(my_menu.voltage_info,"%.2fv",bat_voltage);
        sprintf(my_menu.min_temp_info,"min=%.2f  ",min_temp);
        sprintf(my_menu.max_temp_info,"max=%.2f  ",max_temp);
        sprintf(my_menu.center_temp_info,"center=%.2f  ",center_temp);
    }
}



extern FATFS spi_fs;
extern FIL fil;
extern FRESULT retSD;
extern unsigned int count;
extern unsigned char work[4096];
unsigned char read_buf[50] = {0};
unsigned char write_buf[50] ="hello world\r\n";

//文件读写测试
void test()
{
    UART_printf(&huart1,"\r\n ****** FatFs Example ******\r\n\r\n");
/*##-1- Register the file system object to the FatFs module ##############*/
    retSD = f_mount(&spi_fs, "", 1);
    if(retSD != FR_OK)
    {
        if(retSD == FR_NO_FILESYSTEM)
        {
            UART_printf(&huart1,"F_mount no file system, start formatting spiflash\r\n");//f_mount 没有文件系统,开始格式化spi-flash
            retSD = f_mkfs("",FM_FAT32,0,work,sizeof(work));
            if(retSD != FR_OK)
            {
                UART_printf(&huart1,"F_mkfs formatting failed，err = %d\r\n",
                            retSD);//f_mkfs 格式化失败
                while(1);
            }
            else
            {
                UART_printf(&huart1,"Formatting successfully, start re-mountingthe spi-flash\r\n");//格式化成功，开始重新挂载spi-flash
                retSD = f_mount(&spi_fs, "", 1);
                if(retSD != FR_OK)
                {
                    UART_printf(&huart1,"An error occurred in f_mount,err =%d\r\n", retSD);//f_mount 发生错误
                }
                else UART_printf(&huart1,"spi-flash File system mountedsuccessfully\r\n");//spi-flash文件系统挂载成功
            }
        }
        else
        {
            UART_printf(&huart1,"Another error occurred with f_moun,err =%d\r\n", retSD);//f_mount 发生其他错误
            while(1);
        }
    }
    else UART_printf(&huart1,"spi-flash File system mountedsuccessfully\r\n");//spi-flash文件系统挂载成功
/*----------------------- 文件系统测试：写测试 -----------------------------*/
    UART_printf(&huart1,"\r\n****** File write tests are about to begin...******\r\n");//即将进行文件写入测试
    retSD = f_open(&fil, "test.bmp", FA_OPEN_ALWAYS | FA_WRITE);
    if(retSD == FR_OK)
    {
        UART_printf(&huart1,"Open/create xblnb.txt successfully, write data tofile.\r\n");//打开/创建xblnb.txt文件成功，向文件写入数据。

        f_write(&fil, (uint8_t*)&bmpinfo.bitmapfileheader.bfType, sizeof(bmpinfo.bitmapfileheader.bfType),&count);
        UART_printf(&huart1,"count=%d\n",count);
        f_write(&fil, (uint8_t*)&bmpinfo.bitmapfileheader.bfSize, sizeof(bmpinfo.bitmapfileheader.bfSize),&count);
        UART_printf(&huart1,"count=%d\n",count);
        f_write(&fil, (uint8_t*)&bmpinfo.bitmapfileheader.bfReserved1, sizeof(bmpinfo.bitmapfileheader.bfReserved1),&count);
        UART_printf(&huart1,"count=%d\n",count);
        f_write(&fil, (uint8_t*)&bmpinfo.bitmapfileheader.bfReserved2, sizeof(bmpinfo.bitmapfileheader.bfReserved2),&count);
        UART_printf(&huart1,"count=%d\n",count);
        f_write(&fil, (uint8_t*)&bmpinfo.bitmapfileheader.bfOffBits, sizeof(bmpinfo.bitmapfileheader.bfOffBits),&count);
        UART_printf(&huart1,"count=%d\n",count);
        f_write(&fil, (uint8_t*)&bmpinfo.bmpInfoheader, sizeof(bmpinfo.bmpInfoheader),&count);
        UART_printf(&huart1,"count=%d\n",count);
        f_write(&fil, (uint8_t*)&bmpinfo.RGB_MASK, sizeof(bmpinfo.RGB_MASK),&count);
//        UART_printf(&huart1,"count=%d\n",count);
        f_lseek(&fil,66);
        HAL_Delay(1000);
        for(int i=0;i<240;i++)
        {
            for(int j=0;j<320;j++)
            {
                uint16_t color=RED;
                retSD=f_write(&fil, (uint8_t*)&color, sizeof(color), &count);
                UART_printf(&huart1,"count=%d\n",count);
            }
        }

        if(retSD != FR_OK)
        {
            UART_printf(&huart1,"An error occurred in f_write,err = %d\r\n",
                        retSD);//f_write 发生错误
            UART_printf(&huart1,"close xblnb.txt\r\n");//关闭打开的xblnb.txt文件
            count = 0;
            f_close(&fil);
        }
        else
        {
            UART_printf(&huart1,"File written successfully, write bytedata:%d\n", count);//文件写入成功，写入字节数据：
            UART_printf(&huart1,"The data written to the file is:\r\n%s\r\n",write_buf);//向文件写入的数据为：
            UART_printf(&huart1,"close xblnb.txt\r\n");//关闭打开的xblnb.txt文件
            count = 0;
            f_close(&fil);
        }
    }
    else UART_printf(&huart1,"open/set up xblnb.txt failed,err = %d\r\n",
                     retSD);//打开/创建xblnb.txt文件失败

}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
