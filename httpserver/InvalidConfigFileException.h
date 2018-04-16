/*
 * InvalidConfigFileException.h
 *
 *  Created on: 19 juil. 2016
 *      Author: horfee
 */

#ifndef HTTPSERVER_INVALIDCONFIGFILEEXCEPTION_H_
#define HTTPSERVER_INVALIDCONFIGFILEEXCEPTION_H_

#include <exception>
#include <string>


class InvalidConfigFileException: public std::exception {
public:
	InvalidConfigFileException(std::string reason);
	virtual ~InvalidConfigFileException();

	virtual const char* what() const noexcept;
private:
	std::string reason;
};

#endif /* HTTPSERVER_INVALIDCONFIGFILEEXCEPTION_H_ */
