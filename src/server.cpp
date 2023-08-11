# include "./Server.hpp"
# include "../parser/Parser.hpp"
# include "./Webserver.hpp"

std::string Server::getRequestPathFile(void){
    return (this->getPathResource);
}

std::string  Server::responseRequest(std::string RequestPathResource){
    std::map<std::string, std::string> keyValueMap;
    keyValueMap["path1"] = "/teste/abc";
    keyValueMap["path2"] = "/teste/abc2";
    keyValueMap["path3"] = "/teste/abc";
    keyValueMap["path4"] = "/teste/abc2";
    keyValueMap["path5"] = "/utils/index.html";
    std::string response;

    for (std::map<std::string, std::string>::iterator it = keyValueMap.begin(); it != keyValueMap.end(); ++it){
        if (it->second == RequestPathResource){
            char ponto = '.';
            RequestPathResource.insert(RequestPathResource.begin(), ponto);
            response = getResponseFile(RequestPathResource);
            return(response);
        }
    }
    return("Error 404");

}

std::string  Server::getResponseFile(std::string responseRequestFilePath){
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
        return("Error 404");
    response = createResponseMessage(content);
    return (response);
}

std::string  Server::createResponseMessage(std::string body){
    std::string response = "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/html\r\n"
                            "\r\n"
                            + body + "\r\n";
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
    return (true);
}

bool  Server::checkType( const std::string& requestMessage)
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
