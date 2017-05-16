/*
 * RingBellAction.h
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#ifndef RINGBELLACTION_H_
#define RINGBELLACTION_H_

#include "Action.h"
#include <vector>
#include <list>

namespace alarmpi {

class RingBellAction: public Action {
public:
	RingBellAction(std::string name,int duration, std::string deviceIds, bool synchronous);
	virtual ~RingBellAction();

	virtual void execute(Device* device, Mode* mode);

	void setDuration(int duration);
	int getDuration() const;

	std::string getParams() const;

	virtual std::string getType() const;
	virtual Json::Value toJSON() const;
private:

	void callBack();

	std::string targets;
	int duration;
};

} /* namespace alarmpi */

#endif /* RINGBELLACTION_H_ */
