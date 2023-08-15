# include "./Parser.hpp"
#include "../src/Server.hpp"

std::string getPortFromInputFile(std::string stringLine){
    // Encontra a posição do ":" e do ";" na string
    size_t colonPos = stringLine.find(":");
    size_t semicolonPos = stringLine.find(";");

    // Extrai a parte da string entre ":" e ";"
    std::string portString = stringLine.substr(colonPos + 1, semicolonPos - colonPos - 1);


    return(portString);
}

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

std::string getIpFromInputFile(std::string input){

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

Server Parser::parserFile(std::string inputFilePath) {
    std::ifstream arquivo(inputFilePath.c_str()); // Abrir o arquivo para leitura
    std::map<std::string, std::map<std::string, std::string> > serverMap;
    std::map<std::string, std::map<std::string, std::string> > locationMap;
    std::string ipFromServer;
    std::string locationPath;
    Server serverInstance;

    int errorPageCount = 0;
    int cgiCount = 0;
    bool insideLocationBlock = false;

    if (arquivo.is_open()) {
         // Mapa para armazenar os blocos "server"
        std::string line;
        bool insideServerBlock = false;

        while (std::getline(arquivo, line)) {

            if (line.find("server {")){
                insideServerBlock = true;
            }
            if (insideServerBlock) { //isso pode estar em outra função.

                if (line.find("listen") != std::string::npos){
                    ipFromServer = "Server " + getIpFromInputFile(line);
                    serverMap[ipFromServer]["ip"] =  getIpFromInputFile(line);
                    serverMap[ipFromServer]["port"] = getPortFromInputFile(line);
                }
                else if (line.find("server_name") != std::string::npos)
                    serverMap[ipFromServer]["server_name"] =  getValuesFromArchvie(line);
                else if (line.find("root") != std::string::npos && serverMap[ipFromServer]["root"] == "")
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
                    {
                        insideLocationBlock = false;
                        if (locationMap[ipFromServer]["root "+locationPath] == "")
                            locationMap[ipFromServer]["root "+locationPath] = serverMap[ipFromServer]["root"];
                    }
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
    serverInstance.serverMap = serverMap;
    serverInstance.locationMap = locationMap;
    arquivo.close();
    return serverInstance;
}
