/*
 * AlarmPIActionsServlet.cpp
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#include "AlarmPISimulateSignalReceivedServlet.h"

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

namespace alarmpi {


AlarmPISimulateSignalReceivedServlet::AlarmPISimulateSignalReceivedServlet(AlarmSystem *system): AlarmPIServlet(system) {


}

AlarmPISimulateSignalReceivedServlet::~AlarmPISimulateSignalReceivedServlet() {
}


void AlarmPISimulateSignalReceivedServlet::doGet(HTTPRequest& request, HTTPResponse& response) {
	std::regex rex("/rest/signal/.+");
	if (std::regex_match(request.getURL(), rex)) {
		std::string signal(request.getURL().substr(request.getURL().find_last_of('/') + 1));
		system->simulateSignalReceived(signal);
		response << "Ok";
		response.setContentType("application/json");
	}

}

void AlarmPISimulateSignalReceivedServlet::doPost(HTTPRequest& request, HTTPResponse& response) {
	this->doGet(request, response);

}

void AlarmPISimulateSignalReceivedServlet::doPut(HTTPRequest& request, HTTPResponse& response) {
	this->doGet(request, response);
}

void AlarmPISimulateSignalReceivedServlet::doDelete(HTTPRequest& request, HTTPResponse& response) {
	this->doGet(request, response);
}


} /* namespace alarmpi */
