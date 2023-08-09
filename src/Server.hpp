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


class Server{
    public:
        std::map<std::string, std::map<std::string, std::string> > serverMap;
        std::map<std::string, std::map<std::string, std::string> > locationMap;

        void printMap(std::map<std::string, std::map<std::string, std::string> > map);
        std::string getItemFromMap(Server web, std::string chavePrincipal, std::string chaveSecundaria, std::string valor);
    private:
};

#endif
