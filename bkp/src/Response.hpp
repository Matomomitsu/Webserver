#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <iostream>
# include <vector>
# include <fcntl.h>
# include <fstream>
# include <sstream>
# include <cstring>
# include <cstdio>
# include <vector>
# include <map>
# include <sys/types.h>
# include <stdio.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <string>
# include <dirent.h>
# include <cstdlib>
# include <algorithm>
# include <sys/types.h>
# include <sys/wait.h>
# include "Server.hpp"
# include "Request.hpp"

class Response {
	public:
		static std::string checkLocationRoot(const std::vector<std::string>& webPathSegments, std::map <std::string, std::string>& location, Server &web);
		static std::vector<std::string> splitPath(const std::string& path, char delimiter);
		static std::string  findLocationRoot(Server &web, std::string RequestPathResource);
		static std::string  getResponseFile(std::string responseRequestFilePath, Server &web, std::string RequestPathResource);
		static std::string  responseRequest(Server &web, std::string RequestPathResource);
		static void addIndex(Server &web, std::string &path);
		static std::string  createResponseMessage(Server &web, std::string body);
		std::string deleteResponse(Server &web, std::string pathToDelete);
		static std::string createResponseMessageWithError(std::string body, std::string erro, std::string messageErro);
		static std::string errorType(std::string erro);
	private:

};

#endif
