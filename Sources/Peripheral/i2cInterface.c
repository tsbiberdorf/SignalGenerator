/*
 * i2c.c
 *
 *  Created on: Mar 13, 2015
 *      Author: TBiberdorf
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "Cpu.h"
#include "arm_cm4.h"

#include "i2cInterface.h"

#define i2c_DisableAck()       I2C0_C1 |= I2C_C1_TXAK_MASK

#define i2c_RepeatedStart()    I2C0_C1     |= 0x04;

#define i2c_Start()            I2C0_C1     |= 0x10;\
		I2C0_C1     |= I2C_C1_MST_MASK

#define i2c_Stop()             I2C0_C1  &= ~I2C_C1_MST_MASK;\
		I2C0_C1  &= ~I2C_C1_TX_MASK

#define i2c_EnterRxMode()      I2C0_C1   &= ~I2C_C1_TX_MASK;\
		I2C0_C1   &= ~I2C_C1_TXAK_MASK

#define i2c_Wait()               while((I2C0_S & I2C_S_IICIF_MASK)==0) {} \
		I2C0_S |= I2C_S_IICIF_MASK;

#define i2c_write_byte(data)   I2C0_D = data

#define MWSR                   0x00  /* Master write  */
#define MRSW                   0x01  /* Master read */


uint8_t MasterTransmission;
uint8_t SlaveID;

typedef enum I2CStates_e
{
	eI2CIdle,
	eI2CWriteRegister,
	eI2CWriteData,
	eI2CStop,
}eI2CStates_t;
typedef struct I2CComm_s
{
	uint8_t Register;
	uint8_t *DataPtr;
	uint8_t DataSize;
	eI2CStates_t State;
}sI2CComm_t;

sI2CComm_t gI2COp;

void i2c0_IRQ()
{
	I2C0_S |= I2C_S_IICIF_MASK;

	switch(gI2COp.State)
	{
	case eI2CIdle:
		break;
	case eI2CWriteRegister:
		I2C0_D = gI2COp.Register;
		gI2COp.State = eI2CWriteData;
		break;
	case eI2CWriteData:
		I2C0_D = *(gI2COp.DataPtr++);
		gI2COp.DataSize--;
		if(gI2COp.DataSize == 0)
		{
			gI2COp.State = eI2CStop;
		}
		break;
	case eI2CStop:
		gI2COp.State = eI2CIdle;
		I2C0_C1  &= ~I2C_C1_MST_MASK;
		I2C0_C1  &= ~I2C_C1_TX_MASK;
		disable_irq(INT_I2C0-16) ;   	/* disable I2C0 interrupt in NVIC */
		I2C0_C1 &= ~I2C_C1_IICIE_MASK;
		break;
	}
}


/*******************************************************************/
/*!
 * Start I2C Transmision
 * @param SlaveID is the 7 bit Slave Address
 * @param Mode sets Read or Write Mode
 */
static void IIC_StartTransmission (uint8_t Address, uint8_t SlaveID, uint8_t Mode)
{
	if(Mode == MWSR)
	{
		/* set transmission mode */
		MasterTransmission = MWSR;
	}
	else
	{
		/* set transmission mode */
		MasterTransmission = MRSW;
	}

	/* shift ID in right possition */
	SlaveID = Address << 1;

	/* Set R/W bit at end of Slave Address */
	SlaveID |= (unsigned char)MasterTransmission;

	/* send start signal */
	i2c_Start();

	/* send ID with W/R bit */
	i2c_write_byte(SlaveID);
}


/*******************************************************************/
/*!
 * Pause Routine
 */
void Pause(void){
	int n;
	for(n=1;n<50;n++) {
		__asm("nop");
	}
}

/*******************************************************************/
/*!
 * Read a register from the MPR084
 * @param u8RegisterAddress is Register Address
 * @return Data stored in Register
 */
unsigned char i2cReadRegister(uint8_t Address, uint8_t u8RegisterAddress)
{
	uint8_t result;
	unsigned int j;

	/* Send Slave Address */
	IIC_StartTransmission(Address,SlaveID,MWSR);
	i2c_Wait();

	/* Write Register Address */
	I2C0_D = u8RegisterAddress;
	i2c_Wait();

	/* Do a repeated start */
	I2C0_C1 |= I2C_C1_RSTA_MASK;

	/* Send Slave Address */
	I2C0_D = (Address << 1) | 0x01; //read address
	i2c_Wait();

	/* Put in Rx Mode */
	I2C0_C1 &= (~I2C_C1_TX_MASK);

	/* Turn off ACK */
	I2C0_C1 |= I2C_C1_TXAK_MASK;

	/* Dummy read */
	result = I2C0_D ;
	for (j=0; j<5000; j++){};
	i2c_Wait();

	/* Send stop */
	i2c_Stop();
	result = I2C0_D ;
	Pause();
	return result;
}

/*******************************************************************/
/*!
 * Read a register from the MPR084
 * @param u8RegisterAddress is Register Address
 * @return Data stored in Register
 */
