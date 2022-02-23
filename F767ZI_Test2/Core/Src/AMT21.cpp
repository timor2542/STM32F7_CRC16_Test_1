/*
 * AMT21.cpp
 *
 *  Created on: Jan 26, 2022
 *      Author: SakuranohanaTH
 */
#include "AMT21.h"

AMT21::AMT21(UART_HandleTypeDef *_amt21_huart, uint8_t _address) {
	this->amt21_huart = _amt21_huart;
	this->address = _address;
}

AMT21::~AMT21() {

}

void AMT21::AMT21_Read() {
//	HAL_GPIO_WritePin(dev->DE_port, dev->DE_pin, 1);
	HAL_UART_Transmit(this->amt21_huart, (uint8_t*) &(this->address),
			sizeof(this->address), 100);
//	HAL_GPIO_WritePin(dev->DE_port, dev->DE_pin, 0);
	HAL_UART_Receive(this->amt21_huart, (uint8_t*) &(this->uart_buf), 2, 100);
	this->k0 = (this->uart_buf & 0x4000) == 0x4000;
	this->k1 = (this->uart_buf & 0x8000) == 0x8000;
}

void AMT21::AMT21_Set_Zero() {
//	HAL_GPIO_WritePin(dev->DE_port, dev->DE_pin, 1);
	uint8_t set_zero_command[2] = { (this->address + 0x02), 0x5E };
	HAL_UART_Transmit(this->amt21_huart, (uint8_t*) set_zero_command,
			sizeof(this->address), 100);
//	HAL_GPIO_WritePin(dev->DE_port, dev->DE_pin, 0);
}

void AMT21::AMT21_Reset() {
	//	HAL_GPIO_WritePin(dev->DE_port, dev->DE_pin, 1);
	uint8_t set_zero_command[2] = { (this->address + 0x02), 0x75 };
	HAL_UART_Transmit(this->amt21_huart, (uint8_t*) set_zero_command,
			sizeof(this->address), 100);
	//	HAL_GPIO_WritePin(dev->DE_port, dev->DE_pin, 0);
}

HAL_StatusTypeDef AMT21::AMT21_Check_Value() {
	uint16_t raw_value_temp = this->uart_buf & 0x3FFF;
	uint8_t k0_check = this->uart_buf & 0x0001;
	uint8_t k1_check = (this->uart_buf >> 1) & 0x0001;
	for (uint8_t i = 0; i < 6; i++) {
		this->uart_buf = this->uart_buf >> 2;
		k0_check ^= this->uart_buf & 0x0001;
		k1_check ^= (this->uart_buf >> 1) & 0x0001;
	}
	k0_check = !k0_check;
	k1_check = !k1_check;
	if ((this->k0 == k0_check) && (this->k1 == k1_check)) {
		this->raw_value = raw_value_temp;
		return HAL_OK;
	} else {
		this->raw_value = 0;
		return HAL_ERROR;
	}
}

int16_t AMT21::getRawValue() {
	return this->raw_value;
}
int16_t AMT21::getPrevRawValue(){
	return this->prev_raw_value;
}

int16_t AMT21::getAngPos180() {
	return ((((this->raw_value & 0x2000) >> 13) * (-16383))
			+ (this->raw_value & 0x3FFF)) * -1;
}
int32_t AMT21::getUnwarppedValue(int32_t pulse, int32_t prev_pulse){
	int32_t dPulse = 0;
	if (pulse - prev_pulse > 8191) {
		dPulse = -(16383 - (pulse-prev_pulse));
	} else if ( pulse -  prev_pulse < -8191) {
		dPulse = 16383 - (prev_pulse - pulse);
	} else {
		dPulse =  pulse -  prev_pulse;
	}
	return dPulse;
}
