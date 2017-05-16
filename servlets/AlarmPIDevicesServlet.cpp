/*
 * AlarmPIDevicesServlet.cpp
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#include "AlarmPIDevicesServlet.h"
#include "../devices/AllDevices.h"
#include <regex>

namespace alarmpi {

std::map<std::string,std::function<Device*(Json::Value)>> deviceCreators;

AlarmPIDevicesServlet::AlarmPIDevicesServlet(AlarmSystem *system): AlarmPIServlet(system) {
	deviceCreators["BELL"] = [](Json::Value value){
		int id = value["deviceId"].asInt();
		std::string description = value["description"].asString();
		int on = value["on"].asInt();
		int off = value["off"].asInt();
		return new BellDevice(id, description, on, off);
	};
	deviceCreators["MOTION"] = [](Json::Value value){
		int id = value["deviceId"].asInt();
		std::string description = value["description"].asString();
		return new MotionDevice(id, description);
	};
	deviceCreators["MAGNETIC"] = [](Json::Value value){
		int id = value["deviceId"].asInt();
		std::string description = value["description"].asString();
		return new MagneticDevice(id, description);
	};
	deviceCreators["REMOTE"] = [](Json::Value value){
		int id = value["deviceId"].asInt();
		std::string description = value["description"].asString();
		std::string commands = (value.isMember("commands") ? std::string(""): value["commands"].asString());
		return new RemoteDevice(id, description, commands);
	};
}

AlarmPIDevicesServlet::~AlarmPIDevicesServlet() {
}


void AlarmPIDevicesServlet::doGet(HTTPRequest& request, HTTPResponse& response) {
	std::regex rex("/rest/devices/.+");
	if ( std::regex_match(request.getURL(), rex) ) {
		std::string strDeviceId(request.getURL().substr(request.getURL().find_last_of('/') + 1));
		int deviceId = atoi(strDeviceId.c_str());
		Json::Value m = system->getDevice(deviceId)->toJSON();
		response << m;
	} else {

		// TODO : implements filter on devices details
		Json::Value devices(Json::arrayValue);
		for(auto it : system->getDevices()) {
			devices.append(it->toJSON());
		}
		response << devices;
	}

	response.setContentType("application/json");
}

void AlarmPIDevicesServlet::doPost(HTTPRequest& request, HTTPResponse& response) {

	if ( !system->isConfigMode() ) {
		Json::Value res;
		res["error"] = "Vous devez activer le mode configuration.";
		response << res;
		response.setCode(httpUnauthorized);
	} else {
		Json::Value value;
		Json::Reader reader;
		bool parsingSuccessful = reader.parse(request.getData(), value);

		if (!parsingSuccessful) {
			response.setCode(httpBadRequest);
		} else {
			int idToAdd = atoi(request.getURL().substr(std::string("/rest/devices/").size()).c_str());
			Device* device = system->getDevice(idToAdd);
			if (device != NULL) {
				value.clear();
				value["error"] = "Le capteur " + std::to_string(idToAdd) + " existe déjà.";
				response.setCode(httpBadRequest);
			} else {
				if (value["deviceId"].asString() == "" || value["description"].asString() == "" ) {
					response.setCode(httpBadRequest);
					value.clear();
					value["error"] = "L'identifiant et la description sont obligatoires.";
				} else {
					auto creator = deviceCreators[value["type"].asString()];
					if ( creator != NULL ) {
						device = creator(value);
						system->addDevice(device);
						value = device->toJSON();
					}
					response.setCode(httpOK);
					response << value;
				}
			}
		}
	}

	response.setContentType("application/json");
}

void AlarmPIDevicesServlet::doPut(HTTPRequest& request, HTTPResponse& response) {

	Json::Value res;
	response.setContentType("application/json");

	if ( !system->isConfigMode() ) {
		res["error"] = "Vous devez activer le mode configuration.";
		response << res;
		response.setCode(httpUnauthorized);
	} else {
		Json::Value value;
		Json::Reader reader;
		bool parsingSuccessful = reader.parse(request.getData(), value);
		if (!parsingSuccessful) {
			response.setCode(httpBadRequest);
		} else {
			int idToAlter = atoi(request.getURL().substr(std::string("/rest/devices/").size()).c_str());
			Device* device = system->getDevice(idToAlter);
			if ( value["type"].asString() != device->getType() ) {
				auto creator = deviceCreators[value["type"].asString()];
				if ( creator != NULL ) {
					std::vector<Association> a = system->getAssociations();
					std::vector<Association> associations;
					std::copy_if(a.begin(), a.end(), associations.begin(), [&](Association t){
						return std::get<0>(t) == device;
					});

					system->removeDevice(device);
					delete device;
					device = creator(value);
					system->addDevice(device);
					for(Association a : associations) {
						Mode* mode = std::get<1>(a);
						Action* action = std::get<2>(a);
						system->associateActionAndMode(device, action, mode);
					}
					res = device->toJSON();
				}
			} else {
				device->setDescription(value["description"].asString());
				res = device->toJSON();
			}

			response.setCode(httpOK);
			response << res;
		}
	}


}

void AlarmPIDevicesServlet::doDelete(HTTPRequest& request, HTTPResponse& response) {
	response.setContentType("application/json");

	Json::Value value;
	if ( !system->isConfigMode()) {
		response.setCode(httpUnauthorized);
		value["error"] = "Vous devez activer le mode configuration";
		response << value;
	} else {
		std::string idToRemove = request.getURL().substr(std::string("/rest/devices/").size());
		if (idToRemove.size() == 0) {
			response.setCode(httpBadRequest);
		} else {
			Device* device = system->getDevice(atoi(idToRemove.c_str()));
			if (device == NULL) {
				response.setCode(httpBadRequest);
				value["error"] = "Le capteur " + idToRemove + " n'existe pas.";
			} else {
				system->removeDevice(device);
				delete device;
			}
		}
		response << value;
	}
};


} /* namespace alarmpi */
