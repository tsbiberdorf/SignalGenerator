/*
 * FXOS8700CQ.c
 *
 *  Created on: Mar 13, 2015
 *      Author: TBiberdorf
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "Cpu.h"
#include "FreeRTOS.h"
#include "task.h"
#include "i2cInterface.h"
#include "FXOS8700CQ.h"


#define FXOS_STATUS_REG 		(0x00)
#define FXOS_WHO_AM_I_REG 		(0x0D)
#define FXOS_XYZ_DATA_CFG_REG 	(0x0E)
#define FXOS_ACCEL_CNTRL_REG1 	(0x2A)
#define FXOS_MAG_CNTRL_REG1 	(0x5B)
#define FXOS_MAG_CNTRL_REG2 	(0x5C)


#define WHO_AM_I_VALUE (0xC7)


#define FXOS_OUT_X_MSB (0x01)
#define FXOS_OUT_X_LSB (0x02)
#define FXOS_OUT_Y_MSB (0x03)
#define FXOS_OUT_Y_LSB (0x04)
#define FXOS_OUT_Z_MSB (0x05)
#define FXOS_OUT_Z_LSB (0x06)

#define FXOS_MAG_X_MSB (0x33)
#define FXOS_MAG_X_LSB (0x34)
#define FXOS_MAG_Y_MSB (0x35)
#define FXOS_MAG_Y_LSB (0x36)
#define FXOS_MAG_Z_MSB (0x37)
#define FXOS_MAG_Z_LSB (0x38)


typedef enum _FXOSCmds_e
{
	eFXOSidle,
	eFXOSStatus,
	eFXOSWhoAmI,
	eFXOSDataReadings,
	eInitFXOS,
	eFXOSAccelCntrlReg1,
	eFXOSMagCntrlReg1,
	eFXOSMagCntrlReg2,
}eFXOSCmds_t;

const char *gFXOS8700CQTaskname = "FXOS8700CQ Interface Task";
#define FXOS8700_I2C_ADDRESS                         0x1C

uint32_t gFXOScmd = 0;
uint8_t gFXOSData = 0;

typedef struct sInertialSensor
{
	uint16_t xAccel;
	uint16_t yAccel;
	uint16_t zAccel;
	uint16_t xMag;
	uint16_t yMag;
	uint16_t zMag;
}sInertialSensor_t ;
sInertialSensor_t gInertialSensorReading;

static int32_t initializeInertiaSensor()
{
	uint32_t value;

	value = i2cReadRegister(FXOS8700_I2C_ADDRESS,FXOS_WHO_AM_I_REG);
	if(value != WHO_AM_I_VALUE)
	{
		printf("error with FXOS Init\r\n");
		goto exitMethod;
	}

	i2cWriteRegister(FXOS8700_I2C_ADDRESS,FXOS_ACCEL_CNTRL_REG1,0x00);
	i2cWriteRegister(FXOS8700_I2C_ADDRESS,FXOS_MAG_CNTRL_REG1,0x1f);
	i2cWriteRegister(FXOS8700_I2C_ADDRESS,FXOS_MAG_CNTRL_REG2,0x20);
	i2cWriteRegister(FXOS8700_I2C_ADDRESS,FXOS_XYZ_DATA_CFG_REG,0x01);

	i2cWriteRegister(FXOS8700_I2C_ADDRESS,FXOS_ACCEL_CNTRL_REG1,0x0d);

	exitMethod:
	return 0;
}

static int32_t readInertiaSensor()
{
	uint16_t valueMSB;
	uint16_t valueLSB;

	valueMSB = i2cReadRegister(FXOS8700_I2C_ADDRESS,FXOS_OUT_X_MSB);
	valueLSB = i2cReadRegister(FXOS8700_I2C_ADDRESS,FXOS_OUT_X_LSB);
	gInertialSensorReading.xAccel = (int16_t)((valueMSB<<8) | (valueLSB>>2));

	valueMSB = i2cReadRegister(FXOS8700_I2C_ADDRESS,FXOS_OUT_Y_MSB);
	valueLSB = i2cReadRegister(FXOS8700_I2C_ADDRESS,FXOS_OUT_Y_LSB);
	gInertialSensorReading.yAccel = (int16_t)((valueMSB<<8) | (valueLSB>>2));

	valueMSB = i2cReadRegister(FXOS8700_I2C_ADDRESS,FXOS_OUT_Z_MSB);
	valueLSB = i2cReadRegister(FXOS8700_I2C_ADDRESS,FXOS_OUT_Z_LSB);
	gInertialSensorReading.zAccel = (int16_t)((valueMSB<<8) | (valueLSB>>2));

	valueMSB = i2cReadRegister(FXOS8700_I2C_ADDRESS,FXOS_MAG_X_MSB);
	valueLSB = i2cReadRegister(FXOS8700_I2C_ADDRESS,FXOS_MAG_X_LSB);
	gInertialSensorReading.xMag = (int16_t)((valueMSB<<8) | (valueLSB));

	valueMSB = i2cReadRegister(FXOS8700_I2C_ADDRESS,FXOS_MAG_Y_MSB);
	valueLSB = i2cReadRegister(FXOS8700_I2C_ADDRESS,FXOS_MAG_Y_LSB);
	gInertialSensorReading.yMag = (int16_t)((valueMSB<<8) | (valueLSB));

	valueMSB = i2cReadRegister(FXOS8700_I2C_ADDRESS,FXOS_MAG_Z_MSB);
	valueLSB = i2cReadRegister(FXOS8700_I2C_ADDRESS,FXOS_MAG_Z_LSB);
	gInertialSensorReading.zMag = (int16_t)((valueMSB<<8) | (valueLSB));

	printf("x:%d y:%d z:%d mx:%d my:%d mz:%d\r\n"
			,gInertialSensorReading.xAccel
			,gInertialSensorReading.yAccel
			,gInertialSensorReading.zAccel
			,gInertialSensorReading.xMag
			,gInertialSensorReading.yMag
			,gInertialSensorReading.zMag);

	return 0;
}

/******************************************************************************
 * public
 *****************************************************************************/
