/*
 * Property.cpp
 *
 *  Created on: 31 juil. 2015
 *      Author: horfee
 */

#include "Property.h"
#include <typeinfo>
#include <map>
#include <algorithm>
#include <string>

namespace alarmpi {

Property::Property(string sName, string sDescription, string type, string value): name(sName), description(sDescription), type(type) {

	bValue = false;
	fValue = 0.0f;
	iValue = 0;
	this->setValue(value);
}

Property::Property(string sName, string sDescription, string sValue): Property(sName, sDescription, "STRING", sValue) {}

Property::Property(string sName, string sDescription, int iValue): Property(sName, sDescription, "INT", std::to_string(iValue)) {}

Property::Property(string sName, string sDescription, float fValue): Property(sName, sDescription, "FLOAT", std::to_string(fValue)) {}

Property::Property(string sName, string sDescription, bool bValue): Property(sName, sDescription, "BOOL", bValue ? "true" : "false") {}

string Property::getValueAsString() const {
	std::string t = getType();
	if ( t == "INT"    ) return std::to_string(getIntValue());
	if ( t == "FLOAT"  ) return std::to_string(getFloatValue());
	if ( t == "BOOL"   ) return getBoolValue() ? "true" : "false";
	if ( t == "STRING" ) return getStringValue();
	return "";
}

string Property::getType() const {
	return type;
}

string Property::getName() const {
	return name;
}

string Property::getDescription() const {
	return description;
}

void Property::setDescription(std::string description) {
	this->description = description;
}

int Property::getIntValue() const {
	return iValue;
}

float Property::getFloatValue() const {
	return fValue;
}

bool Property::getBoolValue() const {
	return bValue;
}

string Property::getStringValue() const {
	return string(sValue);
}

void Property::setBoolValue(bool value) {
	bValue = value;
}

void Property::setIntValue(int value) {
	iValue = value;
}

void Property::setFloatValue(float value) {
	fValue = value;
}

void Property::setStringValue(string value) {
	sValue = value;
}

void Property::setValue(std::string value) {
	if ( type == "INT" ) iValue = atoi(value.c_str());
	else if ( type == "FLOAT" ) fValue = atof(value.c_str());
	else if ( type == "BOOL" ) bValue = value == "true" || sValue == "TRUE";
	else {
		this->type = "STRING";
		sValue = value;
	}
}


Json::Value Property::toJSON() const {
	Json::Value res;
	res["name"] = name;
	res["description"] = description;
	res["value"] = getValueAsString();
	res["type"] = type;
	return res;
}

bool Property::operator==(const std::string& name) {
	return this->name == name;
}

bool Property::operator==(const Property& property) {
	return *this == property.name;
}

bool Property::operator!=(const std::string& name) {
	return this->name != name;
}

bool Property::operator!=(const Property& property) {
	return *this != property.name;
}

}
