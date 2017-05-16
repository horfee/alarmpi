/*
 * HTTPServer.h
 *
 *  Created on: 10 août 2015
 *      Author: horfee
 */

#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_
#include <string>
#include <evhttp.h>
#include <map>
#include <vector>
#include <functional>
#include <tuple>
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "HTTPServlet.h"
#include <mutex>
#include <iostream>
#include <fstream>
#include "../json/json.h"

using namespace std;

namespace httpserver {

class HTTPServlet;

class HTTPFileServlet;

class HTTPServer {
	friend class HTTPFileServlet;

public:

	HTTPServer(std::istream& configStream);
	HTTPServer(std::string configFile);
	HTTPServer(string listeningAddr, int listeningPort);
	virtual ~HTTPServer();

	void start();
	void stop();

	void addServlet(std::string url, HTTPServlet *servlet);
	void setDefaultServlet(HTTPServlet *servlet);

	void setAllowedMethods(ev_uint16_t methods );

	std::string getMimeTypeForFile(std::string file);

	std::string getErrorPageFor(int errorCode);

	void setErrorResponse(enum http_response_code errorCode, HTTPResponse& response);
private:

	void init(std::istream& Configstream);
	int port;
	string serverAddr;

	struct event_base *base;
	struct evhttp *http;

	std::map<std::string, HTTPServlet*> handlers;
	HTTPServlet *defaultServlet;

	static void mainCallBack(struct evhttp_request *req, void *arg);

	ev_uint16_t allowedMethods;

	std::map<int, std::string> errorPages;
	std::map<std::string, std::string> mimeTypes;

	std::mutex mimeTypesMutex;

	std::ofstream accessLog;
};

} /* namespace httpserver */

#endif /* HTTPSERVER_H_ */
