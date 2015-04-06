/*
 * FXOS8700CQ.h
 *
 *  Created on: Mar 13, 2015
 *      Author: TBiberdorf
 */

#ifndef FXOS8700CQ_H_
#define FXOS8700CQ_H_




const char *gFXOS8700CQTaskname;

void FXOS8700CQTask(void *pvParameters);

void readFXOSStatus();
void readFXOSWhoAmI();
void readInertiaValues();
void readInertiaValuesMulti();

void initFXOS();

int32_t writeAccelCntlReg(int8_t *Cmd);
int32_t writeMagCntlReg1(int8_t *Cmd);
int32_t writeMagCntlReg2(int8_t *Cmd);

#endif /* FXOS8700CQ_H_ */
