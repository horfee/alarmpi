/*
 * AlarmPIModesServlet.cpp
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#include "AlarmPIModesServlet.h"
#include "../json/json.h"
#include <regex>

namespace alarmpi {

AlarmPIModesServlet::AlarmPIModesServlet(AlarmSystem *alarmSystem):AlarmPIServlet(alarmSystem) {

}

AlarmPIModesServlet::~AlarmPIModesServlet() {

}

void AlarmPIModesServlet::doGet(HTTPRequest& request, HTTPResponse& response) {
	std::string activeMode = system->activeMode();
	std::regex rex("/rest/modes/.+");
	if (std::regex_match(request.getURL(), rex)) {
		std::string modeName(request.getURL().substr(request.getURL().find_last_of('/') + 1));
		Json::Value m = system->getMode(modeName)->toJSON();
		m["isActive"] = *(system->getMode(modeName)) == activeMode;
		response << m;
	} else {

		// Filter modes on name / isActive / Description
		Json::Value modes(Json::arrayValue);
		for (Mode* mode : system->getModes()) {
			Json::Value m = mode->toJSON();
			m["isActive"] = *mode == activeMode;
			modes.append(m);
		}
		response << modes;
	}

	response.setContentType("application/json");
}

void AlarmPIModesServlet::doPost(HTTPRequest& request, HTTPResponse& response) {
	Json::Value res;
	if ( !system->isConfigMode() ) {
		res["error"] = "Vous devez activer le mode configuration.";
		response << res;
		response.setCode(httpUnauthorized);
	} else {
		Json::Value value;
		Json::Reader reader;
		std::string idToAdd = request.getURL().substr(std::string("/rest/modes/").size());
		bool parsingSuccessful = reader.parse(request.getData(), value);

		if (!parsingSuccessful) {
			response.setCode(httpBadRequest);
		} else {
			Mode* mode = system->getMode(idToAdd);
			if (mode != NULL) {
				res["error"] = "Le mode " + idToAdd + " existe déjà.";
				response.setCode(httpBadRequest);
			} else {
				if (value["name"].asString() == "" || value["description"].asString() == "") {
					response.setCode(httpBadRequest);
					res["error"] = "Le nom et la description sont obligatoires.";
				} else {
					std::string name(value["name"].asString());
					std::string description(value["description"].asString());

					mode = new Mode(name, description, ModeType::Active);
					system->addMode(mode);
					response.setCode(httpOK);
					res = mode->toJSON();
				}
			}
		}
	}

	response << res;
	response.setContentType("application/json");
}


void AlarmPIModesServlet::doPut(HTTPRequest& request, HTTPResponse& response) {
	Json::Value res;
	if ( !system->isConfigMode() ) {
		Json::Value value;
		Json::Reader reader;
		std::string idToAlter = request.getURL().substr(std::string("/rest/modes/").size());
		bool parsingSuccessful = reader.parse(request.getData(), value);
		if (!parsingSuccessful) {
			response.setCode(httpBadRequest);
		} else {
			if ( !value["isActive"].asBool() ) {
				res["error"] = "Vous devez activer le mode configuration.";
				response.setCode(httpUnauthorized);
			} else {
				Mode* mode = system->getMode(idToAlter);
				if ( mode == NULL ) {
					response.setCode(httpBadRequest);
				} else {
					std::string password = request.getHeader("password");
					try {
						system->activateMode(mode->getName(), password);
					} catch(InvalidPasswordException& e) {
						response.setCode(httpUnauthorized);
						res["error"] = "Mode de passe invalide";
					}

				}
			}
		}

	} else {
		Json::Value value;
		Json::Reader reader;
		std::string idToAlter = request.getURL().substr(std::string("/rest/modes/").size());
		bool parsingSuccessful = reader.parse(request.getData(), value);
		if (!parsingSuccessful) {
			response.setCode(httpBadRequest);
		} else {
			Mode* am = system->getMode(value["name"].asString());
			am->setDescription(value["description"].asString());
			res = am->toJSON();
			response.setCode(httpOK);
		}
	}


	response << res;
	response.setContentType("application/json");
}

void AlarmPIModesServlet::doDelete(HTTPRequest& request, HTTPResponse& response) {
	Json::Value val;
	std::string idToRemove = request.getURL().substr(
			std::string("/rest/modes/").size());
	if (idToRemove.size() == 0) {
		response.setCode(httpBadRequest);
	} else {
		Mode* mode = system->getMode(idToRemove);
		if (mode == NULL) {
			response.setCode(httpNotFound);
			val["error"] = "Le mode " + idToRemove + " n'existe pas.";
		} else {
			if ( mode->getType() == ModeType::Inactive || mode->getType() == ModeType::Configuration) {
				val["error"] = "Impossible de supprimer le mode" + idToRemove + ".";
				response.setCode(httpUnauthorized);
			} else {
				try {
					system->removeMode(mode);
					delete mode;
				} catch (const std::exception& e) {
					val["error"] = e.what();
					response.setCode(httpBadRequest);
				}
			}
		}
	}
	response << val;
	response.setContentType("application/json");

}

} /* namespace alarmpi */
