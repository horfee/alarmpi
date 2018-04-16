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

Json::Value Action::toJSON() const {
	Json::Value res;
	res["name"] = name;
	res["nextaction"] = nextAction == NULL ? "" : nextAction->name;
	res["type"] = getType();
	res["synchronous"] = synchronous;
	return res;
}

} /* namespace alarmpi */
