/*
 * CallPhoneAction.cpp
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#include "CallPhoneAction.h"

namespace alarmpi {

CallPhoneAction::CallPhoneAction(std::string name, bool synchronous): Action(name, synchronous) {
}

CallPhoneAction::~CallPhoneAction() {
}

void CallPhoneAction::execute(Device* device, Mode* mode) {
//	if ( !stopAsked() ) {
//		setRunning();
		this->system->callPhones();
//	}
//	Action::execute(device, mode);
}

std::string CallPhoneAction::getType() const {
	return "CALLPHONEACTION";
}

} /* namespace alarmpi */
