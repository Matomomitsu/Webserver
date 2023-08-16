# include "./Server.hpp"
# include "../parser/Parser.hpp"
# include "./Webserver.hpp"

std::string itoa(int num) {
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

std::string Server::getRequestPathFile(void){
    return (this->getPathResource);
}

std::string Server::checkLocationPath(const std::vector<std::string>& pathSegments, std::map <std::string, std::string>& location) {
    std::string currentPath;
    std::string matchingLocationPath;

    currentPath += "root /";
    {
        std::map<std::string, std::string>::const_iterator locationPathIterator = location.find(currentPath);
        if (locationPathIterator != location.end()) {
            matchingLocationPath = locationPathIterator->second;
        }
    }
    for (std::vector<std::string>::const_iterator segmentIterator = pathSegments.begin(); segmentIterator != pathSegments.end(); ++segmentIterator) {
        currentPath += *segmentIterator;
        std::map<std::string, std::string>::const_iterator locationPathIterator = location.find(currentPath);
        if (locationPathIterator != location.end()) {
            matchingLocationPath = locationPathIterator->second;
        }
        currentPath += "/";
    }

    return (matchingLocationPath);
}

std::vector<std::string> Server::splitPath(const std::string& path, char delimiter) {
    std::vector<std::string> segments;
    std::stringstream ss(path);
    std::string segment;
    while (std::getline(ss, segment, delimiter)) {
        if (!segment.empty()) {
            segments.push_back(segment);
        }
    }
    return (segments);
}

std::string  Server::findLocationRoot(Server web, std::string RequestPathResource){
    std::map<std::string, std::map<std::string, std::string> >::iterator outerIt;
    std::vector<std::string> pathSegments;

    std::cout << "Valor associado à chave 'root': " << RequestPathResource << std::endl;
    for (outerIt = web.locationMap.begin(); outerIt != web.locationMap.end(); ++outerIt){
        if (outerIt->first == "Server " + web.hostMessageReturn){
            pathSegments = splitPath(RequestPathResource, '/');
            web.locationPath = checkLocationPath(pathSegments, outerIt->second);
            std::cout << web.locationPath << std::endl;
            if (web.locationPath.empty())
            {
                std::map<std::string, std::map<std::string, std::string> >::iterator serverIt;
                for (serverIt = web.serverMap.begin(); serverIt != web.serverMap.end(); ++serverIt)
                {
                    if (serverIt->first == "Server " + web.hostMessageReturn)
                        web.locationPath = serverIt->second["root"];
                }
            }
            RequestPathResource = web.locationPath + RequestPathResource;
        }
    }
    return(RequestPathResource);
}

std::string  Server::responseRequest(Server web, std::string RequestPathResource){
    std::string fullRequestPathResource;

    fullRequestPathResource = findLocationRoot(web,RequestPathResource);
    std::map<std::string, std::string> keyValueMap;
    std::string response;

    response = getResponseFile(fullRequestPathResource, web, RequestPathResource);
    return(response);
}

bool Server::checkIfIsDirectory(Server web, std::string &path){

    std::cout << web.getPathResource << std::endl;

    std::string fileToReturn = getItemFromLocationMap(web, "Server " + web.hostMessageReturn, web.getPathResource + " index");
    std::vector<std::string> files = splitPath(fileToReturn, ' ');
    
    for (size_t i = 0; i < files.size(); ++i){
        std::cout << files[i] << std::endl;
    }

    DIR* directory = opendir(path.c_str());
    if(directory)
    {
        std::cout << "is a directory" << std::endl;
        closedir(directory);
        return(true);
    }
    else
    {
        std::cout << "is NOT a directory" << std::endl;
        closedir(directory);
        return(false);
    }
}

std::string  Server::getResponseFile(std::string responseRequestFilePath, Server web, std::string RequestPathResource){
    checkIfIsDirectory(web, responseRequestFilePath);

    std::ifstream file(responseRequestFilePath.c_str());
    std::string content;
    std::string response;

    
    if (file.is_open()){
        std::string line;
        while(std::getline(file, line)){
            content += line;
        }
        file.close();
    }
    else
    {
        std::map<std::string, std::map<std::string, std::string> >::iterator serverIt;
        for (serverIt = web.serverMap.begin(); serverIt != web.serverMap.end(); ++serverIt)
        {
            if (serverIt->first == "Server " + web.hostMessageReturn)
            {
                if (web.locationPath != serverIt->second["root"])
                {
                    web.locationPath = serverIt->second["root"];
                    responseRequestFilePath = serverIt->second["root"] + RequestPathResource;
                    return (getResponseFile(responseRequestFilePath, web, RequestPathResource));
                }
            }
        }
        return("Error 404");
    }
    response = createResponseMessage(content);
    return (response);
}

std::string  Server::createResponseMessage(std::string body){
    std::string body_size = itoa(body.size() + 1);
    std::string response = "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/html\r\n"
                            "Content-Length: " + body_size + "\r\n"
                            "\r\n"
                            + body + "\n";
    return(response);
}


bool  Server::checkGetRequest( const std::string& message, std::string method)
{
    //Encontra alinha inicial e final
    size_t startLine = message.find(method);
    //size_t endLine = message.find("\r\n", startLine);
    int methodSize = method.length();

    std::string requisiton = message.substr(startLine, methodSize);

    size_t resourcePathStart = message.find("/", startLine + methodSize);

    if (resourcePathStart != startLine + methodSize + 1){
        std::cout << "Wrong messa in request: " << message << std::endl;
        return(false);
    }

    size_t resourcePathEnd = message.find(" ", resourcePathStart);

    std::string resourcePath = message.substr(resourcePathStart, resourcePathEnd - resourcePathStart);

    size_t httpVersionStart = resourcePathEnd + 1;

    // Extrair a versão HTTP
    std::string httpVersion = message.substr(httpVersionStart);

    size_t hostStart = message.find("Host: ");
    if (hostStart == std::string::npos)
    {
        std::cout << "Campo Host não encontrado na mensagem" << std::endl;
        return false;
    }

    size_t hostEnd = message.find("\r\n", hostStart);
    std::string hostLine = message.substr(hostStart, hostEnd - hostStart);

    // Extrair o valor do campo "Host"
    std::string hostValue = hostLine.substr(6); // Remover "Host: "

    // Separar o IP e a porta
    size_t colonPos = hostValue.find(":");
    if (colonPos == std::string::npos)
    {
        std::cout << "Formato de Host inválido: " << hostValue << std::endl;
        return false;
    }

    std::string ipAddress = hostValue.substr(0, colonPos);
    std::string port = hostValue.substr(colonPos + 1);

    // Imprimir os resultados
    std::cout << "Método: " << requisiton << std::endl;
    std::cout << "Caminho do recurso: " << resourcePath << std::endl;
    std::cout << "Versão HTTP: " << httpVersion << std::endl;
    std::cout << "IP: " << ipAddress << std::endl;
    std::cout << "Porta: " << port << std::endl;

    this->getPathResource = resourcePath;
    this->hostMessageReturn = ipAddress;

    return (true);
}

bool  Server::checkType(const std::string& requestMessage)
{
    bool isTypeCorrect;

    if (requestMessage.substr(0, 3) == "GET")
        isTypeCorrect = checkGetRequest(requestMessage, "GET");

    else if (requestMessage.substr(0, 4) == "POST")
        isTypeCorrect = checkGetRequest(requestMessage, "POST");

    else if (requestMessage.substr(0, 6) == "DELETE")
        isTypeCorrect = checkGetRequest(requestMessage, "DELETE");

    return(isTypeCorrect);

}

std::string Server::getItemFromLocationMap(Server web, std::string chavePrincipal, std::string chaveSecundaria){

    std::map<std::string, std::map<std::string, std::string> >::iterator outerIt;
    for (outerIt = web.locationMap.begin(); outerIt != web.locationMap.end(); ++outerIt) {
        if (outerIt->first == chavePrincipal) {
            std::map<std::string, std::string>& innerMap = outerIt->second;

            std::map<std::string, std::string>::iterator innerIt = innerMap.find(chaveSecundaria);
            return(innerIt->second);
        }
    }
    return("wrong");
}


std::string Server::getItemFromServerMap(Server web, std::string chavePrincipal, std::string chaveSecundaria){

    std::map<std::string, std::map<std::string, std::string> >::iterator outerIt;
    for (outerIt = web.serverMap.begin(); outerIt != web.serverMap.end(); ++outerIt) {
        if (outerIt->first == chavePrincipal) {
            std::map<std::string, std::string>& innerMap = outerIt->second;

            std::map<std::string, std::string>::iterator innerIt = innerMap.find(chaveSecundaria);
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
