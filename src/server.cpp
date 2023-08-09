# include "./Server.hpp"
# include "../parser/Parser.hpp"
# include "./Webserver.hpp"

std::string Server::getItemFromMap(Server web, std::string chavePrincipal, std::string chaveSecundaria, std::string valor){

    std::map<std::string, std::map<std::string, std::string> >::iterator outerIt;
    for (outerIt = web.serverMap.begin(); outerIt != web.serverMap.end(); ++outerIt) {
        if (outerIt->first == chavePrincipal) {
            std::map<std::string, std::string>& innerMap = outerIt->second;

            std::map<std::string, std::string>::iterator innerIt = innerMap.find(chaveSecundaria);
            std::cout << "aqui    " << innerIt->second << std::endl;
            if (innerIt != innerMap.end() && innerIt->second == valor) {
                std::cout << "Chave externa: " << outerIt->first << ", Porta: " << innerIt->second << std::endl;
                return(innerIt->second);
            }
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
