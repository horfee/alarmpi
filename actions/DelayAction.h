/*
 * ActivateAction.h
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#ifndef DELAYACTION_H_
#define DELAYACTION_H_

#include "Action.h"

namespace alarmpi {

class DelayAction: public Action {
public:
	DelayAction(std::string name, int delay);
	virtual ~DelayAction();

	virtual void execute(Device* device, Mode* mode);

	virtual std::string getType() const;

	Json::Value toJSON() const;

	int getDelay() const;

private:
	int delay;
};

} /* namespace alarmpi */

#endif /* ACTIVATEACTION_H_ */
