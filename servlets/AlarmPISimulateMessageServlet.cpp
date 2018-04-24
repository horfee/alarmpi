/*
 * AlarmPISimulateMessageServlet.cpp
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#include "AlarmPISimulateMessageServlet.h"

#include <event2/http.h>
#include <iostream>
#include <regex>
#include <string>
#include <map>
#include <functional>
#include "../Utils.h"

#include "../actions/AllActions.h"
#include "../httpserver/HTTPRequest.h"
#include "../httpserver/HTTPResponse.h"
#include "../json/json.h"

namespace alarmpi {


AlarmPISimulateMessageServlet::AlarmPISimulateMessageServlet(AlarmSystem *system): AlarmPIServlet(system) {


}

AlarmPISimulateMessageServlet::~AlarmPISimulateMessageServlet() {
}


void AlarmPISimulateMessageServlet::doGet(HTTPRequest& request, HTTPResponse& response) {
	std::regex rex("/rest/message.+");
	if (std::regex_match(request.getURL(), rex)) {
		//std::string signal(request.getURL().substr(request.getURL().find_last_of('/') + 1));
		std::string message;
		if ( request.hasParameter("message") ) {
			message = request.getParameter("message");
		} else {
			message = request.getData();
		}


		system->sendSMS(urlDecode(message));
		response << "Ok";
		response.setContentType("application/json");
	}

}

void AlarmPISimulateMessageServlet::doPost(HTTPRequest& request, HTTPResponse& response) {
	this->doGet(request, response);

}

void AlarmPISimulateMessageServlet::doPut(HTTPRequest& request, HTTPResponse& response) {
	this->doGet(request, response);
}

void AlarmPISimulateMessageServlet::doDelete(HTTPRequest& request, HTTPResponse& response) {
	this->doGet(request, response);
}


} /* namespace alarmpi */
