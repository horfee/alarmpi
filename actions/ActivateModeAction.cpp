/*
 * ActivateModeAction.cpp
 *
 *  Created on: 16 juin 2017
 *      Author: horfee
 */

#include "ActivateModeAction.h"

namespace alarmpi {

ActivateModeAction::ActivateModeAction(std::string name, std::string m): Action(name, false) {
	if ( m == "" ) throw "Invalid mode";
	mode = m;
}

ActivateModeAction::~ActivateModeAction() {
}

Mode* ActivateModeAction::getMode() const {
	return system->getMode(mode);
}

void ActivateModeAction::execute(Device* device, Mode* mode) {
	std::string encPassword = system->getPassword()->getStringValue();
	system->activateMode(this->mode,encPassword);
}

Json::Value ActivateModeAction::toJSON() const {
	Json::Value res = Action::toJSON();
	res["mode"] = mode;
	return res;
}

std::string ActivateModeAction::getType() const {
	return "ACTIVATEMODEACTION";
}

} /* namespace alarmpi */
