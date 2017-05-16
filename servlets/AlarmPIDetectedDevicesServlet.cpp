/*
 * AlarmPIDevicesServlet.cpp
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#include "AlarmPIDetectedDevicesServlet.h"
#include "../devices/AllDevices.h"
#include <regex>

namespace alarmpi {

AlarmPIDetectedDevicesServlet::AlarmPIDetectedDevicesServlet(AlarmSystem *system): AlarmPIServlet(system) {

}

AlarmPIDetectedDevicesServlet::~AlarmPIDetectedDevicesServlet() {
}


void AlarmPIDetectedDevicesServlet::doGet(HTTPRequest& request, HTTPResponse& response) {
	char buf[sizeof "XXXX-XX-XXTXX:XX:XXZ"];
	Json::Value devices(Json::arrayValue);
	for(std::tuple<int, time_t> detectedDevice : system->getDetectedDevices()) {
		Json::Value tmp;
		tmp["deviceId"] = std::get<0>(detectedDevice);

		strftime(buf, sizeof buf, "%FT%TZ", gmtime(&std::get<1>(detectedDevice)));
		tmp["timestamp"] = std::string(buf);
		devices.append(tmp);
	}
	response << devices;

	response.setContentType("application/json");
}

void AlarmPIDetectedDevicesServlet::doPost(HTTPRequest& request, HTTPResponse& response) {
	response.setCode(httpForbidden);
	response.setContentType("application/json");
}

void AlarmPIDetectedDevicesServlet::doPut(HTTPRequest& request, HTTPResponse& response) {
	response.setCode(httpForbidden);
	response.setContentType("application/json");
}

void AlarmPIDetectedDevicesServlet::doDelete(HTTPRequest& request, HTTPResponse& response) {
	response.setCode(httpForbidden);
	response.setContentType("application/json");
};


} /* namespace alarmpi */
