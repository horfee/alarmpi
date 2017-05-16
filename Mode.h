/*
 * AlarmMode.h
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#ifndef MODE_H_
#define MODE_H_
#include <string>
#include "json/json.h"
#include <iostream>

namespace alarmpi {

//	static const std::string ActiveMode = "Active";
//	static const std::string InactiveMode = "Inactive";
//	static const std::string ConfigMode = "Configuration";

	typedef enum {
		Active = 1,
		Inactive = 2,
		Configuration = 3
	} ModeType;

class Mode {
		friend std::ostream & operator<<(std::ostream &os, const Mode& m);
public:
		Mode(std::string name, std::string description, ModeType type);
		virtual ~Mode();

		std::string getName() const;

		ModeType getType() const;

		std::string getDescription() const;
		void setDescription(std::string description);

		virtual Json::Value toJSON() const;

		virtual bool operator==(const std::string& name);
		virtual bool operator==(const Mode& mode2);

		virtual bool operator!=(const std::string& name);
		virtual bool operator!=(const Mode& mode2);

private:
		std::string name;
		std::string description;
		ModeType type;
};

}


#endif /* MODE_H_ */
