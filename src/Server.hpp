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
# include <sys/types.h>
# include <stdio.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <string>
# include <dirent.h>
# include "CGI.hpp"
# include "Send.hpp"


class CGI;
class Send;


class Server{
    public:
        std::map<std::string, std::map<std::string, std::string> > serverMap;
        std::map<std::string, std::map<std::string, std::string> > locationMap;
        std::string getPathResource;
        std::string hostMessageReturn;
        std::string locationRoot;
        std::string locationPath;
        std::string contentType;
        std::string connection;
        std::string method;
        std::vector<std::string> pathSegments;
        bool containsCgi;
        bool autoindex;
        CGI         cgi;
        std::string cgiInit;
        std::string queryString;

        void printMap(std::map<std::string, std::map<std::string, std::string> > map);
        std::string getItemFromServerMap(Server &web, std::string chavePrincipal, std::string chaveSecundaria);
        std::string getItemFromLocationMap(Server &web, std::string chavePrincipal, std::string chaveSecundaria);
        bool  checkType( const std::string& requestMessage);
        std::string getRequestPathFile(void);
        void checkAutoIndexActive(Server &web);

    private:
};

#endif
