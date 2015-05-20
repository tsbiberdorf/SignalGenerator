/*
 * i2c.h
 *
 *  Created on: Mar 13, 2015
 *      Author: TBiberdorf
 */

#ifndef I2C_H_
#define I2C_H_



void i2cWriteRegister(uint8_t Address, uint8_t u8RegisterAddress, unsigned char u8Data);
uint8_t i2cReadRegister(uint8_t Address, uint8_t u8RegisterAddress, uint8_t *u8Data, uint8_t u8DataSize);
unsigned char i2cMultiReadRegister(uint8_t Address, uint8_t u8RegisterAddress, uint8_t *DataBlock, uint8_t ReadSize);
void i2cWriteData(uint8_t Address, uint8_t *PtrData,uint8_t Size);
void init_I2C(void);

#endif /* I2C_H_ */
