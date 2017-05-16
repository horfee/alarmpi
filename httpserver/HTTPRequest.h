/*
 * HTTPRequest.h
 *
 *  Created on: 11 août 2015
 *      Author: horfee
 */

#ifndef HTTPSERVER_HTTPREQUEST_H_
#define HTTPSERVER_HTTPREQUEST_H_
#include <evhttp.h>
#include <map>
#include <string>
#include <ostream>

namespace httpserver {

struct ci_less : std::binary_function<std::string, std::string, bool>
 {
   // case-independent (ci) compare_less binary function
   struct nocase_compare : public std::binary_function<unsigned char,unsigned char,bool>
   {
	 bool operator() (const unsigned char& c1, const unsigned char& c2) const {
		 return tolower (c1) < tolower (c2);
	 }
   };
   bool operator() (const std::string & s1, const std::string & s2) const {
	 return std::lexicographical_compare
	   (s1.begin (), s1.end (),   // source range
	   s2.begin (), s2.end (),   // dest range
	   nocase_compare ());  // comparison
   }
 };

class HTTPRequest {
public:
	HTTPRequest(struct evhttp_request* req);
	virtual ~HTTPRequest();


	std::string getURL();

	enum evhttp_cmd_type getMethod() const;

	std::string getMethodAsString() const;

	const std::map<std::string, std::string, ci_less> getParameters();

	bool hasParameter(std::string parameter);

	std::string getParameter(std::string parameter);

	const std::map<std::string, std::string, ci_less> getHeaders();

	std::string getHeader(std::string header);

	bool hasHeader(std::string header);

	std::string getData();

	friend std::ostream & operator<<(std::ostream &os, HTTPRequest& request);
private:

	struct evhttp_request* request;
	std::map<std::string, std::string, ci_less>* parameters;
	std::map<std::string, std::string, ci_less>* headers;
	std::string* url;
	std::string* data;




};

} /* namespace httpserver */

#endif /* HTTPSERVER_HTTPREQUEST_H_ */
