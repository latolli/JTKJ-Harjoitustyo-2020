/*
 * opt3001.c
 *
 *  Created on: 22.7.2016
 *  Author: Teemu Leppanen / UBIComp / University of Oulu
 *
 *  Datakirja: http://www.ti.com/lit/ds/symlink/opt3001.pdf
 */

#include <string.h>
#include <math.h>
#include <inttypes.h>
#include <xdc/runtime/System.h>
#include "Board.h"
#include "sensors/opt3001.h"


void opt3001_setup(I2C_Handle *i2c) {
	I2C_Transaction i2cTransaction;
	char itxBuffer[3];

    i2cTransaction.slaveAddress = Board_OPT3001_ADDR;
    itxBuffer[0] = OPT3001_REG_CONFIG;
    itxBuffer[1] = 0xCE; // continuous mode s.22
    itxBuffer[2] = 0x02;
    i2cTransaction.writeBuf = itxBuffer;
    i2cTransaction.writeCount = 3;
    i2cTransaction.readBuf = NULL;
    i2cTransaction.readCount = 0;

    if (I2C_transfer(*i2c, &i2cTransaction)) {
        System_printf("OPT3001: Config write ok\n");
    } 
	else {
        System_printf("OPT3001: Config write failed!\n");
    }
    System_flush();
}

uint16_t opt3001_get_status(I2C_Handle *i2c) {
	uint16_t e=0;
	I2C_Transaction i2cTransaction;
	char itxBuffer[1];
	char irxBuffer[2];

	/* Read sensor state */
	i2cTransaction.slaveAddress = Board_OPT3001_ADDR;
	itxBuffer[0] = OPT3001_REG_CONFIG;
	i2cTransaction.writeBuf = itxBuffer;
	i2cTransaction.writeCount = 1;
	i2cTransaction.readBuf = irxBuffer;
	i2cTransaction.readCount = 2;

	if (I2C_transfer(*i2c, &i2cTransaction)) {
		e = (irxBuffer[0] << 8) | irxBuffer[1];
	} 
	else {
		e = 0;
	}
	return e;
}

double opt3001_get_data(I2C_Handle *i2c) {
	double lux = -1.0;
	I2C_Transaction i2cTransaction;

	if ((opt3001_get_status(i2c) && OPT3001_DATA_READY)) {
		uint8_t txBuffer[1];		// sends 1 byte
		uint8_t rxBuffer[2];		// receives 2 bytes
		txBuffer[0] = OPT3001_REG_RESULT;

		i2cTransaction.slaveAddress = Board_OPT3001_ADDR;	// i2c address for opt3001
		i2cTransaction.writeBuf = txBuffer;
		i2cTransaction.writeCount = 1;
		i2cTransaction.readBuf = rxBuffer;
		i2cTransaction.readCount = 2;


		if (I2C_transfer(*i2c, &i2cTransaction)) {
			//Converted the register values (in rxBuffer) into 16 bit integer
			uint16_t rekisteri = ((uint16_t)rxBuffer[0] << 8) | (uint16_t)rxBuffer[1];
			uint16_t maski2 = 0b0000111111111111;
			uint16_t E0_3, R0_11;

			R0_11 = rekisteri & maski2;
			E0_3 = rekisteri >> 12;
            lux =  0.01 * pow(2, E0_3) * R0_11;
		} 
		else {
			System_printf("OPT3001: Data read failed!\n");
			System_flush();
		}
	} 
	else {
		System_printf("OPT3001: Data not ready!\n");
		System_flush();
	}
	return lux;
}