void readFXOSStatus()
{
	gFXOScmd = eFXOSStatus;
}

void readFXOSWhoAmI()
{
	gFXOScmd = eFXOSWhoAmI;
}

void readInertialValues()
{
	gFXOScmd = eFXOSDataReadings;
}

int32_t writeAccelCntlReg(int8_t *Cmd)
{
	int32_t retCode = 0;
	uint8_t value;
	if( Cmd == NULL)
	{
		retCode = -1;
	}
	retCode = sscanf(Cmd,"%x",&value);
	printf("write value %X\r\n",value);
	gFXOSData = value;
	gFXOScmd = eFXOSAccelCntrlReg1;
	return retCode;
}

int32_t writeMagCntlReg1(int8_t *Cmd)
{
	int32_t retCode = 0;
	uint8_t value;
	if( Cmd == NULL)
	{
		retCode = -1;
	}
	retCode = sscanf(Cmd,"%x",&value);
	printf("write value %X\r\n",value);
	gFXOSData = value;
	gFXOScmd = eFXOSMagCntrlReg1;
	return retCode;
}

int32_t writeMagCntlReg2(int8_t *Cmd)
{
	int32_t retCode = 0;
	uint8_t value;
	if( Cmd == NULL)
	{
		retCode = -1;
	}
	retCode = sscanf(Cmd,"%x",&value);
	printf("write value %X\r\n",value);
	gFXOSData = value;
	gFXOScmd = eFXOSMagCntrlReg2;
	return retCode;
}

void initFXOS()
{
	gFXOScmd = eInitFXOS;
}


void FXOS8700CQTask(void *pvParameters)
{
	uint8_t result;

	printf("start FXOS8700CQTask\r\n");
	init_I2C();
	gFXOScmd = 0;

	while(1)
	{
		if(gFXOScmd)
		{
			switch(gFXOScmd)
			{
			case eFXOSStatus:
				result = i2cReadRegister(FXOS8700_I2C_ADDRESS,FXOS_STATUS_REG);
				break;
			case eFXOSWhoAmI:
				result = i2cReadRegister(FXOS8700_I2C_ADDRESS,FXOS_WHO_AM_I_REG);
				break;
			case eFXOSDataReadings:
				result = readInertiaSensor();
				break;
			case eInitFXOS:
				result = initializeInertiaSensor();
				break;
			case eFXOSAccelCntrlReg1:
				i2cWriteRegister(FXOS8700_I2C_ADDRESS,FXOS_ACCEL_CNTRL_REG1,gFXOSData);
				result = 0;
				break;
			case eFXOSMagCntrlReg1:
				i2cWriteRegister(FXOS8700_I2C_ADDRESS,FXOS_MAG_CNTRL_REG1,gFXOSData);
				result = 0;
				break;
			case eFXOSMagCntrlReg2:
				i2cWriteRegister(FXOS8700_I2C_ADDRESS,FXOS_MAG_CNTRL_REG2,gFXOSData);
				result = 0;
				break;
			}
			printf("result: %x\r\n",result);
			gFXOScmd = 0;
		}
		vTaskDelay(10);
	}
}


