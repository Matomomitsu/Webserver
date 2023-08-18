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

std::string Server::checkLocationRoot(const std::vector<std::string>& webPathSegments, std::map <std::string, std::string>& location, Server &web) {
    std::string currentPath;
    std::string matchingLocationPath;

    currentPath += "root /";
    {
        std::map<std::string, std::string>::const_iterator locationPathIterator = location.find(currentPath);
        if (locationPathIterator != location.end()) {
            matchingLocationPath = locationPathIterator->second;
            web.locationPath = currentPath.substr(5, currentPath.length() - 5);
        }
    }
    for (std::vector<std::string>::const_iterator segmentIterator = webPathSegments.begin(); segmentIterator != webPathSegments.end(); ++segmentIterator) {
        currentPath += *segmentIterator;
        std::map<std::string, std::string>::const_iterator locationPathIterator = location.find(currentPath);
        if (locationPathIterator != location.end()) {
            matchingLocationPath = locationPathIterator->second;
            web.locationPath = currentPath.substr(5, currentPath.length() - 5);
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

std::string  Server::findLocationRoot(Server &web, std::string RequestPathResource){
    std::map<std::string, std::map<std::string, std::string> >::iterator outerIt;

    for (outerIt = web.locationMap.begin(); outerIt != web.locationMap.end(); ++outerIt){
        if (outerIt->first == "Server " + web.hostMessageReturn){
            web.pathSegments = splitPath(RequestPathResource, '/');
            web.locationRoot = checkLocationRoot(web.pathSegments, outerIt->second, web);
            if (web.locationRoot.empty()){
                std::map<std::string, std::map<std::string, std::string> >::iterator serverIt;
                for (serverIt = web.serverMap.begin(); serverIt != web.serverMap.end(); ++serverIt){
                    if (serverIt->first == "Server " + web.hostMessageReturn)
                        web.locationRoot = serverIt->second["root"];
                }
            }
            RequestPathResource = web.locationRoot + RequestPathResource.substr(web.locationPath.length(), RequestPathResource.length() - web.locationPath.length());
        }
    }
    return(RequestPathResource);
}

std::string  Server::responseRequest(Server &web, std::string RequestPathResource){
    std::string fullRequestPathResource;

    fullRequestPathResource = findLocationRoot(web, RequestPathResource);
    std::map<std::string, std::string> keyValueMap;
    std::string response;

    response = getResponseFile(fullRequestPathResource, web, RequestPathResource);
    return(response);
}

void Server::addIndex(Server &web, std::string &path){
    std::string currentPath;
    std::string matchingLocationPath;
    std::string fileToReturn = "index.html";
    std::string temp;

    if (web.locationRoot != getItemFromServerMap(web, "Server " + web.hostMessageReturn, "root")){
        temp = getItemFromLocationMap(web, "Server " + web.hostMessageReturn, "index " + web.locationPath);
        if (temp != "wrong")
            fileToReturn = temp;
    }
    else{
        fileToReturn = getItemFromServerMap(web, "Server " + web.hostMessageReturn, "index");
        if (fileToReturn == "wrong")
            fileToReturn = "index.html";
    }
    std::vector<std::string> files = splitPath(fileToReturn, ' ');
    DIR* directory = opendir(path.c_str());
    if(directory){
        struct dirent* entry;
        while ((entry = readdir(directory)) != NULL){
            for (std::vector<std::string>::const_iterator filesIterator = files.begin(); filesIterator != files.end(); ++filesIterator) {
                if (*filesIterator == entry->d_name){
                    path = path + "/" + entry->d_name;
                    closedir(directory);
                    return ;
                }
            }
        }
        closedir(directory);
        return ;
    }
    else{
        closedir(directory);
        return ;
    }
}

std::string  Server::getResponseFile(std::string responseRequestFilePath, Server &web, std::string RequestPathResource){
    addIndex(web, responseRequestFilePath);
    std::ifstream file(responseRequestFilePath.c_str());
    std::string content;
    std::string response;
    DIR* directory = opendir(responseRequestFilePath.c_str());

    if (file.is_open() && !directory){
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
                if (web.locationRoot != serverIt->second["root"])
                {
                    web.locationRoot = serverIt->second["root"];
                    responseRequestFilePath = serverIt->second["root"] + RequestPathResource;
                    return (getResponseFile(responseRequestFilePath, web, RequestPathResource));
                }
            }
        }
        web.pathSegments.clear();
        web.locationPath.clear();
        web.locationRoot.clear();
        std::vector<std::string>().swap(web.pathSegments);
        return("Error 404");
    }
    response = createResponseMessage(content);
    web.pathSegments.clear();
    web.locationPath.clear();
    web.locationRoot.clear();
    std::vector<std::string>().swap(web.pathSegments);
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
    struct hostent *he = gethostbyname(ipAddress.c_str());
    struct in_addr **addr_list;
    if (he == NULL)
    {
        std::cerr << "gethostbyname()" << std::endl;
        return (false);
    }
    addr_list = (struct in_addr **)he->h_addr_list;

    ipAddress = inet_ntoa(*addr_list[0]);
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
