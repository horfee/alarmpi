/*
 * ActivateAction.h
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#ifndef ACTIVATEACTION_H_
#define ACTIVATEACTION_H_

#include "Action.h"

namespace alarmpi {

class ActivateAction: public Action {
public:
	ActivateAction(std::string name, int valueToSend, std::string targetIds, bool synchronous);
	virtual ~ActivateAction();

	virtual void execute(Device* device, Mode* mode);

	int getValue() const;

	std::string getTargets() const;

	virtual std::string getType() const;

	virtual Json::Value toJSON() const;

private:
	std::string targets;
	int value;
};

} /* namespace alarmpi */

#endif /* ACTIVATEACTION_H_ */
