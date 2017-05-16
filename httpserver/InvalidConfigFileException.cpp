/*
 * InvalidConfigFileException.cpp
 *
 *  Created on: 19 juil. 2016
 *      Author: horfee
 */

#include "InvalidConfigFileException.h"

InvalidConfigFileException::InvalidConfigFileException(std::string reason): reason(reason) {
}

const char* InvalidConfigFileException::what() const EXCEPTION {
	return reason.c_str();
}

InvalidConfigFileException::~InvalidConfigFileException() {
}

