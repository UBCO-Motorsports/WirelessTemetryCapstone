/*
 * UJA1075.c
 *
 *  Created on: Oct. 14, 2021
 *      Author: lvoze
 */

#include "UJA1075.h"

//This defines the WD configuration set on both startup and when feeding watchdog
const uint8_t WD_SETUP = (UJA_REG_WDGANDSTATUS << 5) | (UJA_RO_RW << 4) | (UJA_WMC_WND << 3) | (UJA_NWP_1024);

void UJA1075_Init(UJA1075_Handle * handle)
{
	uint8_t txdata[2];

	//Force SBC in standby mode
	txdata[0] = (UJA_REG_MODECONTROL << 5) | (UJA_RO_RW << 4) | (UJA_MC_STBY << 2);
	txdata[1] = 0;
	UJA1075_Write(handle, txdata);

	//Setup WDG and Status register
	txdata[0] = WD_SETUP;
	txdata[1] = 0;
	UJA1075_Write(handle, txdata);

	//Set normal mode and enable CAN voltage
	txdata[0] = (UJA_REG_MODECONTROL << 5) | (UJA_RO_RW << 4) | (UJA_MC_V2ON << 2);
	txdata[1] = 0;
	UJA1075_Write(handle, txdata);
}

void UJA1075_FeedWD(UJA1075_Handle * handle)
{
	uint8_t txdata[2];
	txdata[0] = WD_SETUP;
	txdata[1] = 0;
	UJA1075_Write(handle, txdata);
}

void UJA1075_Write(UJA1075_Handle * handle, uint8_t * txdata)
{
	HAL_GPIO_WritePin(handle->ChipSelect_GPIO_Port, handle->ChipSelect_GPIO_Pin, 0);
	HAL_SPI_Transmit(handle->SPI_Handle, txdata, 2, 100);
	HAL_GPIO_WritePin(handle->ChipSelect_GPIO_Port, handle->ChipSelect_GPIO_Pin, 1);
}
