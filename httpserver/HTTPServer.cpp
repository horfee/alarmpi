/*
 * HTTPServer.cpp
 *
 *  Created on: 10 août 2015
 *      Author: horfee
 */

#include "HTTPServer.h"
#include <iostream>
#include <functional>
#include <thread>
#include <regex>
#include <algorithm>
#include "InvalidConfigFileException.h"

namespace httpserver {

void HTTPServer::init(std::istream& configStream) {
	Json::Value config;
	Json::Value::const_iterator it, it2;
	Json::Reader reader;

	bool parsingSuccessful = reader.parse(configStream, config, false);
	if ( !parsingSuccessful ) {
		throw InvalidConfigFileException(reader.getFormattedErrorMessages());
	}

	serverAddr = config["listening"].asString();
	port = atoi( serverAddr.substr(serverAddr.find(":") + 1).c_str());
	serverAddr = serverAddr.substr(0, serverAddr.find(":"));
	base = NULL;
	http = NULL;

	Json::Value methods = config["methods"];
	allowedMethods = 0;
	for(unsigned int i = 0; i < methods.size(); i++) {
		if ( methods[i] == "POST"    ) 	allowedMethods |= EVHTTP_REQ_POST;
		if ( methods[i] == "PUT"     ) 	allowedMethods |= EVHTTP_REQ_PUT;
		if ( methods[i] == "GET"     ) 	allowedMethods |= EVHTTP_REQ_GET;
		if ( methods[i] == "DELETE"  ) 	allowedMethods |= EVHTTP_REQ_DELETE;
		if ( methods[i] == "HEAD"    ) 	allowedMethods |= EVHTTP_REQ_HEAD;
		if ( methods[i] == "OPTIONS" ) 	allowedMethods |= EVHTTP_REQ_OPTIONS;
		if ( methods[i] == "CONNECT" ) 	allowedMethods |= EVHTTP_REQ_CONNECT;
		if ( methods[i] == "PATCH"   ) 	allowedMethods |= EVHTTP_REQ_PATCH;
	}

//	allowedMethods = EVHTTP_REQ_GET | EVHTTP_REQ_POST ;
	defaultServlet = NULL;

	it = config["errorPages"].begin();
	it2= config["errorPages"].end();
	while(it != it2) {
		if ( !it.key().isNull() && !it->isNull() ) {
			if ( it.key().type() == Json::intValue || it.key().type() == Json::uintValue )
				errorPages[it.key().asInt()] = it->asString();
			else if ( it.key().type() == Json::stringValue ) {
				errorPages[atoi(it.key().asString().c_str())] = it->asString();
			}
		}

		it++;
	}
	it = config["mimeTypes"].begin();
	it2= config["mimeTypes"].end();
	while(it != it2) {
		std::string key(it.key().asString());
		std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		mimeTypes[key] = it->asString();
		it++;
	}

	if ( config["accessLog"].isNull() )
		accessLog.open("access.log", ios::out | ios::app);
	else
		accessLog.open(config["accessLog"].asString(), ios::out | ios::app);
}
HTTPServer::HTTPServer(std::istream& configStream) {
	init(configStream);
}
HTTPServer::HTTPServer(std::string configFile) {
	std::ifstream file(configFile, std::ifstream::in | std::ifstream::binary);
	init(file);
//	file->close();

}

HTTPServer::HTTPServer(string listeningAddr, int listeningPort) {
	Json::Value config;

	Json::Value methods(Json::arrayValue);
	Json::Value errorPages;
	Json::Value mimeTypes;

	methods.append("GET");
	methods.append("POST");
	methods.append("PUT");
	methods.append("DELETE");

	errorPages[HTTP_OK]					= "./errorPages/200.html";
	errorPages[HTTP_NOCONTENT]			= "./errorPages/204.html";
	errorPages[HTTP_MOVEPERM]			= "./errorPages/301.html";
	errorPages[HTTP_MOVETEMP]			= "./errorPages/302.html";
	errorPages[HTTP_NOTMODIFIED]		= "./errorPages/304.html";
	errorPages[HTTP_BADREQUEST]			= "./errorPages/400.html";
	errorPages[HTTP_NOTFOUND]			= "./errorPages/404.html";
	errorPages[HTTP_BADMETHOD]			= "./errorPages/405.html";
	errorPages[HTTP_ENTITYTOOLARGE]		= "./errorPages/413.html";
	errorPages[HTTP_EXPECTATIONFAILED]	= "./errorPages/417.html";
	errorPages[HTTP_INTERNAL]           = "./errorPages/500.html";
	errorPages[HTTP_NOTIMPLEMENTED]     = "./errorPages/501.html";
	errorPages[HTTP_SERVUNAVAIL]		= "./errorPages/503.html";


	mimeTypes["txt"]	= "text/plain";
	mimeTypes["c"] 		= "text/plain";
	mimeTypes["h"] 		= "text/plain";
	mimeTypes["html"]	= "text/html";
	mimeTypes["htm"]	= "text/htm";
	mimeTypes["css"]	= "text/css";
	mimeTypes["js"]		= "text/javascript";
	mimeTypes["gif"]	= "image/gif";
	mimeTypes["jpg"]	= "image/jpeg";
	mimeTypes["jpeg"]	= "image/jpeg";
	mimeTypes["png"]	= "image/png";
	mimeTypes["svg"]	= "image/svg";
	mimeTypes["pdf"]	= "application/pdf";
	mimeTypes["ps"]		= "application/postsript";

	config["listening"] = listeningAddr + std::string(":") + std::to_string(listeningPort);
	config["allowedMethod"] = methods;
	config["errorPages"] = errorPages;
	config["mimeTypes"] = mimeTypes;

	config["accessLog"] = "/dev/null";

	std::string str;
	Json::StyledWriter writer;
	str = writer.write(config);
	std::istringstream stream(str);
	init(stream);
}

HTTPServer::~HTTPServer() {
	stop();
	accessLog.close();
}

void HTTPServer::setDefaultServlet(HTTPServlet *servlet) {
	defaultServlet = servlet;
	defaultServlet->server = this;
}

std::string HTTPServer::getMimeTypeForFile(std::string file) {
	if ( file.size() == 0 ) return "";
	std::string extension(file.substr(file.find_last_of("/") + 1));
	int ind = extension.find_last_of(".");
	if ( ind >= 0 ) extension = extension.substr( ind + 1);
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

	try {
		return this->mimeTypes.at(extension);
	} catch( std::out_of_range& e) {
		return "application/misc";
	}
}

std::string HTTPServer::getErrorPageFor(int errorCode) {
	return errorPages[errorCode];
}
void HTTPServer::setErrorResponse(enum http_response_code errorCode, HTTPResponse& response) {

	try {
		response.setCode(errorCode);
		response.setContentType(this->getMimeTypeForFile(errorPages[errorCode]));
		response.appendFile(errorPages[errorCode]);
	} catch( FileNotFoundException& e) {
	}

}

void HTTPServer::mainCallBack(struct evhttp_request *req, void *ctx) {
	HTTPServer* self = (HTTPServer*)ctx;

	HTTPRequest httpRequest(req);
	HTTPResponse httpResponse;

	std::string url = httpRequest.getURL();
	std::smatch matches;

	self->accessLog << "New request : " << std::endl;
	self->accessLog << httpRequest.getMethodAsString() << " " << httpRequest.getURL() << std::endl;

	for(auto it : httpRequest.getHeaders() ) {
		self->accessLog << it.first << ": " << it.second << std::endl;
	}
	self->accessLog << "----" << std::endl;

	std::map<std::string, HTTPServlet*>::iterator it;
	bool found = false;
	for(it = self->handlers.begin(); it != self->handlers.end() && !found; ++it) {
		std::regex key(it->first, std::regex::grep|std::regex::icase);
		if (std::regex_match(url, matches, key) ) {
			HTTPServlet* servlet = it->second;
			found = true;
			enum evhttp_cmd_type method = evhttp_request_get_command(req);

			self->accessLog << "Handled by servlet matching " << it->first << std::endl;
			try {
				switch(method) {
				case EVHTTP_REQ_GET:
					servlet->doGet(httpRequest, httpResponse);
					break;
				case EVHTTP_REQ_POST:
					servlet->doPost(httpRequest, httpResponse);
					break;
				case EVHTTP_REQ_HEAD:
					servlet->doHead(httpRequest, httpResponse);
					break;
				case EVHTTP_REQ_PUT:
					servlet->doPut(httpRequest, httpResponse);
					break;
				case EVHTTP_REQ_DELETE:
					servlet->doDelete(httpRequest, httpResponse);
					break;
				case EVHTTP_REQ_OPTIONS:
					servlet->doOptions(httpRequest, httpResponse);
					break;
				case EVHTTP_REQ_TRACE:
					servlet->doTrace(httpRequest, httpResponse);
					break;
				case EVHTTP_REQ_CONNECT:
					servlet->doConnect(httpRequest, httpResponse);
					break;
				case EVHTTP_REQ_PATCH:
					servlet->doPatch(httpRequest, httpResponse);
					break;
				}
			} catch (std::exception &e) {
				std::cerr << e.what() << std::endl;
				self->setErrorResponse(httpInternalServerError, httpResponse);
			} catch (...) {
				std::cerr << "An error happened" << std::endl;
				self->setErrorResponse(httpInternalServerError, httpResponse);
			}

			evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", httpResponse.getContentType().c_str());
			for(auto header : httpResponse.getHeaders()) {
				evhttp_add_header(evhttp_request_get_output_headers(req), header.first.c_str(), header.second.c_str());
			}
			evhttp_send_reply(req, httpResponse.getCode(), httpResponse.getReason().c_str(),httpResponse.getBuffer());
		}
	}
	if ( !found ) {
		HTTPServlet *defaultServlet = self->defaultServlet;
		if ( defaultServlet ) {
			enum evhttp_cmd_type method = evhttp_request_get_command(req);
			self->accessLog << "Handled by default servlet " << std::endl;
			try {
				switch(method) {
				case EVHTTP_REQ_GET:
					defaultServlet->doGet(httpRequest, httpResponse);
					break;
				case EVHTTP_REQ_POST:
					defaultServlet->doPost(httpRequest, httpResponse);
					break;
				case EVHTTP_REQ_HEAD:
					defaultServlet->doHead(httpRequest, httpResponse);
					break;
				case EVHTTP_REQ_PUT:
					defaultServlet->doPut(httpRequest, httpResponse);
					break;
				case EVHTTP_REQ_DELETE:
					defaultServlet->doDelete(httpRequest, httpResponse);
					break;
				case EVHTTP_REQ_OPTIONS:
					defaultServlet->doOptions(httpRequest, httpResponse);
					break;
				case EVHTTP_REQ_TRACE:
					defaultServlet->doTrace(httpRequest, httpResponse);
					break;
				case EVHTTP_REQ_CONNECT:
					defaultServlet->doConnect(httpRequest, httpResponse);
					break;
				case EVHTTP_REQ_PATCH:
					defaultServlet->doPatch(httpRequest, httpResponse);
					break;
				}
			} catch(std::exception &e) {
				std::cerr << e.what() << std::endl;
				self->setErrorResponse(httpInternalServerError, httpResponse);
			} catch(void* e2) {
				std::cerr << "An error happened" << std::endl;
				self->setErrorResponse(httpInternalServerError, httpResponse);
			}
			evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", httpResponse.getContentType().c_str());
			for(auto header : httpResponse.getHeaders()) {
				evhttp_add_header(evhttp_request_get_output_headers(req), header.first.c_str(), header.second.c_str());
			}
			evhttp_send_reply(req, httpResponse.getCode(), httpResponse.getReason().c_str(),httpResponse.getBuffer());
		} else {
			evhttp_send_error(req, HTTP_NOTIMPLEMENTED, "No handler found for this url");
		}

	}
}

void HTTPServer::addServlet(std::string url, HTTPServlet *servlet) {
	handlers[url] = servlet;
	servlet->server = this;
}

void HTTPServer::setAllowedMethods(ev_uint16_t methods ) {

	allowedMethods = methods;
	if (http != NULL ) {
		evhttp_set_allowed_methods(http, allowedMethods);
	}

}


void HTTPServer::start() {

	/* Create new event base */
	base = event_base_new();
	if (!base) {
		std::cerr << "Couldn't open event base" << std::endl;
		throw "Unable to start server";
	}
	http = evhttp_new(base);

	if (!http) {
		std::cerr << "couldn't create evhttp. Exiting." << std::endl;
		throw "Unable to create evhttp";
	}
	evhttp_set_allowed_methods(http, allowedMethods);

	evhttp_set_gencb(http, &HTTPServer::mainCallBack , this);
	// static_cast(void(*)(struct evhttp_request *, void *))t, (char *)"/");

	/* Now we tell the evhttp what port to listen on */
	struct evhttp_bound_socket *handle = evhttp_bind_socket_with_handle(http, serverAddr.c_str(), port);
	if (!handle) {
		std::cerr << "couldn't bind to port " << to_string(port) << ". Exiting." << std::endl;
		throw "Unable to bind address " + serverAddr + ":" + to_string(port);
	}

	/* Lets rock */
	std::thread t([&](){
		event_base_dispatch(base);
		std::cout << "End of dispatch" << std::endl;
	});
	t.detach();

}

void HTTPServer::stop() {
	event_base_loopexit(base, NULL);
	evhttp_free(http);
	event_base_free(base);
}

} /* namespace httpserver */
