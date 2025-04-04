/*
 * MS5837.c
 *
 *  Created on: 1 Jan 2022
 *      Author: Ali Eren Mercan
 *      Author: Furkan Kirlangic
 */

#include "MS5837.h"

_MS5837 MS5837;

typedef struct{
	uint8_t Command;
	uint8_t *LocalRegAddr;
	uint8_t Length;
}_MS5837_I2C_TX_IT_ParameterCarry;
_MS5837_I2C_TX_IT_ParameterCarry MS5837_I2C_TX_IT_ParameterCarry;

void MS5837_I2C_TransmitCallbackForReceiving(){
	HAL_I2C_RegisterCallback(MS5837_I2C_PORT, HAL_I2C_MASTER_TX_COMPLETE_CB_ID, BAR30_TX_CALLBACK);
	HAL_I2C_RegisterCallback(MS5837_I2C_PORT, HAL_I2C_MASTER_RX_COMPLETE_CB_ID, BAR30_RX_CALLBACK);
	HAL_I2C_Master_Receive_IT(MS5837_I2C_PORT, MS5837_DEV_ADDR<<1, MS5837_I2C_TX_IT_ParameterCarry.LocalRegAddr, MS5837_I2C_TX_IT_ParameterCarry.Length);
}

void MS5837_BusRead(uint8_t Command, uint8_t *LocalRegAddr, uint8_t Length){
	MS5837_I2C_TX_IT_ParameterCarry.Length = Length;
	MS5837_I2C_TX_IT_ParameterCarry.LocalRegAddr = LocalRegAddr;
	MS5837_I2C_TX_IT_ParameterCarry.Command = Command;
	HAL_I2C_RegisterCallback(MS5837_I2C_PORT, HAL_I2C_MASTER_TX_COMPLETE_CB_ID, MS5837_I2C_TransmitCallbackForReceiving);
	HAL_I2C_Master_Transmit_IT(MS5837_I2C_PORT, MS5837_DEV_ADDR<<1, &MS5837_I2C_TX_IT_ParameterCarry.Command, 1);
}

void MS5837_MemoryRead(uint8_t Command, uint8_t *LocalRegAddr, uint8_t Length){
	HAL_I2C_RegisterCallback(MS5837_I2C_PORT, HAL_I2C_MEM_RX_COMPLETE_CB_ID, BAR30_READ_CMP_CALLBACK);
	HAL_I2C_Mem_Read_IT(MS5837_I2C_PORT, MS5837_DEV_ADDR<<1, Command, I2C_MEMADD_SIZE_8BIT, LocalRegAddr, Length);
}

void MS5837_BusWriteForInit(uint8_t Command){
	static uint8_t BusBuffer;
	BusBuffer = Command;
	HAL_I2C_RegisterCallback(MS5837_I2C_PORT, HAL_I2C_MASTER_TX_COMPLETE_CB_ID, BAR30_TX_CALLBACK);
	HAL_I2C_Master_Transmit_IT(MS5837_I2C_PORT, MS5837_DEV_ADDR<<1, &BusBuffer, 1);
}

void MS5837_BusWriteForConvertion(uint8_t Command){
	static uint8_t BusBuffer;
	BusBuffer = Command;
	HAL_I2C_RegisterCallback(MS5837_I2C_PORT, HAL_I2C_MASTER_TX_COMPLETE_CB_ID, BAR30_CONV_CMP_CALLBACK);
	HAL_I2C_Master_Transmit_IT(MS5837_I2C_PORT, MS5837_DEV_ADDR<<1, &BusBuffer, 1);
}

void MS5837_AbortI2C_IT(){
	HAL_I2C_Master_Abort_IT(MS5837_I2C_PORT, MS5837_DEV_ADDR<<1);
	HAL_I2C_DeInit(MS5837_I2C_PORT);
	HAL_I2C_Init(MS5837_I2C_PORT);
}

void MS5837_TypedefInit(_MS5837 **MS5837_AddrOnTopLayer){
	*MS5837_AddrOnTopLayer = &MS5837;
	BAR30_TX_CALLBACK = NULL;
	BAR30_RX_CALLBACK = NULL;
	MS5837_I2C_PORT = NULL;
	MS5837_DEV_ADDR = MS5837_I2C_ADDR;
	LAST_CONVERSION = CONVERSION_IDLE;
}

uint8_t MS5837_CRC4(uint16_t n_prom[]) {
	uint16_t n_rem = 0;
	n_prom[0] = ((n_prom[0]) & 0x0FFF);
	n_prom[7] = 0;

	for ( uint8_t i = 0 ; i < 16; i++ ) {
		if ( i%2 == 1 ) {
			n_rem ^= (uint16_t)((n_prom[i>>1]) & 0x00FF);
		} else {
			n_rem ^= (uint16_t)(n_prom[i>>1] >> 8);
		}
		for ( uint8_t n_bit = 8 ; n_bit > 0 ; n_bit-- ) {
			if ( n_rem & 0x8000 ) {
				n_rem = (n_rem << 1) ^ 0x3000;
			} else {
				n_rem = (n_rem << 1);
			}
		}
	}

	n_rem = ((n_rem >> 12) & 0x000F);
	return n_rem ^ 0x00;
}

