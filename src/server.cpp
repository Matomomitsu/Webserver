# include "./Server.hpp"
# include "../parser/Parser.hpp"
# include "./Webserver.hpp"
# include "./Request.hpp"

void Server::checkAutoIndexActive(Server &web){
    std::string checkAutoindex = web.getItemFromServerMap(web, "Server "+web.hostMessageReturn, "autoindex");
    if(checkAutoindex == "on"){
        web.autoindex = true;
    }
    else
        web.autoindex = false;
}


std::string Server::getRequestPathFile(void){
    return (this->getPathResource);
}

bool  Server::checkType(const std::string& requestMessage)
{
    bool isTypeCorrect;

    if (requestMessage.substr(0, 3) == "GET")
        isTypeCorrect = Request::checkGetRequest(*this ,requestMessage, "GET");

    else if (requestMessage.substr(0, 4) == "POST")
        isTypeCorrect = Request::checkGetRequest(*this ,requestMessage, "POST");

    else if (requestMessage.substr(0, 6) == "DELETE"){
        isTypeCorrect = Request::checkGetRequest(*this ,requestMessage, "DELETE");
    }

    return(isTypeCorrect);

}

std::string Server::getItemFromLocationMap(Server &web, std::string chavePrincipal, std::string chaveSecundaria){

    std::map<std::string, std::map<std::string, std::string> >::iterator outerIt;
    for (outerIt = web.locationMap.begin(); outerIt != web.locationMap.end(); ++outerIt) {
        if (outerIt->first == chavePrincipal) {
            std::map<std::string, std::string>& innerMap = outerIt->second;

            std::map<std::string, std::string>::iterator innerIt = innerMap.find(chaveSecundaria);
            if (innerIt != innerMap.end())
                return(innerIt->second);
        }
    }
    return("wrong");
}


std::string Server::getItemFromServerMap(Server &web, std::string chavePrincipal, std::string chaveSecundaria){

    std::map<std::string, std::map<std::string, std::string> >::iterator outerIt;
    for (outerIt = web.serverMap.begin(); outerIt != web.serverMap.end(); ++outerIt) {
        if (outerIt->first == chavePrincipal) {
            std::map<std::string, std::string>& innerMap = outerIt->second;

            std::map<std::string, std::string>::iterator innerIt = innerMap.find(chaveSecundaria);
            if (innerIt != innerMap.end())
                return(innerIt->second);
        }
    }
    return("wrong");
}

void Server::printMap(std::map<std::string, std::map<std::string, std::string> > map){
    std::map<std::string, std::map<std::string, std::string> >::iterator outerIt;
    for (outerIt = map.begin(); outerIt != map.end(); ++outerIt) {
        std::cout << "Server: " << outerIt->first << std::endl;

        std::map<std::string, std::string>& innerMap = outerIt->second;
        std::map<std::string, std::string>::iterator innerIt;
        for (innerIt = innerMap.begin(); innerIt != innerMap.end(); ++innerIt) {
            std::cout << "  " << innerIt->first << ": " << innerIt->second << std::endl;
        }
    }
}
