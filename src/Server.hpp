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

class Server{
    public:
        std::map<std::string, std::map<std::string, std::string> > serverMap;
        std::map<std::string, std::map<std::string, std::string> > locationMap;
        std::string getPathResource;

        void printMap(std::map<std::string, std::map<std::string, std::string> > map);
        std::string getItemFromMap(Server web, std::string chavePrincipal, std::string chaveSecundaria, std::string valor);
        std::string  responseRequest(std::string RequestPathResource);
        std::string  getResponseFile(std::string responseRequestFilePath);
        std::string  createResponseMessage(std::string body);
        bool  checkGetRequest( const std::string& message, std::string method);
        bool  checkType( const std::string& requestMessage);
        std::string getRequestPathFile(void);

    private:
};

#endif
