/*
 * AlarmPIActionsServlet.cpp
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#include "AlarmPIActionsServlet.h"

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
#include "../AlarmSystemDAO.h"
#include "../DAOFactory.h"

namespace alarmpi {

std::map<std::string,std::function<Action*(Json::Value)>> actionCreators;

AlarmPIActionsServlet::AlarmPIActionsServlet(AlarmSystem *system): AlarmPIServlet(system) {

	actionCreators["ACTIVATEACTION"] = [](Json::Value v) {
		std::string id(trim(v["name"].asString()));
		bool synchronous = v["synchronous"].asBool();
		int valueToSend = v["value"].asInt();
		std::string targetIds(trim( v["targetIds"].asString()));
		return new ActivateAction(id, valueToSend, targetIds, synchronous);
	};
	actionCreators["ACTIVATEMODEACTION"] = [](Json::Value v) {
		std::string id(trim(v["name"].asString()));
//		bool synchronous = v["synchronous"].asBool();
		std::string mode = v["mode"].asString();
		return new ActivateModeAction(id, mode);
	};
	actionCreators["CALLPHONEACTION"] = [](Json::Value v) {
		std::string id(trim(v["name"].asString()));
		bool synchronous = v["synchronous"].asBool();
		return new CallPhoneAction(id, synchronous);
	};
	actionCreators["DELAYACTION"] = [](Json::Value v) {
		std::string id(trim(v["name"].asString()));
		int delay = v["delay"].asInt();
		return new DelayAction(id, delay);
	};
	actionCreators["RINGBELLACTION"] = [](Json::Value v) {
		std::string id(trim(v["name"].asString()));
		bool synchronous = v["synchronous"].asBool();
		int duration = v["duration"].asInt();
		std::string devices(trim(v["targetIds"].asString()));
		return new RingBellAction(id, duration, devices, synchronous);
	};
	actionCreators["SENDMESSAGEACTION"] = [](Json::Value v){
		std::string id(trim(v["name"].asString()));
		bool synchronous = v["synchronous"].asBool();
		std::string message(trim(v["message"].asString()));
		return new SendMessageAction(id, message, synchronous);
	};
}

AlarmPIActionsServlet::~AlarmPIActionsServlet() {
}


void AlarmPIActionsServlet::doGet(HTTPRequest& request, HTTPResponse& response) {
	std::regex rex("/rest/actions/.+");
	if (std::regex_match(request.getURL(), rex)) {
		std::string actionName(request.getURL().substr(request.getURL().find_last_of('/') + 1));
		Json::Value m = system->getAction(actionName)->toJSON();
		response << m;
	} else {
		// TODO : implements filters on TYPE, NAME
		Json::Value actions(Json::arrayValue);
		for(Action* action: system->getActions()) {
			actions.append(action->toJSON());
		}
		response << actions;
		response.setContentType("application/json");
	}

}

void AlarmPIActionsServlet::doPost(HTTPRequest& request, HTTPResponse& response) {
	Json::Value res;
	if ( !system->isConfigMode() ) {
		res["error"] = "Vous devez activer le mode configuration.";
		response << res;
		response.setCode(httpUnauthorized);
	} else {
		Json::Value value;
		Json::Reader reader;
		std::string idToAdd = request.getURL().substr(std::string("/rest/actions").size());
		if ( idToAdd[0] == '/') idToAdd = idToAdd.substr(1);
		bool parsingSuccessful = reader.parse(request.getData(), value);
		if (!parsingSuccessful) {
			response.setCode(httpBadRequest);
		} else {
			Action* action = system->getAction(idToAdd);
			if ( action != NULL ) {
				res["error"] = "L'action " + idToAdd + " existe déjà.";
				response << res;
				response.setCode(httpBadRequest);
			} else {
				std::string type = value["type"].asString();
				auto creator = actionCreators[type];
				if ( creator != NULL ) {
					action = creator(value);
					std::string next = value["nextaction"].asString();
					if ( next != "" ) {
						Action* nextAction = system->getAction(next);
						if ( nextAction == NULL ) {
							res["error"] = "L'action suivante " + next + " n'existe pas.";
							response << res;
							response.setCode(httpBadRequest);

							delete action;
						} else {
							action->setNextAction(nextAction);
							system->addAction(action);
							response.setCode(httpOK);
							value = action->toJSON();
							response << value;
						}
					} else {
						system->addAction(action);
						response.setCode(httpOK);
						value = action->toJSON();
						response << value;
					}

				} else {
					response.setCode(httpOK);
					response << value;
				}

			}
		}
	}

	response.setContentType("application/json");
}

void AlarmPIActionsServlet::doPut(HTTPRequest& request, HTTPResponse& response) {
	Json::Value res;
	if ( !system->isConfigMode() ) {
		res["error"] = "Vous devez activer le mode configuration.";
		response << res;
		response.setCode(httpUnauthorized);
	} else {
		Json::Value value;
		Json::Reader reader;
		std::string idToAlter = request.getURL().substr(std::string("/rest/actions/").size());
		bool parsingSuccessful = reader.parse(request.getData(), value);
		if (!parsingSuccessful) {
			response.setCode(httpBadRequest);
		} else {

			Action* action = system->getAction(idToAlter);
//			Action* origAction = action;

			if ( action == NULL ) {
				res["error"] = "L'action " + idToAlter + " n'existe pas.";
				response << res;
				response.setCode(httpBadRequest);
			} else {
				std::string type = value["type"].asString();
				auto creator = actionCreators[type];
				if ( creator != NULL ) {

					std::string next = value["nextaction"].asString();

					Action* nextAction = next == "" ? NULL : system->getAction(next);
					if ( next != "" && nextAction == NULL ) {
						res["error"] = "L'action suivante " + next + " n'existe pas.";
						response << res;
						response.setCode(httpBadRequest);
					} else {
						std::vector<std::tuple<int, std::string>> assocs;
						for(Association a : system->getAssociations()) {
							if ( std::get<2>(a) == action ) {
								std::tuple<int, std::string> t = std::make_tuple(std::get<0>(a)->getId(), std::get<1>(a)->getName());
								assocs.push_back(t);
							}
						}

						std::vector<Action*> chainedActions;
						for(auto it : system->getActions()) {
							if ( it->getNextAction() == action ) {
								chainedActions.push_back(it);
							}
						}
						system->removeAction(action);
						delete action;

						action = creator(value);
						action->setNextAction(nextAction);
						for(Action* a : chainedActions) {
							a->setNextAction(action);
						}
						system->addAction(action);
						for(std::tuple<int, std::string> t : assocs) {
							int devId = std::get<0>(t);
							std::string m = std::get<1>(t);
							system->associateActionAndMode(system->getDevice(devId), action, system->getMode(m));
						}
						for(Action* a : chainedActions) {
							DAOFactory::getInstance()->getDAO()->persistAction(a);
						}
						response.setCode(httpOK);
						value = action->toJSON();
						response << value;
					}


				} else {
					response.setCode(httpOK);
					response << value;
				}

			}
		}
	}

	response.setContentType("application/json");
}

void AlarmPIActionsServlet::doDelete(HTTPRequest& request, HTTPResponse& response) {
	Json::Value res;
	if ( !system->isConfigMode() ) {
		res["error"] = "Vous devez activer le mode configuration.";
		response << res;
		response.setCode(httpUnauthorized);
	} else {
		Json::Value value;
		Json::Reader reader;
		std::string idToRemove = request.getURL().substr(std::string("/rest/actions/").size());
		Action* action = system->getAction(idToRemove);
		if ( action == NULL ) {
			res["error"] = "L'action " + idToRemove + " n'existe pas.";
			response << res;
			response.setCode(httpUnauthorized);
		} else {
			system->removeAction(action);
			response.setCode(httpOK);

			delete action;
		}
	}
	response.setContentType("application/json");
}


} /* namespace alarmpi */
