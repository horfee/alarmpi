/*
 * Action.cpp
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#include "Action.h"
#include <iostream>

namespace alarmpi {

Action::Action(std::string name, bool synchronous): name(name), synchronous(synchronous) {
	nextAction = NULL;
	system = NULL;
//	mustStop = false;
//	running = false;

}

Action::~Action() {
	//delete nextAction;
}

bool Action::isSynchronous() const {
	return synchronous;
}

std::string Action::getName() const {
	return name;
}

void Action::setNextAction(Action* action) {
	nextAction = action;
}

Action* Action::getNextAction() const {
	return nextAction;
}

//void Action::execute(Device* device, Mode* mode) {
//	std::cout << "Executing action " << this->getName() << "(" << mode->getName() << ")" << std::endl;
//	if ( !stopAsked() && !synchronous ) {
//		thread t([&]() {
//			if ( this->nextAction != NULL ) {
//				this->nextAction->execute(device, mode);
//			}
//			this->clearMustStop();
//		});
//		t.detach();
//	} else if ( !stopAsked() ) {
//		if ( this->nextAction != NULL ) {
//			this->nextAction->execute(device, mode);
//		}
//	}
//	this->clearMustStop();
//	runningMutex.lock();
//	running = false;
//	runningMutex.unlock();
//}
//
//void Action::clearMustStop() {
//	mustStopMutex.lock();
//	mustStop = false;
//	mustStopMutex.unlock();
//}
//
//void Action::stop() {
//	if ( !isRunning() ) return;
//	mustStopMutex.lock();
//	mustStop = true;
//	mustStopMutex.unlock();
//}
//
//bool Action::isRunning() {
//	runningMutex.lock();
//	bool r = running;
//	runningMutex.unlock();
//	return r;
//}
//
//void Action::setRunning() {
//	runningMutex.lock();
//	running = true;
//	runningMutex.unlock();
//}
//
//bool Action::stopAsked() {
//	bool res;
//	mustStopMutex.lock();
//	res = mustStop;
//	mustStopMutex.unlock();
//	return res;
//}

Json::Value Action::toJSON() const {
	Json::Value res;
	res["name"] = name;
	res["nextaction"] = nextAction == NULL ? "" : nextAction->name;
	res["type"] = getType();
	res["synchronous"] = synchronous;
	return res;
}

} /* namespace alarmpi */
