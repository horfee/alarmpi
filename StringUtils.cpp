/*
 * StringUtils.cpp
 *
 *  Created on: 6 avr. 2017
 *      Author: horfee
 */

#include "StringUtils.h"
#include <algorithm>
#include <iostream>

std::string trim(const std::string &s)
{
   auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
   auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
   return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}


bool stringContains(std::string text, std::string search) {
	return text.find(search) != std::string::npos;
}


std::vector<std::string> split(std::string text, std::initializer_list<std::string> separators ) {
//	std::istringstream iss(text);
	std::vector<std::string>res;
	while ( text.size() > 0 ) {
		std::string::size_type indx = std::string::npos;
		for(std::string s : separators) {
			indx = std::min(indx,text.find(s));
		}
		indx = std::min(indx, text.size());
		std::string line = text.substr(0, indx);
		if ( indx < text.size() )
			text = text.substr(indx + 1);
		else
			text = text.substr(indx);

		if ( !line.empty() ) {
			res.push_back(line);
		}

	}

	return res;
}
