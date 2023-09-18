# include "./Parser.hpp"
#include "../src/Server.hpp"

std::string trim(const std::string& str) {
    std::string::const_iterator it2 = std::find_if( str.begin() , str.end() , std::not1(std::ptr_fun<int, int>(std::isspace)));
    std::string::const_iterator it3 = std::find_if( str.rbegin() , str.rend() , std::not1(std::ptr_fun<int, int>(std::isspace))).base();
    return str.substr(it2 - str.begin() , it3 - it2);
}

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

std::string getErrorNumber(const std::string& stringComplete){
    std::size_t pos = stringComplete.find("error_page ");
    std::string value = "";
     if (pos != std::string::npos) {
        pos += sizeof("error_page");
        value = stringComplete.substr(pos);
     }

    std::size_t pos2 = value.find(" ") ;
    if (pos2 != std::string::npos) {
        value = value.substr(0, pos2);
     }
    return value;
}

std::string getErrorPageValues(const std::string& stringComplete, std::string error){
    std::size_t pos = stringComplete.find(error);
    std::string value = "";
     if (pos != std::string::npos) {
        pos += error.length() + 1;
        value = stringComplete.substr(pos);
     }
    std::size_t pos2 = value.find(" ") ;
    if (pos2 != std::string::npos) {
        pos2 += 1;
        value = value.substr(pos2);
        value = value.substr(0, value.length() - 1);
     }
    return value;
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
    std::string ip;
    std::string port;
    Server serverInstance;

    int errorPageCount = 0;
    bool insideLocationBlock = false;

    if (arquivo.is_open()) {
         // Mapa para armazenar os blocos "server"
        std::string line;
        bool insideServerBlock = false;

        while (std::getline(arquivo, line)) {
            line = trim(line);
            if (line.substr(0, 8) == "server {"){
                insideServerBlock = true;
            }
            if (insideServerBlock) { //isso pode estar em outra função.

                if (line.substr(0, 7) == "listen "){
                    ip = getIpFromInputFile(line);
                    port = getPortFromInputFile(line);
                    ipFromServer = "Server " + ip+":"+port;
                    if (serverMap[ipFromServer]["ip"]  != "")
                        throw InvalidFile();
                    serverMap[ipFromServer]["ip"] =  ip;
                    serverMap[ipFromServer]["port"] = port;
                }
                else if (line.substr(0, 10) == "autoindex ")
                    serverMap[ipFromServer]["autoindex"] =  getValuesFromArchvie(line);
                else if (line.substr(0, 12) == "server_name ")
                    serverMap[ipFromServer]["server_name"] =  getValuesFromArchvie(line);
                else if (line.substr(0, 5) == "root " && !insideLocationBlock)
                    serverMap[ipFromServer]["root"] =  getValuesFromArchvie(line);
                else if (line.substr(0, 6) == "index " && !insideLocationBlock)
                    serverMap[ipFromServer]["index"] =  getValuesFromArchvie(line);
                else if (line.substr(0, 21) == "client_max_body_size " && !insideLocationBlock)
                    serverMap[ipFromServer]["client_max_body_size"] =  getValuesFromArchvie(line);
                else if (line.substr(0, 13) == "limit_except " && !insideLocationBlock)
                    serverMap[ipFromServer]["limit_except"] =  getValuesFromArchvie(line);
                else if (line.substr(0, 11) == "error_page "){
                    errorPageCount++;
                    std::stringstream ss;
                    ss << errorPageCount;
                    std::string stringValue = ss.str();
                    serverMap[ipFromServer]["error_page "+getErrorNumber(line)] =  getErrorPageValues(line, "error_page");
                }
                else if (line.substr(0, 4) == "cgi "){
                    serverMap[ipFromServer]["cgi"] +=  " "+getValuesFromArchvie(line);;
                }
                if (line.substr(0, 9) == "location "){
                    insideLocationBlock = true;
                    locationPath = extractPathAfterSlash(line);
                }
                if (insideLocationBlock){
                    if (line == "}")
                    {
                        insideLocationBlock = false;
                        if (locationMap[ipFromServer]["root "+locationPath] == "")
                            locationMap[ipFromServer]["root "+locationPath] = serverMap[ipFromServer]["root"];
                        //locationMap[ipFromServer]["index "+locationPath] += " " + serverMap[ipFromServer]["index"];
                    }
                    else if (line.substr(0, 9) == "location "){
                        locationMap[ipFromServer]["Path "+locationPath] =  locationPath;
                    }
                    else if (line.substr(0, 13) == "limit_except ")
                        locationMap[ipFromServer]["limit_except "+locationPath] =  getValuesFromArchvie(line);
                    else if (line.substr(0, 21) == "client_max_body_size ")
                        locationMap[ipFromServer]["client_max_body_size "+locationPath] =  getValuesFromArchvie(line);
                    else if (line.substr(0, 5) == "root ")
                        locationMap[ipFromServer]["root "+locationPath] =  getValuesFromArchvie(line);
                    else if (line.substr(0, 6) == "index ")
                        locationMap[ipFromServer]["index "+locationPath] =  getValuesFromArchvie(line);
                }
            }
        }
    }
    serverInstance.serverMap = serverMap;
    serverInstance.locationMap = locationMap;
    arquivo.close();
    return serverInstance;
}

const char *Parser::InvalidFile::what() const throw(){
	return ("Configuration file is invalid");
}
