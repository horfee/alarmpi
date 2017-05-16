/*
 * ActivateAction.cpp
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#include "ActivateAction.h"
#include "../devices/ActionnableDevice.h"
#include <iostream>
#include "../StringUtils.h"
#include <algorithm>

namespace alarmpi {

ActivateAction::ActivateAction(std::string name, int valueToSend, std::string deviceIds, bool synchronous): Action(name, synchronous), targets(deviceIds), value(valueToSend) {


}

int ActivateAction::getValue() const {
	return value;
}

std::string ActivateAction::getTargets() const {
	return targets;
}

ActivateAction::~ActivateAction() {
}


void ActivateAction::execute(Device* device, Mode* mode) {
//	std::vector<std::string> targets = split(this->targets, {","});
	std::vector<Device*> devices;
	if ( !targets.empty() ) {
		std::copy_if(system->getDevices().begin(), system->getDevices().end(), devices.begin(), [&](const Device* dev) { return stringContains(targets, std::to_string(dev->getId())); });
	} else {
		devices = system->getDevices();
	}

	for(Device* dev : this->system->getDevices()) {
		if ( typeid(*dev) == typeid (ActionnableDevice) ) {
			this->system->sendRFMessage((ActionnableDevice*)dev,value);
		}
	}
}

Json::Value ActivateAction::toJSON() const {
	Json::Value res = Action::toJSON();
	res["value"] = value;
	res["targetIds"] = targets;
	return res;
}

std::string ActivateAction::getType() const {
	return "ACTIVATEACTION";
}

} /* namespace alarmpi */
