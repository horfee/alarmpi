/*
 * AlarmPISystemServlet.cpp
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#include "AlarmPIPingServlet.h"

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


AlarmPIPingServlet::AlarmPIPingServlet(AlarmSystem *system): AlarmPIServlet(system) {

}

AlarmPIPingServlet::~AlarmPIPingServlet() {
}


void AlarmPIPingServlet::doGet(HTTPRequest& request, HTTPResponse& response) {
	response << "pong";
	response.setContentType("application/json");
}

void AlarmPIPingServlet::doPost(HTTPRequest& request, HTTPResponse& response) {
	this->doGet(request, response);
}

void AlarmPIPingServlet::doPut(HTTPRequest& request, HTTPResponse& response) {
	this->doGet(request, response);
}

void AlarmPIPingServlet::doDelete(HTTPRequest& request, HTTPResponse& response) {
	this->doGet(request, response);
}


} /* namespace alarmpi */
