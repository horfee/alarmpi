/*
 * Action.h
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#ifndef ACTION_H_
#define ACTION_H_

#include <string>
#include <mutex>
#include "../AlarmSystem.h"
#include "../devices/Device.h"
#include "../json/json.h"
#include "../Mode.h"

namespace alarmpi {

class AlarmSystem;

class Action {
	friend class AlarmSystem;
public:
	Action(std::string name, bool synchronous);
	virtual ~Action();

	Action* getNextAction() const;
	void setNextAction(Action* nextAction);

	virtual void execute(Device* device, Mode* mode) = 0;

	bool isSynchronous() const;
//	void stop();
//
//	bool isRunning();
//
//	bool stopAsked();

	std::string getName() const;

	virtual std::string getType() const = 0;

	virtual Json::Value toJSON() const;

protected:
	AlarmSystem* system;

	//void setRunning();

	//void clearMustStop();

private:
	Action* nextAction;
	std::string name;
	bool synchronous;
//	bool mustStop;
//	bool running;
//
//	std::mutex mustStopMutex;
//	std::mutex runningMutex;
};

} /* namespace alarmpi */

#endif /* ACTION_H_ */
