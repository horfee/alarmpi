/*
 * AlarmMode.cpp
 *
 *  Created on: 13 août 2015
 *      Author: horfee
 */
#include "Mode.h"

#include <strings.h>

namespace alarmpi {

std::ostream & operator<<(std::ostream &os, const Mode& m) {
	os << m.getName() << ": " << m.getDescription();
	return os;
}

Mode::Mode(std::string name, std::string description, ModeType type) {
	this->name = name;
	this->description = description;
	this->type = type;
}

Mode::~Mode() {

}

ModeType Mode::getType() const {
	return type;
}

std::string Mode::getName() const {
	return name;
}

void Mode::setDescription(std::string description) {
	this->description = description;
}

std::string Mode::getDescription() const {
	return this->description;
}

bool Mode::operator==(const std::string& name) {
	return strcasecmp(this->name.c_str(), name.c_str()) == 0;
}

bool Mode::operator==(const Mode& mode) {
	return *this == mode.name;
}

bool Mode::operator!=(const std::string& name) {
	return strcasecmp(this->name.c_str(), name.c_str()) != 0;
}

bool Mode::operator!=(const Mode& mode) {
	return *this != mode.name;
}

Json::Value Mode::toJSON() const {
	Json::Value value;

	value["name"] = name;
	value["description"] = description;
	value["type"] = type;
	return value;
}


}


