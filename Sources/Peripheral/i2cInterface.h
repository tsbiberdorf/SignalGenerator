/*
 * i2c.h
 *
 *  Created on: Mar 13, 2015
 *      Author: TBiberdorf
 */

#ifndef I2C_H_
#define I2C_H_



void i2cWriteRegister(uint8_t Address, uint8_t u8RegisterAddress, unsigned char u8Data);
uint8_t i2cReadRegister(uint8_t Address, uint8_t u8RegisterAddress);

void init_I2C(void);

#endif /* I2C_H_ */
