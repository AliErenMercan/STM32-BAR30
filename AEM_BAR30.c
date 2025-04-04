/*
 * AEM_BAR30.c
 *
 *  Created on: 1 Jan 2022
 *      Author: Ali Eren Mercan
 */

#include "AEM_BAR30.h"

_AEM_BAR30 AEM_BAR30;


void AEM_BAR30_Init(){
    BAR30_I2C_STATUS = BAR30_I2C_IDLE;
    BAR30_STATUS = BAR30_INIT;
    BAR30_INIT_STEP = BAR30_MS5837_RESET;
    BAR30_READ_STEP = MS5837_CONVERT_D2;
    READ_PROM_STEP = C0;
    MS5837_DriverInit();
    BAR30_PENDING_TIMEOUT = 0;
    D1toD2_COUNTER = 0;
}

void MS5837_DriverInit(){
    MS5837_TypedefInit(&MS5837_TYPEDEF_PTR);
    MS5837_TYPEDEF_PTR->MS5837_I2C_Port = &BAR30_I2C_PORT;
    MS5837_TYPEDEF_PTR->TopLayerTxCallback = BAR30_TRANSMIT_IT;
    MS5837_TYPEDEF_PTR->TopLayerRxCallback = BAR30_RECEIVE_IT;
    MS5837_TYPEDEF_PTR->TopLayerConvertCompleteCallback = BAR30_CONVERTION_COMPLETE_IT;
    MS5837_TYPEDEF_PTR->TopLayerReadCompleteCallback = BAR30_DATA_READ_COMPLETE_IT;
}



void BAR30_Process(){
    if(BAR30_I2C_STATUS == BAR30_I2C_IDLE){
    	BAR30_PENDING_TIMEOUT = 0;
        BAR30_WhenI2C_Idle();
    }
    else if(BAR30_I2C_STATUS == BAR30_I2C_PENDING){
        BAR30_WhenI2C_Pending();
    }
    else{
        AEM_BAR30_Init();
    }
}

void BAR30_WhenI2C_Pending(){
    if(++BAR30_PENDING_TIMEOUT > BAR30_I2C_TIMEOUT){
        MS5837_AbortI2C_IT();
        AEM_BAR30_Init();
    }
}

void BAR30_WhenI2C_Idle(){

    if(BAR30_STATUS == BAR30_READ){
        BAR30_ProcessRead();
    }
    else if(BAR30_STATUS == BAR30_INIT){
        BAR30_ProcessInit();
    }
    else if(BAR30_STATUS == BAR30_DELAY){
        BAR30_ProcessDelay();
    }
    else{
    	AEM_BAR30_Init();
    }

}

void BAR30_ProcessRead(){
	if(BAR30_READ_STEP == MS5837_READ_ADC){
		if(++D1toD2_COUNTER > D1toD2_CHANGE_THRESOLD){
			D1toD2_COUNTER = 0;
			BAR30_READ_STEP = MS5837_CONVERT_D2;
		}
		else{
			BAR30_READ_STEP = MS5837_CONVERT_D1;
		}
		BAR30_I2C_STATUS = BAR30_I2C_PENDING;
		MS5837_GetCurrentADC();
	}
	else{
		AEM_BAR30_Init();
	}
}

void BAR30_ConvertProcess(){
	BAR30_I2C_STATUS = BAR30_I2C_PENDING;
	if(BAR30_READ_STEP == MS5837_CONVERT_D1){
		ConvertD1();
	}
	else if(BAR30_READ_STEP == MS5837_CONVERT_D2){
		ConvertD2();
	}
	else{
		AEM_BAR30_Init();
	}
	BAR30_CalcDepth();
}

void BAR30_ProcessInit(){

    if(BAR30_INIT_STEP == BAR30_MS5837_RESET){
                        BAR30_I2C_STATUS = BAR30_I2C_PENDING;
                        MS5837_Reset();
                        BAR30_DelayStart(400);
                        BAR30_INIT_STEP = BAR30_READ_PROM;
    }
    else if(BAR30_INIT_STEP == BAR30_READ_PROM){
                        ReadPromProcess();
    }
    else if(BAR30_INIT_STEP == BAR30_D2_CONVERT){
						BAR30_I2C_STATUS = BAR30_I2C_PENDING;
						ConvertD2();
                        BAR30_INIT_STEP = BAR30_D2_GET_ADC;
    }
    else if(BAR30_INIT_STEP == BAR30_D2_GET_ADC){
						BAR30_I2C_STATUS = BAR30_I2C_PENDING;
						MS5837_GetCurrentADC();
                        BAR30_INIT_STEP = BAR30_D1_CONVERT;
    }
    else if(BAR30_INIT_STEP == BAR30_D1_CONVERT){
						BAR30_I2C_STATUS = BAR30_I2C_PENDING;
						ConvertD1();
                        BAR30_INIT_STEP = BAR30_D1_GET_ADC;
    }
    else if(BAR30_INIT_STEP == BAR30_D1_GET_ADC){
						BAR30_I2C_STATUS = BAR30_I2C_PENDING;
						MS5837_GetCurrentADC();
                        BAR30_INIT_STEP = BAR30_END_INIT;
    }
    else if(BAR30_INIT_STEP == BAR30_END_INIT){
    					BAR30_CalcDepth();
        				BAR30_STATUS = BAR30_READ;
        				BAR30_INIT_STEP = BAR30_MS5837_RESET;
    }
    else{
                        AEM_BAR30_Init();
    }
}

void ReadPromProcess(){
    if(READ_PROM_STEP == READ_PROM_END){
    					LSBtoMSB_C_Values();
    					if(CheckTheCRC()){
    						BAR30_INIT_STEP = BAR30_D2_CONVERT;
                            READ_PROM_STEP = C0;
    					}
    					else{
                            AEM_BAR30_Init();
    					}
    }
    else{
        				BAR30_I2C_STATUS = BAR30_I2C_PENDING;
                        MS5837_PROM_Read(READ_PROM_STEP);
                        READ_PROM_STEP++;
    }
}


void BAR30_ProcessDelay(){
    if(BAR30_DELAY_COUNTER > BAR30_DELAY_CONSTANT){
        BAR30_STATUS = OLD_BAR30_STATUS;
    }
}

void BAR30_DelayStart(uint32_t ms){
    BAR30_DELAY_COUNTER = 0;
    BAR30_DELAY_CONSTANT = ms - 1;
    OLD_BAR30_STATUS = BAR30_STATUS;
    BAR30_STATUS = BAR30_DELAY;
}

void BAR30_CalcDepth(){
	MS5837_30BA_Calc_PoweredByAEM();
	AEM_BAR30.Depth = (((PRESSURE*100)-101300)/(1029*9.80665)) * 100;
}

void BAR30_RECEIVE_IT(){
    BAR30_I2C_STATUS = BAR30_I2C_IDLE;
}

void BAR30_TRANSMIT_IT(){
    BAR30_I2C_STATUS = BAR30_I2C_IDLE;
}

void BAR30_CONVERTION_COMPLETE_IT(){
    BAR30_I2C_STATUS = BAR30_I2C_IDLE;
    BAR30_READ_STEP = MS5837_READ_ADC;
}

void BAR30_DATA_READ_COMPLETE_IT(){
    BAR30_I2C_STATUS = BAR30_I2C_IDLE;
	MS5837_ReadDataRawToProcess();
    if(BAR30_STATUS == BAR30_READ){
    	BAR30_ConvertProcess();
    }
}
