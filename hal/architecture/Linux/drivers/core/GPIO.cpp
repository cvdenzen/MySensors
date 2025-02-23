/*
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2022 Sensnology AB
 * Full contributor list: https://github.com/mysensors/MySensors/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 */

#include "GPIO.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <gpiod.h>
#include "log.h"

// Declare a single default instance
GPIOClass GPIO = GPIOClass();

GPIOClass::GPIOClass()
{

	// struct gpiod_line* gpiod_lines[GPIOD_MAX_LINE_DEFINITIONS];
	// const char *chipdevname ;
	// struct gpiod_chip *chip;
	// Open GPIO chip
	chip = gpiod_chip_open(chipdevname);
	if (chip == NULL) {
		logError("Failed to open chip %s\n",chipdevname);
	}

	for (int i=0; i<GPIOD_MAX_LINE_DEFINITIONS; i++) {
		gpiod_lines[i] = NULL;
	}

}

GPIOClass::GPIOClass(const GPIOClass& other)
{
	chipdevname = other.chipdevname;
	chip = other.chip;
}

GPIOClass::~GPIOClass()
{
	gpiod_chip_close(chip);
}

void GPIOClass::pinMode(uint8_t pin, uint8_t mode)
{

	if (pin >= GPIOD_MAX_LINE_DEFINITIONS) {
		logError("GPIOClass::pinMode pin number too big: %d >= %d",pin,GPIOD_MAX_LINE_DEFINITIONS);
		return;
	}

	// Already defined?
	if (gpiod_lines[pin] == NULL) {
		gpiod_lines[pin] = gpiod_chip_get_line(chip, pin);
	}
	if (mode == OUTPUT) {
		if (gpiod_line_request_output(gpiod_lines[pin],"mysgw",0) != 0) {
			logError("Failure gpiod_line_reques_output for pin %d",pin);
			exit(1);
		} else {
			if (gpiod_line_request_input(gpiod_lines[pin],"mysgw") != 0) {
				logError("Failure gpiod_line_request_input for pin %d",pin);
				exit(1);
			}
		}
	}
}

void GPIOClass::digitalWrite(uint8_t pin, uint8_t value)
{
	// Already defined?
	if (gpiod_lines[pin] == NULL) {
		gpiod_lines[pin] = gpiod_chip_get_line(chip, pin);
		pinMode(pin,OUTPUT);
	}
	if (gpiod_line_set_value(gpiod_lines[pin],value) != 0) {
		logError("Failure setting pin %d to value %d",pin,value);
		exit(1);
	}
}

uint8_t GPIOClass::digitalRead(uint8_t pin)
{
	// Already defined?
	if (gpiod_lines[pin] == NULL) {
		gpiod_lines[pin] = gpiod_chip_get_line(chip, pin);
		pinMode(pin,INPUT);
	}
	uint8_t value;
	value = gpiod_line_get_value(gpiod_lines[pin]);
	if (value > 1) {
		logError("Failure getting value from pin %d",pin);
		exit(1);
	}
	return value;
}

uint8_t GPIOClass::digitalPinToInterrupt(uint8_t pin)
{
	return pin;
}

GPIOClass& GPIOClass::operator=(const GPIOClass& other)
{
	if (this != &other) {
		chip = other.chip;
		for (int i = 0; i < GPIOD_MAX_LINE_DEFINITIONS ; ++i) {
			gpiod_lines[i] = other.gpiod_lines[i];
		}
	}
	return *this;
}
