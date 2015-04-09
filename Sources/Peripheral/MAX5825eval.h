/*
 * MAX5825eval.h
 *
 *  Created on: Apr 5, 2015
 *      Author: ThreeBoys
 */

#ifndef MAX5825EVAL_H_
#define MAX5825EVAL_H_



const char *gMAX5825evalTaskname;

void MAX5825evalTask(void *pvParameters);

void initMAX5825();
void parseDACCmd(const int8_t *Cmd);


#endif /* MAX5825EVAL_H_ */
