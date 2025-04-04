/*
 * MS5837.h
 *
 *  Created on: 1 Jan 2022
 *      Author: Ali Eren Mercan
 *      Author: Furkan Kirlangic
 */

#ifndef MS5837_H_
#define MS5837_H_

#include "main.h"

#define     MS5837_I2C_ADDR         0x76


#define PROM_SIZE					7


#define MS5837_RESET_CMD            0x1E
#define MS5837_ADC_READ             0x00
#define MS5837_PROM_READ            0xA0
#define MS5837_PROM_C0_READ         0xA0
#define MS5837_PROM_C1_READ         0xA2
#define MS5837_PROM_C2_READ         0xA4
#define MS5837_PROM_C3_READ         0xA6
#define MS5837_PROM_C4_READ         0xA8
#define MS5837_PROM_C5_READ         0xAA
#define MS5837_PROM_C6_READ         0xAC
#define MS5837_CONVERT_D1_8192      0x4A
#define MS5837_CONVERT_D2_8192      0x5A


typedef enum{
    PROM_C0 = 0,
    PROM_C1,
    PROM_C2,
    PROM_C3,
    PROM_C4,
    PROM_C5,
    PROM_C6,
}_PROM_Index;

typedef enum{
	CONVERSION_IDLE 		= 0,
	D1_CONVERSION_PROCESS 	= 1,
	D2_CONVERSION_PROCESS 	= 2
}_lastConversionProcess;


//Values read from MS5837
typedef struct  {
    uint16_t C[7];
    uint8_t D1_BUFF[3];
    uint8_t D2_BUFF[3];
    uint32_t D1;
    uint32_t D2;


    //VALUES FOR CALCULATION
    int32_t dT;
    int64_t OFF;
    int64_t SENS;

    int32_t OFFi;
    int32_t SENSi;
    int64_t Ti;
    int64_t OFF2;
    int64_t SENS2;
}_MS5837_Values;


typedef struct  {
    I2C_HandleTypeDef* MS5837_I2C_Port;
    void (*TopLayerConvertCompleteCallback)();
    void (*TopLayerReadCompleteCallback)();
    void (*TopLayerTxCallback)();
    void (*TopLayerRxCallback)();

    uint8_t	MS5837_DevAddr;
    _MS5837_Values  MS5837_Values;
    int32_t TEMP;
    int32_t P;

    _lastConversionProcess lastConversionProcess;
}_MS5837;
extern _MS5837 MS5837;

//_MS5837_Values Defines:

#define         C_BEGIN_ADDR	MS5837.MS5837_Values.C
#define         CRC_FACTORY     MS5837.MS5837_Values.C[0]
#define         SENS_T1         MS5837.MS5837_Values.C[1]
#define         OFF_T1          MS5837.MS5837_Values.C[2]
#define         TCS             MS5837.MS5837_Values.C[3]
#define         TCO             MS5837.MS5837_Values.C[4]
#define         T_REF           MS5837.MS5837_Values.C[5]
#define         TEMPSENS        MS5837.MS5837_Values.C[6]


#define         D1           	MS5837.MS5837_Values.D1
#define         D2        		MS5837.MS5837_Values.D2

#define         D1_BUFF         MS5837.MS5837_Values.D1_BUFF
#define         D2_BUFF       	MS5837.MS5837_Values.D2_BUFF

#define         MS5837_VALUES  	MS5837.MS5837_Values
#define         PRESSURE	  	MS5837.P
#define         TEMPERATURE	  	MS5837.TEMP

#define			LAST_CONVERSION MS5837.lastConversionProcess

#define         MS5837_I2C_PORT             MS5837.MS5837_I2C_Port
#define         MS5837_DEV_ADDR             MS5837.MS5837_DevAddr
#define         BAR30_TX_CALLBACK           MS5837.TopLayerTxCallback
#define         BAR30_RX_CALLBACK           MS5837.TopLayerRxCallback
#define         BAR30_CONV_CMP_CALLBACK     MS5837.TopLayerConvertCompleteCallback
#define         BAR30_READ_CMP_CALLBACK     MS5837.TopLayerReadCompleteCallback



void MS5837_TypedefInit(_MS5837 **MS5837_AddrOnTopLayer);


void MS5837_AbortI2C_IT();
void MS5837_Reset();
void MS5837_PROM_Read(_PROM_Index PROM_Index);
void MS5837_BusRead(uint8_t Command, uint8_t *LocalRegAddr, uint8_t Length);
void LSBtoMSB_C_Values();
uint8_t CheckTheCRC();

void ConvertD1();
void LSBtoMSB_D1();

void ConvertD2();
void LSBtoMSB_D2();

void MS5837_GetCurrentADC();

void MS5837_ReadDataRawToProcess();

void MS5837_30BA_Calc_PoweredByAEM();



#endif /* MS5837_H_ */
