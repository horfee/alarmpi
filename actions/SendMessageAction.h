/*
 * SendMessageAction.h
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#ifndef SENDMESSAGEACTION_H_
#define SENDMESSAGEACTION_H_

#include "Action.h"

namespace alarmpi {

class SendMessageAction: public Action {
public:
	SendMessageAction(std::string name, std::string message, bool synchronous);
	virtual ~SendMessageAction();

	std::string getMessage() const;
	virtual void execute(Device* device, Mode* mode);

	virtual std::string getType() const;

	virtual Json::Value toJSON() const;
private:
	std::string message;
};

} /* namespace alarmpi */

#endif /* SENDMESSAGEACTION_H_ */
