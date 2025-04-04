/*
 * AEM_BAR30.h
 *
 *  Created on: 1 Jan 2022
 *      Author: Ali Eren Mercan
 */

#ifndef AEM_BAR30_H_
#define AEM_BAR30_H_

#include "MS5837.h"
#include "main.h"

#define         BAR30_TASK_HZ       		50
#define         BAR30_I2C_TIMEOUT         	(BAR30_TASK_HZ >> 1)

extern I2C_HandleTypeDef hi2c1;
#define         BAR30_I2C_PORT      		hi2c1

#define         D1toD2_CHANGE_THRESOLD      49

typedef enum{
    BAR30_I2C_IDLE    =       0,
	BAR30_I2C_PENDING =       1
}_BAR30_I2C_Status;

typedef enum{
	BAR30_INIT    		=       0,
	BAR30_READ    		=       1,
	BAR30_DELAY   		=       2
}_BAR30_Status;

typedef enum{
	BAR30_MS5837_RESET    	=       0,
	BAR30_READ_PROM    		=       1,
	BAR30_D2_CONVERT		=		2,
	BAR30_D2_GET_ADC		=		3,
	BAR30_D1_CONVERT		=		4,
	BAR30_D1_GET_ADC		=		5,
	BAR30_END_INIT   		=       6
}_BAR30_Init_Step;

typedef enum{
	MS5837_CONVERT_D1    			=       0,
	MS5837_CONVERT_D2    			=       1,
	MS5837_READ_ADC					= 		2
}_BAR30_Read_Step;

typedef enum{
    C0              =       0,
    C1              =       1,
    C2              =       2,
    C3              =       3,
    C4              =       4,
    C5              =       5,
    C6              =       6,
    READ_PROM_END   =       7
}_BAR30_ReadProm_Step;

typedef struct{
    _BAR30_I2C_Status       BAR30_I2C_Status;
    _BAR30_Status           BAR30_Status;
    _BAR30_Status           oBAR30_Status;

    _BAR30_Init_Step        BAR30_Init_Step;
    _BAR30_ReadProm_Step    BAR30_ReadProm_Step;
    _BAR30_Read_Step		BAR30_Read_Step;
    uint8_t					ChangeD1toD2_Counter;
    _MS5837                 *MS5837_Addr;

    float					Depth;

    uint32_t                PendingTimeout;
    uint32_t                DelayCounter;
    uint32_t                DelayConstant;
}_AEM_BAR30;
extern _AEM_BAR30 AEM_BAR30;

#define     BAR30_I2C_STATUS            AEM_BAR30.BAR30_I2C_Status
#define     BAR30_STATUS                AEM_BAR30.BAR30_Status
#define     OLD_BAR30_STATUS            AEM_BAR30.oBAR30_Status
#define     BAR30_INIT_STEP             AEM_BAR30.BAR30_Init_Step
#define     READ_PROM_STEP              AEM_BAR30.BAR30_ReadProm_Step
#define     BAR30_READ_STEP             AEM_BAR30.BAR30_Read_Step
#define     D1toD2_COUNTER             	AEM_BAR30.ChangeD1toD2_Counter
#define     MS5837_TYPEDEF_PTR          AEM_BAR30.MS5837_Addr
#define     BAR30_PENDING_TIMEOUT       AEM_BAR30.PendingTimeout
#define     BAR30_DELAY_COUNTER         AEM_BAR30.DelayCounter
#define     BAR30_DELAY_CONSTANT        AEM_BAR30.DelayConstant


void AEM_BAR30_Init();
void MS5837_DriverInit();
void BAR30_Process();
void BAR30_WhenI2C_Pending();
void BAR30_WhenI2C_Idle();
void BAR30_ProcessInit();
void ReadPromProcess();
void BAR30_ProcessRead();
void BAR30_ProcessDelay();
void BAR30_DelayStart(uint32_t ms);
void BAR30_ConvertProcess();

void BAR30_CalcDepth();

void BAR30_RECEIVE_IT();
void BAR30_TRANSMIT_IT();

void BAR30_CONVERTION_COMPLETE_IT();
void BAR30_DATA_READ_COMPLETE_IT(); //IT FOR D1 or D2 Read IT

#endif /* AEM_BAR30_H_ */
