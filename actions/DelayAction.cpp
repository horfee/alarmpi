/*
 * ActivateAction.cpp
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#include "DelayAction.h"
#include <iostream>
namespace alarmpi {

DelayAction::DelayAction(std::string name, int delay): Action(name, true), delay(delay) {


}

int DelayAction::getDelay() const {
	return delay;
}

DelayAction::~DelayAction() {
}

void DelayAction::execute(Device* device, Mode* mode) {
//	if ( !stopAsked() ){
//		setRunning();
	std::this_thread::sleep_for(std::chrono::seconds(delay));
//	}
//	Action::execute(device, mode);
}

Json::Value DelayAction::toJSON() const {
	Json::Value v = Action::toJSON();
	v["delay"] = getDelay();
	return v;

}

std::string DelayAction::getType() const {
	return "DELAYACTION";
}

} /* namespace alarmpi */
