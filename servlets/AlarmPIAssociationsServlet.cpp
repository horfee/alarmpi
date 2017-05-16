/*
 * AlarmPIAssociationsServlet.cpp
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#include "AlarmPIAssociationsServlet.h"
#include <regex>
#include <tuple>

namespace alarmpi {

AlarmPIAssociationsServlet::AlarmPIAssociationsServlet(AlarmSystem *system): AlarmPIServlet(system) {
}

AlarmPIAssociationsServlet::~AlarmPIAssociationsServlet() {
}


void AlarmPIAssociationsServlet::doGet(HTTPRequest& request, HTTPResponse& response) {
	std::regex rex("/rest/associations/.+");
	if (std::regex_match(request.getURL(), rex)) {
		std::string assocName(request.getURL().substr(request.getURL().find_last_of('/') + 1));
		int deviceId = atoi(assocName.substr(0, assocName.find("-")).c_str());
		std::string mode(assocName.substr(assocName.find("-") + 1));
		Action* action = system->getAssociation(deviceId, mode);
		Json::Value assoc;

		assoc["deviceId"]	= deviceId;
		assoc["mode"]		= mode;
		assoc["action"]		= action->getName();
		assoc["name"]		= std::to_string(deviceId) + "-" + mode;

		response << assoc;
		response.setCode(httpOK);
	} else {
		Json::Value associations(Json::arrayValue);

		int pDeviceId = atoi(("0" + request.getParameter("deviceId")).c_str());
		std::string pMode = request.getParameter("mode");
		std::string pAction = request.getParameter("action");

		std::vector<std::tuple<Device*, Mode*, Action*>> sysAssoc = system->getAssociations();
		std::vector<std::tuple<Device*, Mode*, Action*>> res;

		std::for_each(sysAssoc.begin(), sysAssoc.end(), [&](std::tuple<Device*, Mode*, Action*> elt) {
			Device* device = std::get<0>(elt);
			Mode* mode = std::get<1>(elt);
			Action* action = std::get<2>(elt);

			if ( pDeviceId == 0 && pMode == "" && pAction == "" ) { res.push_back(elt); return; }
			if ( pDeviceId == 0 && pMode == "" && pAction == action->getName() ) { res.push_back(elt); return; }
			if ( pDeviceId == 0 && pMode == mode->getName() && pAction == "" ) { res.push_back(elt); return; }
			if ( pDeviceId == 0 && pMode == mode->getName() && pAction == action->getName()) { res.push_back(elt); return; }
			if ( pDeviceId == device->getId() && pMode == "" && pAction == "") { res.push_back(elt); return; }
			if ( pDeviceId == device->getId() && pMode == "" && pAction == action->getName()) { res.push_back(elt); return; }
			if ( pDeviceId == device->getId() && pMode == mode->getName() && pAction == "") { res.push_back(elt); return; }

		});

		for(auto elt : res) {
			Device* device = std::get<0>(elt);
			Mode* mode = std::get<1>(elt);
			Action* action = std::get<2>(elt);

			Json::Value assoc;
			assoc["deviceId"]	= device->getId();
			assoc["mode"]		= mode->getName();
			assoc["action"]		= action->getName();
			assoc["name"]		= std::to_string(device->getId()) + "-" + mode->getName();

			associations.append(assoc);
		}
		response << associations;
		response.setCode(httpOK);
	}

	response.setContentType("application/json");
}

void AlarmPIAssociationsServlet::doPost(HTTPRequest& request, HTTPResponse& response) {
	Json::Value res;
	if ( !system->isConfigMode() ) {
		res["error"] = "Vous devez activer le mode configuration.";
		response << res;
		response.setCode(httpUnauthorized);
	} else {
		Json::Value value;
		Json::Reader reader;
		std::string idToAdd = request.getURL().substr(std::string("/rest/associations").size());
		if ( idToAdd[0] == '/' ) idToAdd = idToAdd.substr(1);
		bool parsingSuccessful = reader.parse(request.getData(), value);
		if (!parsingSuccessful) {
			response.setCode(httpBadRequest);
		} else {
			int deviceId = value["deviceId"].asInt();
			std::string sMode = value["mode"].asString();
			std::string sAction = value["action"].asString();

			Device* device = system->getDevice(deviceId);
			Mode* mode = system->getMode(sMode);
			Action* action = system->getAction(sAction);
			Action* action2 = system->getAssociation(deviceId, sMode);

			if ( device == NULL ) {
				value["error"] = "Le capteur " + std::to_string(deviceId) + " n'a pas été trouvé.";
				response << value;
				response.setCode(httpNotFound);
			} else if ( mode == NULL ) {
				value["error"] = "Le mode " + sMode + " n'a pas été trouvé.";
				response << value;
				response.setCode(httpNotFound);
			} else if ( action == NULL ){
				value["error"] = "L'action " + sAction + " n'a pas été trouvée.";
				response << value;
				response.setCode(httpNotFound);
			} else if ( action2 != NULL ) {
				value["error"] = "Le capteur " + std::to_string(deviceId)  + " est déjà associé au mode " + sMode + "("+ action2->getName() + ").";
				response << value;
				response.setCode(httpBadRequest);
			} else {
				system->associateActionAndMode(device, action, mode);
				value["deviceId"]	= device->getId();
				value["mode"]		= mode->getName();
				value["action"]		= action->getName();
				value["name"]		= std::to_string(device->getId()) + "-" + mode->getName();
				response << value;
				response.setCode(httpOK);
			}
		}
	}
	response.setContentType("application/json");
}

void AlarmPIAssociationsServlet::doPut(HTTPRequest& request, HTTPResponse& response) {
	Json::Value res;
	if ( !system->isConfigMode() ) {
		res["error"] = "Vous devez activer le mode configuration.";
		response << res;
		response.setCode(httpUnauthorized);
	} else {
		Json::Value value;
		Json::Reader reader;
		std::string idToAlter = request.getURL().substr(std::string("/rest/associations/").size());
		bool parsingSuccessful = reader.parse(request.getData(), value);
		if (!parsingSuccessful) {
			response.setCode(httpBadRequest);
		} else {
			int deviceId = value["deviceId"].asInt();
			std::string sMode = value["mode"].asString();
			std::string sAction = value["action"].asString();

			Device* device = system->getDevice(deviceId);
			Mode* mode = system->getMode(sMode);
			Action* action = system->getAction(sAction);
			Action* action2 = system->getAssociation(deviceId, sMode);

			if ( device == NULL ) {
				value["error"] = "Le capteur " + std::to_string(deviceId) + " n'a pas été trouvé.";
				response << value;
				response.setCode(httpBadRequest);
			} else if ( mode == NULL ) {
				value["error"] = "Le mode " + sMode + " n'a pas été trouvé.";
				response << value;
				response.setCode(httpBadRequest);
			} else if ( action == NULL ){
				value["error"] = "L'action " + sAction + " n'a pas été trouvée.";
				response << value;
				response.setCode(httpBadRequest);
			} else if ( action2 == NULL ) {
				value["error"] = "L'association n'existe pas pour le capteur " + std::to_string(deviceId) + " et le mode " + sMode + ".";
				response << value;
				response.setCode(httpBadRequest);
			} else {
				if ( action != action2 ) {
					system->associateActionAndMode(device, action, mode);
				}

				value["deviceId"]	= device->getId();
				value["mode"]		= mode->getName();
				value["action"]		= action->getName();
				value["name"]		= std::to_string(device->getId()) + "-" + mode->getName();
				response << value;
				response.setCode(httpOK);
			}
		}
	}
	response.setContentType("application/json");
}

void AlarmPIAssociationsServlet::doDelete(HTTPRequest& request, HTTPResponse& response) {
	Json::Value res;
	if ( !system->isConfigMode() ) {
		res["error"] = "Vous devez activer le mode configuration.";
		response << res;
		response.setCode(httpUnauthorized);
	} else {
		Json::Value value;
		Json::Reader reader;
		std::string idToDelete = request.getURL().substr(std::string("/rest/associations/").size());
		int deviceId = atoi(idToDelete.substr(0, idToDelete.find("-")).c_str());
		std::string sMode(idToDelete.substr(idToDelete.find("-") + 1));

		Device* device = system->getDevice(deviceId);
		Mode* mode = system->getMode(sMode);
		if ( device == NULL ) {
			value["error"] = "Le capteur " + std::to_string(deviceId) + " n'a pas été trouvé.";
			response << value;
			response.setCode(httpBadRequest);
		} else if ( mode == NULL ) {
			value["error"] = "Le mode " + sMode + " n'a pas été trouvé.";
			response << value;
			response.setCode(httpBadRequest);
		} else {
			system->removeAssociation(device, mode);
			response.setCode(httpOK);
		}
	}
	response.setContentType("application/json");
}

} /* namespace alarmpi */