uint8_t CheckTheCRC(){
	uint8_t err = 0;
	uint8_t crcRead;
	uint16_t tmpProm[PROM_SIZE];

	for(uint8_t i = 0; i < PROM_SIZE; i++){
		tmpProm[i] = C_BEGIN_ADDR[i];
	}

	crcRead = tmpProm[0] >> 12;
	uint8_t crcCalculated = MS5837_CRC4(tmpProm);
	if ( crcCalculated == crcRead ) {
				err = 1;
	}
	return err;
}

void LSBtoMSB_C_Values(){
	for(uint8_t i = 0; i < PROM_SIZE; i++){
		C_BEGIN_ADDR[i] = (uint16_t)((uint16_t)(C_BEGIN_ADDR[i] << 8) | (uint8_t)(C_BEGIN_ADDR[i] >> 8));
	}
}

void MS5837_PROM_Read(_PROM_Index PROM_Index){
	MS5837_BusRead(MS5837_PROM_READ | (PROM_Index << 1), (uint8_t*)(C_BEGIN_ADDR + PROM_Index), 2);
}

void MS5837_Reset(){
	MS5837_BusWriteForInit(MS5837_RESET_CMD);
}


void ConvertD1(){
	MS5837_BusWriteForConvertion(MS5837_CONVERT_D1_8192);
	LAST_CONVERSION = D1_CONVERSION_PROCESS;
}

void ConvertD2(){
	MS5837_BusWriteForConvertion(MS5837_CONVERT_D2_8192);
	LAST_CONVERSION = D2_CONVERSION_PROCESS;
}

void MS5837_GetCurrentADC(){

	if(LAST_CONVERSION == D1_CONVERSION_PROCESS){
		MS5837_MemoryRead(MS5837_ADC_READ, D1_BUFF, 3);
	}
	else if(LAST_CONVERSION == D2_CONVERSION_PROCESS){
		MS5837_MemoryRead(MS5837_ADC_READ, D2_BUFF, 3);
	}
	else{
	}
}

void MS5837_ReadDataRawToProcess(){
	if(LAST_CONVERSION == D1_CONVERSION_PROCESS){
		LSBtoMSB_D1();
	}
	else if(LAST_CONVERSION == D2_CONVERSION_PROCESS){
		LSBtoMSB_D2();
	}
	else{

	}
}

void LSBtoMSB_D1(){
	D1 = D1_BUFF[2] | (D1_BUFF[1] << 8) | (D1_BUFF[0] << 16);
}

void LSBtoMSB_D2(){
	D2 = D2_BUFF[2] | (D2_BUFF[1] << 8) | (D2_BUFF[0] << 16);
}

void MS5837_30BA_Calc_PoweredByAEM(){
	MS5837_VALUES.dT = D2 - (uint32_t)(T_REF << 8);
	MS5837_VALUES.OFF = (uint32_t)(OFF_T1 << 16)  + ((uint32_t)(TCO * MS5837_VALUES.dT) >> 7);
	MS5837_VALUES.SENS = (uint32_t)(SENS_T1 << 15) + ((uint32_t)(TCS * MS5837_VALUES.dT) >> 8);
	PRESSURE = (((D1 * MS5837_VALUES.SENS) >> 21) - MS5837_VALUES.OFF) >> 13;
	PRESSURE = PRESSURE / 10;
	TEMPERATURE = 2000 + ((MS5837_VALUES.dT * TEMPSENS) >> 23);
	if(TEMPERATURE < 2000){
		MS5837_VALUES.Ti = (int64_t)(3 * (MS5837_VALUES.dT*MS5837_VALUES.dT)) >> 33;
		int32_t powOfTempMinus2000 = (TEMPERATURE - 2000)*(TEMPERATURE - 2000);
		MS5837_VALUES.OFFi = (3 * powOfTempMinus2000) >> 1;
		MS5837_VALUES.SENSi = (5 * powOfTempMinus2000) >> 3;
		if(TEMPERATURE < -1500){
			int32_t powOfTempPlus1500 = (TEMPERATURE + 1500)*(TEMPERATURE + 1500);
			MS5837_VALUES.OFFi += 7 * (powOfTempPlus1500);
			MS5837_VALUES.SENSi += 4 * (powOfTempPlus1500);
		}
	}
	else{
		MS5837_VALUES.Ti = (int64_t)(MS5837_VALUES.dT*MS5837_VALUES.dT) >> 36;
		MS5837_VALUES.OFFi = ((TEMPERATURE - 2000)*(TEMPERATURE - 2000)) >> 4;
		MS5837_VALUES.SENSi = 0;
	}

	MS5837_VALUES.OFF2 = MS5837_VALUES.OFF - MS5837_VALUES.OFFi;
	MS5837_VALUES.SENS2 = MS5837_VALUES.SENS - MS5837_VALUES.SENSi;

	TEMPERATURE = (TEMPERATURE - MS5837_VALUES.Ti);
	PRESSURE = (((D1 * MS5837_VALUES.SENS2) >> 21) - MS5837_VALUES.OFF2) >> 13;
	PRESSURE = PRESSURE / 10;
}
