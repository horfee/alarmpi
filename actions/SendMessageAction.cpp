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
		std::string msg = message;
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

		pos = msg.find("$action.");
		if ( pos != std::string::npos ) {
			size_t pos2 = msg.find(" ", pos + 8);
			std::string val = msg.substr(pos + 8, pos2 - (pos + 8));
			Json::Value v = this->toJSON();
			Json::Value v2 = v[val];
			std::string res;
			if ( v2.type() == Json::ValueType::stringValue ) {
				res = v2.asString();
			} else if ( v2.type() == Json::ValueType::intValue ) {
				res = std::to_string(v2.asInt());
			} else if ( v2.type() == Json::ValueType::realValue ) {
				res = std::to_string(v2.asDouble());
			} else if ( v2.type() == Json::ValueType::booleanValue ) {
				res = std::to_string(v2.asBool());
			} else if ( v2.type() == Json::ValueType::uintValue ) {
				res = std::to_string(v2.asUInt());
			}
			msg = msg.substr(0, pos) + res + msg.substr(pos2);
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
