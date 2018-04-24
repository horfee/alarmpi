/*
 * ActivateModeAction.h
 *
 *  Created on: 16 juin 2017
 *      Author: horfee
 */

#ifndef ACTIONS_ACTIVATEMODEACTION_H_
#define ACTIONS_ACTIVATEMODEACTION_H_

#include "Action.h"

namespace alarmpi {

class ActivateModeAction: public Action {
public:
	ActivateModeAction(std::string name, std::string mode);

	virtual ~ActivateModeAction();

	virtual void execute(Device* device, Mode* mode);

	Mode* getMode() const;

	virtual std::string getType() const;

	virtual Json::Value toJSON() const;

private:

	std::string mode;
};

} /* namespace alarmpi */

#endif /* ACTIONS_ACTIVATEMODEACTION_H_ */
