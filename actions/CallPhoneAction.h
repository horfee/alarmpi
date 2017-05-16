/*
 * CallPhoneAction.h
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#ifndef CALLPHONEACTION_H_
#define CALLPHONEACTION_H_

#include "Action.h"

namespace alarmpi {

class CallPhoneAction: public Action {
public:
	CallPhoneAction(std::string name, bool synchronous);
	virtual ~CallPhoneAction();

	virtual void execute(Device* device, Mode* mode);

	virtual std::string getType() const;
};

} /* namespace alarmpi */

#endif /* CALLPHONEACTION_H_ */
