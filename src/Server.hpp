#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <vector>
# include <fcntl.h>
# include <fstream>
# include <sstream>
# include <cstdio>
# include <vector>
# include <map>
#include <sys/types.h>
#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <dirent.h>

class Server{
    public:
        std::map<std::string, std::map<std::string, std::string> > serverMap;
        std::map<std::string, std::map<std::string, std::string> > locationMap;
        std::string getPathResource;
        std::string hostMessageReturn;
        std::string locationPath;

        void printMap(std::map<std::string, std::map<std::string, std::string> > map);
        std::string getItemFromServerMap(Server web, std::string chavePrincipal, std::string chaveSecundaria);
        std::string getItemFromLocationMap(Server web, std::string chavePrincipal, std::string chaveSecundaria);
        std::string  responseRequest(Server web, std::string RequestPathResource);
        std::string  getResponseFile(std::string responseRequestFilePath, Server web, std::string RequestPathResource);
        std::string  createResponseMessage(std::string body);
        bool  checkGetRequest( const std::string& message, std::string method);
        bool  checkType( const std::string& requestMessage);
        std::string getRequestPathFile(void);
        std::string  findLocationRoot(Server web, std::string RequestPathResource);
        std::vector<std::string> splitPath(const std::string& path, char delimiter);
        std::string checkLocationPath(const std::vector<std::string>& pathSegments, std::map <std::string, std::string>& locationMap);
        bool checkIfIsDirectory(Server web, std::string &path);

    private:
};

#endif
