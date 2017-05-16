/*
 * HTTPResponse.h
 *
 *  Created on: 11 août 2015
 *      Author: horfee
 */

#ifndef HTTPSERVER_HTTPRESPONSE_H_
#define HTTPSERVER_HTTPRESPONSE_H_
#include <evhttp.h>
#include <string>
#include <iostream>
#include "../json/json.h"
#include <exception>

namespace httpserver {

class FileNotFoundException: public std::exception {};

enum http_response_code {
	httpContinue=100,
	httpSwitchingProtocols=101,
	httpProcessing=102,
	httpOK=200,
	httpCreated=201,
	httpAccepted=202,
	httpNonAuthoritativeInformation=203,
	httpNoContent=204,
	httpResetContent=205,
	httpPartialContent=206,
	httpMultiStatus=207,
	httpContentDifferent=210,
	httpIMUsed=226,
	httpMultipleChoices=300,
	httpMovedPermanently=301,
	httpMovedTemporarily=302,
	httpSeeOther=303,
	httpNotModified=304,
	httpUseProxy=305,
	httpTemporaryRedirect=307,
	httpPermanentRedirect=308,
	httpToomanyRedirects=310,
	httpBadRequest=400,
	httpUnauthorized=401,
	httpPaymentRequired=402,
	httpForbidden=403,
	httpNotFound=404,
	httpMethodNotAllowed=405,
	httpNotAcceptable=406,
	httpProxyAuthenticationRequired=407,
	httpRequestTimeout=408,
	httpConflict=409,
	httpGone=410,
	httpLengthRequired=411,
	httpPreconditionFailed=412,
	httpRequestEntityTooLarge=413,
	httpRequestURITooLong=414,
	httpUnsupportedMediaType=415,
	httpRequestedrangeunsatisfiable=416,
	httpExpectationfailed=417,
	httpBadmappingMisdirectedRequest=421,
	httpUnprocessableentity=422,
	httpLocked=423,
	httpMethodfailure=424,
	httpUnorderedCollection=425,
	httpUpgradeRequired=426,
	httpPreconditionRequired=428,
	httpTooManyRequests=429,
	httpRequestHeaderFieldsTooLarge=431,
	httpRetryWith=449,
	httpBlockedbyWindowsParentalControls=450,
	httpUnavailableForLegalReasons=451,
	httpUnrecoverableError=456,
	httpClienthasclosedconnection=499,
	httpInternalServerError=500,
	httpNotImplemented=501,
	httpBadGatewayouProxyError=502,
	httpServiceUnavailable=503,
	httpGatewayTimeout=504,
	httpHTTPVersionnotsupported=505,
	httpVariantalsonegociate=506,
	httpInsufficientstorage=507,
	httpLoopdetected=508,
	httpBandwidthLimitExceeded=509,
	httpNotextended=510,
	httpNetworkauthenticationrequired=511,
	httpWebserverisreturninganunknownerror=520
};

typedef std::ostream& (*ostream_manipulator)(std::ostream&);

class HTTPResponse {


public:
	friend HTTPResponse& operator<<(HTTPResponse& out, const std::string& str);
	friend HTTPResponse& operator<<(HTTPResponse& out, int& value);
	friend HTTPResponse& operator<<(HTTPResponse& out, long& value);
	friend HTTPResponse& operator<<(HTTPResponse& out, float& value);
	friend HTTPResponse& operator<<(HTTPResponse& out, double& value);
	friend HTTPResponse& operator<<(HTTPResponse& out, bool& value);
	friend HTTPResponse& operator<<(HTTPResponse& out, const char * value);
	friend HTTPResponse& operator<<(HTTPResponse& out, Json::Value& value);

	void appendFile(std::string fileName);
	void appendFile(FILE * f);

//	friend HTTPResponse& operator<<(HTTPResponse& out, int fd);


	typedef HTTPResponse& (*HTTPResponseManipulator)(HTTPResponse&);

	// take in a function with the custom signature
	HTTPResponse& operator<<(HTTPResponseManipulator manip)
	{
		// call the function, and return it's value
		return manip(*this);
	}

	// define the custom endl for this stream.
	// note how it matches the `HTTPResponseManipulator`
	// function signature
	static HTTPResponse& endl(HTTPResponse& stream)
	{
		stream << std::endl;
		return stream;
	}

	// this is the type of std::cout
	typedef std::basic_ostream<char, std::char_traits<char> > CoutType;

	// this is the function signature of std::endl
	typedef CoutType& (*StandardEndLine)(CoutType&);

	// define an operator<< to take in std::endl
	HTTPResponse& operator<<(StandardEndLine manip)
	{
		// call the function, but we cannot return it's value
		manip(std::cout);

		return *this;
	}


	evbuffer* getBuffer();

	HTTPResponse();
	virtual ~HTTPResponse();

	int getCode() const;
	void setCode(enum http_response_code code);

	std::string getReason() const;
	void setReason(std::string reason);


	std::string getContentType() const;
	void setContentType(std::string contentType);

	void addHeader(std::string header, std::string value);
	void removeHeader(std::string header);

	std::map<std::string, std::string> getHeaders()const;

protected:

private:

	std::string reason;
	int code;
	evbuffer* buffer;
	std::string contentType;
	std::map<std::string, std::string> headers;
};

} /* namespace httpserver */

#endif /* HTTPSERVER_HTTPRESPONSE_H_ */
