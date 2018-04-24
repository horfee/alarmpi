/*
 * AlarmPISystemServlet.cpp
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#include "AlarmPISystemServlet.h"

#include <event2/http.h>
#include <iostream>
#include <regex>
#include <string>
#include <map>
#include <functional>

#include "../actions/AllActions.h"
#include "../httpserver/HTTPRequest.h"
#include "../httpserver/HTTPResponse.h"
#include "../json/json.h"
#include "../Utils.h"

namespace alarmpi {


AlarmPISystemServlet::AlarmPISystemServlet(AlarmSystem *system, bool* stopAsked, bool* rebootAsked): AlarmPIServlet(system) {
	this->stopAsked = stopAsked;
	this->rebootAsked = rebootAsked;

}

AlarmPISystemServlet::~AlarmPISystemServlet() {
}


void AlarmPISystemServlet::doGet(HTTPRequest& request, HTTPResponse& response) {
	Json::Value res;
	Json::Value wifis(Json::arrayValue);
	for(alarmpi::WifiDesc w : system->listWifi()) {
		Json::Value wn;
		wn["name"] = w.name;
		wn["strength"] = w.strength;
		wifis.append(wn);
	}
	Json::Value ipAddresses(Json::arrayValue);
	for(std::string w : system->ipAddresses()) {
		ipAddresses.append(w);
	}
	res["currentMode"] = system->activeMode();
	res["currentMode.description"] = system->getMode(system->activeMode())->getDescription();
	res["isConnectedToNetwork"] = system->isConnectedToNetwork();
	res["ipAddresses"] = ipAddresses;
	res["currentWifi"] = system->getCurrentESSI();
	res["wifiNetworks"] = wifis;
	res["version"] = system->getVersion();

	response << res;
	response.setContentType("application/json");
}

void AlarmPISystemServlet::doPost(HTTPRequest& request, HTTPResponse& response) {
	std::string action = request.getParameter("action");
	std::transform(action.begin(), action.end(), action.begin(), ::tolower);

	Json::Value value;
	Json::Reader reader;
	bool parsingSuccessful = request.getData().empty() || reader.parse(request.getData(), value);
	if ( !parsingSuccessful ) {
		response.setCode(httpBadRequest);
	} else {
		std::string encPassword = request.getHeader("password");//value["password"].asString();

		//logMessage( LOG_DEBUG, "Action   : %s", action.c_str());
		//logMessage( LOG_DEBUG, "Password : %s", encPassword.c_str());

		if ( !system->testPassword(encPassword) ) {
			response.setCode(httpUnauthorized);
			return;
		}

		if ( action == "login") {
			response.setCode(httpOK);
		} else if ( action == "shutdown" ) {
			*(this->stopAsked) = true;
			response.setCode(httpOK);
		} else if ( action == "reboot" ) {
			*(this->stopAsked) = true;
			*(this->rebootAsked ) = true;
			response.setCode(httpOK);
		} else if ( action == "activate" ) {
			std::string mode = value["mode"].asString();
			system->activateMode(mode, encPassword);
			response.setCode(httpOK);
		} else if ( action == "simulate") {
			int deviceId = value["deviceId"].asInt();
			int val = value["value"].asInt();
			BellDevice device(deviceId, "temp", val, 0);
			system->sendRFMessage(&device, val);
			response.setCode(httpOK);
		} else if ( action == "changepassword" ) {
			system->changePassword(encPassword, value["newPassword"].asString());
			response.setCode(httpOK);
		} else if ( action == "selectwifi" ) {
			std::string ssid = value["ssid"].asString();
			std::string wifiPassword = value["password"].asString();
			system->connectToWifi(ssid, wifiPassword);
		} else if ( action == "createAP") {
			std::string ssid = value["ssid"].asString();
			if ( ssid == "" ) ssid = "Alarm-PI";
			std::string password = value["password"].asString();
			system->createAccessPoint(ssid, password);
		}
	}

	response.setContentType("application/json");
}

void AlarmPISystemServlet::doPut(HTTPRequest& request, HTTPResponse& response) {
	response.setContentType("application/json");
}

void AlarmPISystemServlet::doDelete(HTTPRequest& request, HTTPResponse& response) {
	response.setContentType("application/json");
}


} /* namespace alarmpi */