unsigned char i2cMultiReadRegister(uint8_t Address, uint8_t u8RegisterAddress, uint8_t *DataBlock, uint8_t ReadSize)
{
	uint8_t result = 0;
	unsigned int j;

	/* Send Slave Address */
	IIC_StartTransmission(Address,SlaveID,MWSR);
	i2c_Wait();

	/* Write Register Address */
	I2C0_D = u8RegisterAddress;
	i2c_Wait();

	/* Do a repeated start */
	I2C0_C1 |= I2C_C1_RSTA_MASK;

	/* Send Slave Address */
	I2C0_D = (Address << 1) | 0x01; //read address
	i2c_Wait();

	/* Put in Rx Mode */
	I2C0_C1 &= (~I2C_C1_TX_MASK);

	I2C0_C1 &= (~I2C_C1_TXAK_MASK);
	/* Turn off ACK */
//	I2C0_C1 |= I2C_C1_TXAK_MASK;

	/* Dummy read */
	result = I2C0_D ;
	for (j=0; j<5000; j++){};
	i2c_Wait();

	result = 0;
	while(ReadSize--)
	{
		DataBlock[result++] = I2C0_D;
		i2c_Wait();

		if(ReadSize == 1)
		{
			/* Turn off ACK */
			I2C0_C1 |= I2C_C1_TXAK_MASK;
		}
	}
	/* Send stop */
	i2c_Stop();
	DataBlock[result] = I2C0_D ;
	Pause();
	return result;
}

/*******************************************************************/
/*!
 * Write a byte of Data to specified register on I2C0 bus
 * @param Address I2C bus address
 * @param u8RegisterAddress is Register Address
 * @param u8Data is Data to write
 */
void i2cWriteRegister(uint8_t Address, uint8_t u8RegisterAddress, uint8_t u8Data)
{
	gI2COp.Register = u8RegisterAddress;
	gI2COp.DataPtr = &u8Data;
	gI2COp.DataSize = 1;
	gI2COp.State = eI2CWriteRegister;

	enable_irq(INT_I2C0-16) ;   	/* enable I2C0 interrupt in NVIC */
	I2C0_C1 |= I2C_C1_IICIE_MASK;

	/* send data to slave */
	IIC_StartTransmission(Address,SlaveID,MWSR);
	i2c_Wait();

	I2C0_D = u8RegisterAddress;
	i2c_Wait();

	I2C0_D = u8Data;
	i2c_Wait();

	i2c_Stop();

	Pause();
}

/*******************************************************************/
/*!
 * Write a byte of Data to specified register on I2C0 bus
 * @param Address I2C bus address
 * @param PtrData pointer to where the data is located
 * @param Size number of bytes to write
 */
void i2cWriteData(uint8_t Address, uint8_t *PtrData,uint8_t Size)
{
	uint8_t i;
	if(Size)
	{
		gI2COp.Register = Address;
		gI2COp.DataPtr = PtrData;
		gI2COp.DataSize = Size;
		gI2COp.State = eI2CWriteRegister;

		enable_irq(INT_I2C0-16) ;   		/* enable PIT2 interrupt in NVIC */
		I2C0_C1 |= I2C_C1_IICIE_MASK;
		/* send data to slave */
		IIC_StartTransmission(Address,SlaveID,MWSR);
//		i2c_Wait();
//
//		for(i=0; i<Size; i++)
//		{
//			I2C0_D = PtrData[i];
//			i2c_Wait();
//		}
//		i2c_Stop();
//
//		Pause();
	}
}
/**
 * @details Initialise the I2C0 Bus.
 *
 * PTB2 - I2C0_SCL
 * PTB3 - I2C0_SDA
 *
 * @return void
 */
void init_I2C(void)
{
	uint32_t regValue;
	SIM_SCGC4 |= SIM_SCGC4_I2C0_MASK; //Turn on clock to I2C0 module

	/* configure GPIO for I2C0 function */
	//    PORTB_PCR2 = PORT_PCR_MUX(2);
	//    PORTB_PCR3 = PORT_PCR_MUX(2);

	regValue = I2C0_S;
	if(regValue & I2C_S_ARBL_MASK)
	{
		I2C0_S |= I2C_S_ARBL_MASK;
		printf("had to write I2C_S_ARBL_MASK\r\n");
	}
	regValue = I2C0_S;

	I2C0_F  = 0xf; /* set MULT and ICR */
//	I2C0_F  = 0x14; /* set MULT and ICR */
//	I2C0_F  = I2C_F_MULT(0x00) | I2C_F_ICR(0x14);       /* set MULT and ICR */
//	I2C0_F  = I2C_F_MULT(0x20) | I2C_F_ICR(0x00);       /* set MULT and ICR */

	I2C0_C1 = I2C_C1_IICEN_MASK;       /* enable IIC */

	set_irq_priority (INT_I2C0-16, 1); 	/* assign priority of I2C0 irq in NVIC */
//	enable_irq(INT_I2C0-16) ;   		/* enable I2C0 interrupt in NVIC */

}

