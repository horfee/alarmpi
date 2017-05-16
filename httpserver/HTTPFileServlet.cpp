/*
 * HTTPFileServlet.cpp
 *
 *  Created on: 17 sept. 2015
 *      Author: horfee
 */

#include "HTTPFileServlet.h"
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <iostream>

namespace httpserver {

HTTPFileServlet::HTTPFileServlet(std::string rootFolder): rootFolder(rootFolder) {
}

HTTPFileServlet::~HTTPFileServlet() {
}

void HTTPFileServlet::doGet(HTTPRequest &request, HTTPResponse &response) {

	auto createFolderIndex = [&](std::string path){
		struct dirent *ent;
		DIR* d;
		if (!(d = opendir(path.c_str()))) {
			throw "Invalid Path";
		}

		ostringstream os;
		os << "<!DOCTYPE html>\n"
		<< "<html>\n"
		<< "	<head>\n"
		<<		"		<meta http-equiv='Content-Type' content='text/html; charset=UTF-8' />"
		<<		"		<meta charset='utf-8'>\n"
		<<		"		<title>Index Of " << request.getURL() << "</title>\n"
		<<		"		<base href='" << request.getURL() << "'/>\n"
		<<		"		<style>"
		<<		"			html {"
		<<		"				background-color: lightgrey;"
		<<		"			}"
		<<		"			body {"
		<<		"				 margin:50px;"
		<<		"				 background-color: white;"
		<<		"				 border: 1px thin solid grey;"
		<<		"				 border-radius: 10px;"
		<<		"				 padding: 3px;"
		<<		"			}"
		<<		"			h1 {"
		<<		"				color:grey;"
		<<		"			}"
		<<		"			ul {"
		<<		"				list-style-type: none;"
		<<		"			}"
		<<		"			li {"
		<<		"				vertical-align: middle;"
		<<		"			}"
		<<		"			.file:before {"
		<<		"				background-image: url('/images/file.png');"
		<<		"				background-size: contain;"
		<<		"				display: inline-block;"
		<<		"				width: 32px;"
		<<		"				height: 32px;"
		<<		"				content: '';"
		<<		"			}"
		<<		"			.folder:before {"
		<<		"				background-image: url('/images/folder.png');"
		<<		"				background-size: contain;"
		<<		"				display: inline-block;"
		<<		"				width: 32px;"
		<<		"				height: 32px;"
		<<		"				content: '';"
		<<		"			}"
		<<		"		</style>"
		<<		"	</head>\n"
		<<		"	<body>\n"
		<<		"		<h1>Index Of " << request.getURL() << "</h1>\n"
		<<		"		<br/>\n"
		<<		"		<ul>\n";

		while ((ent = readdir(d))) {
			std::string name(ent->d_name);
			os << "   		<li class=\"" << ( ent->d_type == DT_REG ? "file" : ent->d_type == DT_DIR ? "folder" : "")<< "\"><a href=\"" << request.getURL() << (request.getURL()[request.getURL().size() - 1] != '/'? "/" : "") << name << "\">" << name << "</a>\n";
		}
		os	<< 	"		</ul>\n"
			<<	"	</body>\n"
			<<	"</html>";
		closedir(d);
		return os.str();
	};



	std::string fileName(rootFolder);
	if ( fileName[fileName.length() - 1] != '/' ) fileName += "/";
		// here fileName ends with "/"

	if ( request.getURL().length() == 0 ) {
		response << createFolderIndex(fileName);
		response.setContentType(getServer()->getMimeTypeForFile(".html"));
		response.setCode(httpOK);
	} else if ( request.getURL()[0] == '/') {
		struct stat buffer;

		fileName += request.getURL().substr(1);
		if (stat (fileName.c_str(), &buffer) == 0) {
			if ( S_ISDIR(buffer.st_mode) ) {
				response << createFolderIndex(fileName);
				response.setContentType(getServer()->getMimeTypeForFile(".html"));
				response.setCode(httpOK);
			} else {
				try {
					response.appendFile(fileName);
					response.setCode(httpOK);
					response.setContentType(getServer()->getMimeTypeForFile(fileName));
				} catch( FileNotFoundException& e) {
					getServer()->setErrorResponse(httpNotFound, response);
				}

			}

		} else {
			getServer()->accessLog << "Error opening file unexist.ent: " << strerror(errno) << std::endl;
			getServer()->setErrorResponse(httpNotFound, response);
		}

	}
}

} /* namespace httpserver */
