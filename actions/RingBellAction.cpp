/*
 * RingBellAction.cpp
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#include "RingBellAction.h"
#include <thread>
#include <typeinfo>
#include <stdarg.h>
#include <unistd.h>
#include <algorithm>
#include "../StringUtils.h"

namespace alarmpi {

RingBellAction::RingBellAction(std::string name, int duration, std::string deviceIds, bool synchronous): Action(name, synchronous) {
	this->duration = duration;
	targets = deviceIds;
}

std::string RingBellAction::getParams() const {
	return targets;
}

int RingBellAction::getDuration() const {
	return duration;
}

RingBellAction::~RingBellAction() {
}

Json::Value RingBellAction::toJSON() const {
	Json::Value v = Action::toJSON();
	v["duration"] = duration;
	v["targetIds"] = targets;
	return v;

}

std::string RingBellAction::getType() const {
	return "RINGBELLACTION";
}

void RingBellAction::execute(Device* device, Mode* mode) {

	std::vector<Device*> devices;
	if ( !targets.empty() ) {
		auto dev = system->getDevices();
		std::copy_if(dev.begin(), dev.end(), std::back_inserter(devices), [=](const Device* dev) { return stringContains(targets, std::to_string(dev->getId())); });
	} else {
		devices = system->getDevices();
	}
	for(Device* dev : devices) {
		if ( typeid(*dev) == typeid (BellDevice) ) {
			this->system->sendRFMessage((BellDevice*)dev,((BellDevice*)dev)->getOnValue());
		}
	}
	usleep(duration * 1000 * 60);
	for(Device* dev : devices) {
		if ( typeid(*dev) == typeid (BellDevice) ) {
			this->system->sendRFMessage((BellDevice*)dev,((BellDevice*)dev)->getOffValue());
		}
	}
}

} /* namespace alarmpi */
