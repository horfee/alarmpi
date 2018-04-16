/*
 * AlarmPIPropertiesServlet.cpp
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#include "AlarmPIPropertiesServlet.h"
#include "../Utils.h"

namespace alarmpi {

AlarmPIPropertiesServlet::AlarmPIPropertiesServlet(AlarmSystem *system): AlarmPIServlet(system) {

}

AlarmPIPropertiesServlet::~AlarmPIPropertiesServlet() {
}


void AlarmPIPropertiesServlet::doGet(HTTPRequest& request, HTTPResponse& response) {
	Json::Value properties(Json::arrayValue);

	for(Property* property : system->getProperties()) {
		if ( *property != std::string("MODE") && *property != std::string("PASSWORD") ) {
			properties.append(property->toJSON());
		}
	}

	response << properties;
	response.setContentType("application/json");
}

void AlarmPIPropertiesServlet::doPost(HTTPRequest& request, HTTPResponse& response) {
	Json::Value value;
	Json::Reader reader;
	std::string idToAdd = request.getURL().substr(std::string("/rest/properties/").size());
	bool parsingSuccessful = reader.parse( request.getData(), value);

	if ( !parsingSuccessful ) {
		response.setCode(httpBadRequest);
	} else {
		Property* prop = system->getProperty(idToAdd);
		if ( prop != NULL ) {
			value.clear();
			value["error"] = "La propriété " + idToAdd + " existe déjà.";
			response.setCode(httpBadRequest);
		} else {
			std::string name = value["name"].asString();
			std::string description = value["description"].asString();
			std::string type = value["type"].asString();
			std::string val = value["value"].asString();

			if ( !system->isConfigMode() ) {
				response.setCode(httpUnauthorized);
				value.clear();
				value["error"] = "Vous devez activer le mode configuration.";
			} else {
				prop = new Property(name, description, type, val);
				system->addProperty(prop);
				response.setCode(httpOK);
				value = prop->toJSON();
			}
		}
	}
	response.setContentType("application/json");
}

void AlarmPIPropertiesServlet::doPut(HTTPRequest& request, HTTPResponse& response) {
	Json::Value value;
	Json::Reader reader;
	std::string idToAlter = request.getURL().substr(std::string("/rest/properties/").size());
	bool parsingSuccessful = reader.parse( request.getData(), value);

	logMessage( LOG_DEBUG, "Parsed : %d", parsingSuccessful);
	if ( !parsingSuccessful ) {
		response.setCode(httpBadRequest);
	} else {
		if ( system->isConfigMode() ) {
			Property* property = system->getProperty(value["name"].asString());
			property->setDescription(value["description"].asString());
			property->setValue(value["value"].asString());
			value = property->toJSON();
			response.setCode(httpOK);
		} else {
			response.setCode(httpBadRequest);
			value.clear();
			value["error"] = "Vous devez activer le mode configuration";
		}

	}
	response << value;
	response.setContentType("application/json");
}

void AlarmPIPropertiesServlet::doDelete(HTTPRequest& request, HTTPResponse& response) {
	Json::Value val;
	std::string idToRemove = request.getURL().substr(std::string("/rest/properties/").size());
	if ( idToRemove.size() == 0) {
		response.setCode(httpBadRequest);
	} else {
		Property* prop = system->getProperty(idToRemove);
		if ( prop == NULL ) {
			response.setCode(httpNotFound);
			val["error"] = "La propriété " + idToRemove + " n'existe pas.";
		} else {
			try {
				system->removeProperty(prop);
				delete prop;
			} catch (const std::exception& e) {
				val["error"] = e.what();
				response.setCode(httpBadRequest);
			}
		}
	}
	response << val;
	response.setContentType("application/json");
}



} /* namespace alarmpi */
