/*
 * AlarmPIPhonesServlet.cpp
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#include "AlarmPIPhonesServlet.h"
#include <regex>

namespace alarmpi {

AlarmPIPhonesServlet::AlarmPIPhonesServlet(AlarmSystem *system): AlarmPIServlet(system) {
}

AlarmPIPhonesServlet::~AlarmPIPhonesServlet() {
}


void AlarmPIPhonesServlet::doGet(HTTPRequest& request, HTTPResponse& response) {
	std::string activeMode = system->activeMode();
	std::regex rex("/rest/phones/.+");
	if (std::regex_match(request.getURL(), rex)) {
		std::string phone(request.getURL().substr(request.getURL().find_last_of('/') + 1));
		Json::Value phoneValue;
		phoneValue["phoneNumber"] = phone;
		phoneValue["index"] = system->getPhone(phone);
		response << phoneValue;
	} else {
		Json::Value modes(Json::arrayValue);
		int i = 1;
		for (auto it : system->getPhones()) {
			Json::Value m;
			m["phoneNumber"] = it;
			m["index"] = i++;
			modes.append(m);
		}
		response << modes;
	}

	response.setContentType("application/json");
}

void AlarmPIPhonesServlet::doPost(HTTPRequest& request, HTTPResponse& response) {
	Json::Value res;
	if ( !system->isConfigMode() ) {
		res["error"] = "Vous devez activer le mode configuration.";
		response << res;
		response.setCode(httpUnauthorized);
	} else {
		Json::Value value;
		Json::Reader reader;
		std::string idToAdd = request.getURL().substr(std::string("/rest/phones/").size());
		bool parsingSuccessful = reader.parse(request.getData(), value);

		if (!parsingSuccessful) {
			response.setCode(httpBadRequest);
		} else {
			if (system->getPhone(idToAdd) != -1) {
				res["error"] = "Le numéro de téléphone " + idToAdd + " existe déjà.";
				response << res;
				response.setCode(httpForbidden);
			} else {
				if (value["phoneNumber"].asString() == "" || value["index"].asString() == "") {
					response.setCode(httpBadRequest);
					res["error"] = "Le numéro et la position sont obligatoires.";
					response << value;
				} else {
					std::string phoneNumber = value["phoneNumber"].asString();
					int index = value["index"].asInt();
					std::cout << "post : Index recu : " + std::to_string(index) << std::endl;
					system->addPhoneNumber(phoneNumber, index - 1);
					response.setCode(httpOK);
					res["phoneNumber"] = phoneNumber;
					res["index"] = index;
				}

			}
		}
	}
	response << res;
	response.setContentType("application/json");
}

void AlarmPIPhonesServlet::doPut(HTTPRequest& request, HTTPResponse& response) {
	Json::Value res;
	if ( !system->isConfigMode() ) {
		res["error"] = "Vous devez activer le mode configuration.";
		response << res;
		response.setCode(httpUnauthorized);
	} else {
		Json::Value value;
		Json::Reader reader;
		std::string idToAdd = request.getURL().substr(std::string("/rest/phones/").size());
		bool parsingSuccessful = reader.parse(request.getData(), value);

		if (!parsingSuccessful) {
			response.setCode(httpBadRequest);
		} else {
			if (system->getPhone(idToAdd) == -1) {
				res["error"] = "Le numéro de téléphone " + idToAdd + " n'existe pas.";
				response << res;
				response.setCode(httpNotFound);
			} else {
				if (value["phoneNumber"].asString() == "" || value["index"].asString() == "") {
					response.setCode(httpBadRequest);
					res["error"] = "Le numéro et la position sont obligatoires.";
					response << value;
				} else {
					std::string phoneNumber = value["phoneNumber"].asString();
					int index = value["index"].asInt();
					std::cout << "put Index recu : " + std::to_string(index) << std::endl;
					system->addPhoneNumber(phoneNumber, index - 1);
					response.setCode(httpOK);
					res["phoneNumber"] = phoneNumber;
					res["index"] = index;
				}
			}
		}
	}
	response << res;
	response.setContentType("application/json");
}

void AlarmPIPhonesServlet::doDelete(HTTPRequest& request, HTTPResponse& response) {
	Json::Value res;
	if ( !system->isConfigMode() ) {
		res["error"] = "Vous devez activer le mode configuration.";
		response << res;
		response.setCode(httpUnauthorized);
	} else {
		Json::Value val;
		std::string idToRemove = request.getURL().substr(std::string("/rest/phones/").size());
		if (idToRemove.size() == 0) {
			response.setCode(httpBadRequest);
		} else {
			int index = system->getPhone(idToRemove);
			if (index == -1) {
				response.setCode(httpNotFound);
				res["error"] = "Le numéro de téléphone " + idToRemove + " n'existe pas.";
			} else {
				system->removePhoneNumber(idToRemove);
			}
		}
	}

	response << res;
	response.setContentType("application/json");
}
} /* namespace alarmpi */
