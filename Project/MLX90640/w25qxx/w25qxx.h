#ifndef __BSP_W25QXX_H
#define __BSP_W25QXX_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "spi.h"

#include "main.h"
#define PRINTF_DEBUG    printf
#define W25Q80 	0XEF13 	
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
#define W25Q128	0XEF17
  
//W25Q128一共是16M，这里只分出来了12M使用，是为了后续的测试USB读卡器做准备
#define W25Q128FV_FLASH_SIZE                  (1024*1024*12)    //定义了用户使用的空间大小
#define W25Q128FV_SECTOR_SIZE                 65536       		//一个块的大小是64KB*1024 = 65536byte
#define W25Q128FV_SUBSECTOR_SIZE              4096              //一个扇区的大小是4KB*1024= 4096byte
#define W25Q128FV_PAGE_SIZE                   256               //单次写入最多字节数


/** 
  * @brief  W25Q128FV Commands  
  */  
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 

#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 

//这个地方的片选信号，是最后一个自定义的片选IO的引脚
#define W25Qx_Enable() 			HAL_GPIO_WritePin(W25Q_CS_GPIO_Port, W25Q_CS_Pin, GPIO_PIN_RESET)
#define W25Qx_Disable() 		HAL_GPIO_WritePin(W25Q_CS_GPIO_Port, W25Q_CS_Pin, GPIO_PIN_SET)


void BSP_W25QXX_Init(void);

uint16_t  BSP_W25QXX_ReadID(void);  	    		//读取FLASH ID
uint8_t	 BSP_W25QXX_ReadSR(void);        		//读取状态寄存器 

void BSP_W25QXX_Write_SR(uint8_t sr);  	 	//写状态寄存器
void BSP_W25QXX_Write_Enable(void);  		//写使能 
void BSP_W25QXX_Write_Disable(void);		//写保护

void BSP_W25QXX_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);
void BSP_W25QXX_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead);   //读取flash
void BSP_W25QXX_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);//写入flash

void BSP_W25QXX_Erase_Chip(void);    	  	//整片擦除
void BSP_W25QXX_Erase_Sector(uint32_t Dst_Addr);	//扇区擦除

void BSP_W25QXX_Wait_Busy(void);           	//等待空闲
void BSP_W25QXX_PowerDown(void);        	//进入掉电模式

void BSP_W25QXX_WAKEUP(void);				//唤醒
#ifdef __cplusplus
}
#endif
#endif /* __W25Qx_H */