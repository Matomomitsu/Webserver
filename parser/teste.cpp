#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>

std::string extractPathAfterSlash(const std::string& input) {
    std::string result;
    std::size_t startPos = input.find('/');
    if (startPos != std::string::npos) {
        std::size_t endPos = input.find_first_of(" {", startPos);
        if (endPos != std::string::npos) {
            result = input.substr(startPos, endPos - startPos);
        }
    }
    return result;
}

std::string getIp(std::string input){

    size_t pos_space = input.find(' ');

    std::string ip_with_port = input.substr(pos_space + 1);

    size_t pos_colon = ip_with_port.find(':');
    if (pos_colon == std::string::npos)
        return("Ip Incorreto");

    std::string ip = ip_with_port.substr(0, pos_colon);
    return(ip);
}

std::string getValuesFromArchvie(const std::string& stringComplete){
    std::string value = stringComplete.substr(stringComplete.find(" ") + 1);
    value = value.substr(0, value.length() - 1);
    return value;
}

int main() {
    std::ifstream arquivo("input.txt"); // Abrir o arquivo para leitura
    std::map<std::string, std::map<std::string, std::string>> serverMap;
    std::map<std::string, std::map<std::string, std::string>> locationMap;
    std::string ipFromServer;
    std::string locationPath;
    int errorPageCount = 0;
    int cgiCount = 0;
    bool insideLocationBlock = false;
    
    if (arquivo.is_open()) {
         // Mapa para armazenar os blocos "server"
        std::string line;
        bool insideServerBlock = false;
        ServerBlock currentBlock;

        while (std::getline(arquivo, line)) {

            if (line.find("server {")){
                insideServerBlock = true;
            }
            if (insideServerBlock) { //isso pode estar em outra função.
                
                if (line.find("listen") != std::string::npos){
                    ipFromServer = "Server " + getIp(line);
                    serverMap[ipFromServer]["ip"] =  getValuesFromArchvie(line);
                }
                else if (line.find("server_name") != std::string::npos)
                    serverMap[ipFromServer]["server_name"] =  getValuesFromArchvie(line);
                else if (line.find("root") != std::string::npos)
                    serverMap[ipFromServer]["root"] =  getValuesFromArchvie(line);
                else if (line.find("index") != std::string::npos)
                    serverMap[ipFromServer]["index"] =  getValuesFromArchvie(line);
                else if (line.find("error_page") != std::string::npos){
                    errorPageCount++;
                    std::stringstream ss;
                    ss << errorPageCount;
                    std::string stringValue = ss.str();
                    serverMap[ipFromServer]["error_page "+stringValue] =  getValuesFromArchvie(line);
                }
                else if (line.find("timeout") != std::string::npos)
                    serverMap[ipFromServer]["timeout"] =  getValuesFromArchvie(line);
                else if (line.find("cgi") != std::string::npos){
                    cgiCount++;
                    std::stringstream ss;
                    ss << cgiCount;
                    std::string stringValue = ss.str();
                    serverMap[ipFromServer]["cgi "+stringValue] =  getValuesFromArchvie(line);
                }
                if (line.find("location") != std::string::npos){
                    insideLocationBlock = true;
                    locationPath = extractPathAfterSlash(line);
                }
                if (insideLocationBlock){
                    if (line.find('}') != std::string::npos)
                        insideLocationBlock = false;
                    else if (line.find("location") != std::string::npos){
                        locationMap[ipFromServer]["Path "+locationPath] =  locationPath;
                    }
                    else if (line.find("limit_except") != std::string::npos)
                        locationMap[ipFromServer]["limit_except "+locationPath] =  getValuesFromArchvie(line);
                    else if (line.find("root") != std::string::npos)
                        locationMap[ipFromServer]["root "+locationPath] =  getValuesFromArchvie(line);
                }
            }
        }
    }
     for (const auto& serverPair : serverMap) {
        const std::string& serverName = serverPair.first;
        const std::map<std::string, std::string>& serverInfo = serverPair.second;

        std::cout << "Server Name: " << serverName << std::endl;
        for (const auto& infoPair : serverInfo) {
            const std::string& key = infoPair.first;
            const std::string& value = infoPair.second;
            std::cout << "  " << key << ": " << value << std::endl;
        }
    }
     for (const auto& serverPair : locationMap) {
        const std::string& serverName = serverPair.first;
        const std::map<std::string, std::string>& serverInfo = serverPair.second;

        std::cout << "LOCATION: " << serverName << std::endl;
        for (const auto& infoPair : serverInfo) {
            const std::string& key = infoPair.first;
            const std::string& value = infoPair.second;
            std::cout << "  " << key << ": " << value << std::endl;
        }
    }
    return 0;
}
