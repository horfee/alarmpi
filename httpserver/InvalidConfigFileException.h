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

#ifdef RPI
#ifndef _GLIBCXX_USE_NOEXCEPT
	#define _GLIBCXX_USE_NOEXCEPT	noexcept
#endif
	#define EXCEPTION	_GLIBCXX_USE_NOEXCEPT
#else
	#define EXCEPTION	_NOEXCEPT

#endif
class InvalidConfigFileException: public std::exception {
public:
	InvalidConfigFileException(std::string reason);
	virtual ~InvalidConfigFileException();

	virtual const char* what() const EXCEPTION;
private:
	std::string reason;
};

#endif /* HTTPSERVER_INVALIDCONFIGFILEEXCEPTION_H_ */
