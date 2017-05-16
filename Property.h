/*
 * Property.h
 *
 *  Created on: 31 juil. 2015
 *      Author: horfee
 */

#ifndef PROPERTY_H_
#define PROPERTY_H_

#include <tuple>
#include <string>
#include "json/json.h"

using namespace std;

namespace alarmpi {


class Property {
public:

	Property(string sName, string sDescription, string type, string sValue);
	Property(string sName, string sDescription, string sValue);
	Property(string sName, string sDescription, int iValue);
	Property(string sName, string sDescription, float fValue);
	Property(string sName, string sDescription, bool bValue);
	virtual ~Property() {};

	string getName() const;
	string getDescription() const;

	bool getBoolValue() const;
	int getIntValue() const;
	string getStringValue() const;
	float getFloatValue() const;

	string getType() const;

	string getValueAsString() const;


	void setName(string name);
	void setDescription(string description);

	void setBoolValue(bool value);
	void setIntValue(int value);
	void setStringValue(string value);
	void setFloatValue(float value);
	void setValue(std::string value);

	virtual Json::Value toJSON() const;

	virtual bool operator==(const std::string& name);
	virtual bool operator==(const Property& property);

	virtual bool operator!=(const std::string& name);
	virtual bool operator!=(const Property& property);

private:
	string name;
	string description;
	string type;

	string sValue;
	int iValue;
	bool bValue;
	float fValue;

};

} /* namespace alarmpi */

#endif /* PROPERTY_H_ */
