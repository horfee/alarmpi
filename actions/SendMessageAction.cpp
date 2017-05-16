/*
 * SendMessageAction.cpp
 *
 *  Created on: 3 août 2015
 *      Author: horfee
 */

#include "SendMessageAction.h"
#include "../Properties.h"

#include <ctime>
#include <locale>

namespace alarmpi {

SendMessageAction::SendMessageAction(std::string name, std::string message, bool synchronous): Action(name, synchronous), message(message) {
}

SendMessageAction::~SendMessageAction() {
}

std::string SendMessageAction::getMessage() const {
	return message;
}

void SendMessageAction::execute(Device* device, Mode* mode) {
//	if ( !stopAsked() ) {
//		setRunning();
		size_t pos = message.find("$device");
		std::string msg;
		if ( pos != std::string::npos ) {
			msg = message.substr(0, pos) + device->getDescription() + message.substr(pos + std::string("$device").length());
		}
		pos = msg.find("$mode");
		if ( pos != std::string::npos ) {
			msg = msg.substr(0, pos) + mode->getName() + msg.substr(pos + std::string("$mode").length());
		}
		pos = msg.find("$time");
		if ( pos != std::string::npos ) {
			std::time_t t = std::time(NULL);
			char mbstr[100];
			std::strftime(mbstr, sizeof(mbstr), "%X", std::localtime(&t));
			msg = msg.substr(0, pos) + std::string(mbstr) + msg.substr(pos + std::string("$time").length());
		}
		pos = msg.find("$date");
		if ( pos != std::string::npos ) {
			std::time_t t = std::time(NULL);
			char mbstr[100];
			std::strftime(mbstr, sizeof(mbstr), "%x", std::localtime(&t));
			msg = msg.substr(0, pos) + std::string(mbstr) + msg.substr(pos + std::string("$date").length());
		}

//		if ( !stopAsked() ) {
			system->sendSMS(msg);
//		}
//	}
//	Action::execute(device, mode);
}

Json::Value SendMessageAction::toJSON() const {
	Json::Value res = Action::toJSON();
	res["message"] = message;
	return res;
}

std::string SendMessageAction::getType() const {
	return "SENDMESSAGEACTION";
}
} /* namespace alarmpi */
