/*
 * HTTPRequest.cpp
 *
 *  Created on: 11 août 2015
 *      Author: horfee
 */

#include "HTTPRequest.h"
//#include <event-internal.h>
#include <iostream>
#include <sstream>

namespace httpserver {


HTTPRequest::HTTPRequest(struct evhttp_request* req) {
	this->request = req;
	url = NULL;
	parameters = NULL;
	headers = NULL;
	data = NULL;

}

HTTPRequest::~HTTPRequest() {
	if ( url ) delete url;
	if ( parameters ) delete parameters;
	if ( headers ) delete headers;
	if ( data ) delete data;
}

std::string HTTPRequest::getURL() {
	if ( url == NULL ) {
		const char *uri = evhttp_request_get_uri(request);
		struct evhttp_uri *decoded = evhttp_uri_parse(uri);
		const char *path;

		if (!decoded) {
			printf("It's not a good URI. Sending BADREQUEST\n");
			return "";
		}

		/* Let's see what path the user asked for. */
		path = evhttp_uri_get_path(decoded);
		if (!path) path = "/";

		/* We need to decode it, to see what path the user really wanted. */
		char *decoded_path = evhttp_uridecode(path, 0, NULL);
		std::string res;
		if ( decoded_path == NULL ) {
			url = new std::string("");
		} else {
			url = new std::string(decoded_path);
		}
		evhttp_uri_free(decoded);
		free(decoded_path);
	}

	return *url;
}

std::map<enum evhttp_cmd_type, std::string> httpStringMethods = {
		{EVHTTP_REQ_GET,		"GET"     },
		{EVHTTP_REQ_POST,		"POST"    },
		{EVHTTP_REQ_HEAD,		"HEAD"    },
		{EVHTTP_REQ_PUT,		"PUT"     },
		{EVHTTP_REQ_DELETE,		"DELETE"  },
		{EVHTTP_REQ_OPTIONS,	"OPTIONS" },
		{EVHTTP_REQ_TRACE,		"TRACE"   },
		{EVHTTP_REQ_CONNECT,	"CONNECT" },
		{EVHTTP_REQ_PATCH,		"PATCH"   }
};

std::string HTTPRequest::getMethodAsString() const {
	return httpStringMethods[getMethod()];
}

enum evhttp_cmd_type HTTPRequest::getMethod() const {
	return evhttp_request_get_command(request);
}

const std::map<std::string, std::string, ci_less> HTTPRequest::getParameters() {
	if ( parameters == NULL ) {
		parameters = new std::map<std::string, std::string, ci_less>();
		const char *uri = evhttp_request_get_uri(request);
		struct evhttp_uri *decoded = evhttp_uri_parse(uri);
		const char* query = evhttp_uri_get_query(decoded);

		if ( query != NULL ) {
			std::string q(query);
			std::stringstream ss(q);
			std::string tok;

			while(getline(ss, tok, '&')) {
				size_t p = tok.find('=');
				std::string key = tok.substr(0, p);
				std::string val = tok.substr(p + 1);
				parameters->insert(std::pair<std::string, std::string>(key, val));
			}
		}
		evhttp_uri_free(decoded);
	}
	return *parameters;
}

bool HTTPRequest::hasParameter(std::string parameter) {
	auto m = getParameters();
	return m.find(parameter) != m.end();
}

bool HTTPRequest::hasHeader(std::string header) {
	auto m = getHeaders();
	return m.find(header) != m.end();
}

std::string HTTPRequest::getParameter(std::string parameter) {
	if ( hasParameter(parameter) ) {
		return (*parameters)[parameter];
	}
	return "";
}

std::string HTTPRequest::getHeader(std::string header) {
	if ( hasHeader(header) ) {
		return (*headers)[header];
	}
	return "";
}

const std::map<std::string, std::string, ci_less> HTTPRequest::getHeaders() {
	if ( headers == NULL ) {
		headers = new std::map<std::string, std::string, ci_less>();

		struct evkeyvalq *header = evhttp_request_get_input_headers(request);
		struct evkeyval* kv = header->tqh_first;
		while (kv) {
			std::string key(kv->key);
			std::string value(kv->value);
			(*headers)[key] = value;
			kv = kv->next.tqe_next;
		}
	}
	return *headers;
}

std::string HTTPRequest::getData() {
	if ( data == NULL ) {
		struct evbuffer* buffer = evhttp_request_get_input_buffer(request);
		size_t lg = evbuffer_get_length(buffer);
		char * _data = (char*)calloc(lg, sizeof(char));
		evbuffer_copyout(buffer, (void *)_data, lg);
		data = new std::string(_data);
		free(_data);
	}

	return *data;
}

std::ostream & operator<<(std::ostream &os, HTTPRequest& p)
{
	std::string httpVersion("HTTP/1.1");
//	std::string url(p.getURL());
	const char *uri = evhttp_request_get_uri(p.request);
	os << p.getMethodAsString() << " " << uri << " " << httpVersion << std::endl;
	for(auto header : p.getHeaders()) {
		os << header.first << ": " << header.second << std::endl;
	}
	os << std::endl;
	os << p.getData();
    return os;
}

} /* namespace httpserver */
