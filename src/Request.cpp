#include "Request.hpp"
#include "Post.hpp"

std::string Request::itoa(int num) {
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

std::vector<std::string> getCgiItens(Server &web){
    std::string cgi = web.getItemFromServerMap(web, "Server "+web.hostMessageReturn, "cgi");
    std::vector<std::string> words;
    std::istringstream iss(cgi);
    std::string word;

    while(iss >> word)
        words.push_back(word);
    return(words);
}

void    Request::getCgiPath(Server &web)
{
    char* path = std::getenv("PATH");
    std::vector<std::string> vectorPaths;
    size_t                   i;

    i = 0;
    vectorPaths = Response::splitPath(path, ':');
    while (i < vectorPaths.size()) {
        if (access((vectorPaths[i] + "/" + web.cgiInit).c_str(), F_OK | X_OK) == 0)
        {
            web.cgiInit = vectorPaths[i] + "/" + web.cgiInit;
            break ;
        }
        i++;
    }
    if (i == vectorPaths.size())
        web.cgiInit = "";
}

bool  Request::checkGetRequest( Server &web, const std::string& message, std::string method)
{
    web.method = method;
    size_t startLine = message.find(method);
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

    std::string httpVersion = message.substr(httpVersionStart);

    size_t hostStart = message.find("Host: ");
    if (hostStart == std::string::npos)
    {
        std::cout << "Campo Host não encontrado na mensagem" << std::endl;
        return false;
    }

    size_t hostEnd = message.find("\r\n", hostStart);
    std::string hostLine = message.substr(hostStart, hostEnd - hostStart);

    std::string hostValue = hostLine.substr(6);

    size_t colonPos = hostValue.find(":");
    if (colonPos == std::string::npos)
    {
        std::cout << "Formato de Host inválido: " << hostValue << std::endl;
        return false;
    }
    web.host = hostValue;
    std::string ipAddress = hostValue.substr(0, colonPos);
    std::string port = hostValue.substr(colonPos + 1);
    struct addrinfo hints = {}, *res;
    char ipstr[INET6_ADDRSTRLEN];

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(ipAddress.c_str(), NULL, &hints, &res) != 0)
    {
        std::cerr << "getaddrinfo() error" << std::endl;
        return false;
    }

    std::string connectionType;
    std::size_t findConnectionType;

    findConnectionType = message.find("Connection: ");
    if (findConnectionType != std::string::npos){
        connectionType = message.substr(message.find("Connection: "));
        connectionType = connectionType.substr(0, connectionType.find("\r\n\r\n"));
        web.connection = connectionType.substr(12);
    }
    else
        web.connection = "text/plain";

    struct sockaddr_in *ip = (struct sockaddr_in *)res->ai_addr;
    void *addr = &(ip->sin_addr);
    inet_ntop(res->ai_family, addr, ipstr, sizeof(ipstr));
    if (ipstr != ipAddress){
        std::string serverNames;
        std::string strigIpstr;

        strigIpstr = ipstr;
        serverNames = web.getItemFromServerMap(web, static_cast<std::string>("Server " + strigIpstr+":"+port), "server_name");
        serverNames = " " + serverNames + " ";
        if (serverNames == "wrong" || serverNames.find(" " + ipAddress + " ") == std::string::npos){
            std::cerr << "URL can not communicate with this server" << std::endl;
            freeaddrinfo(res);
            return false;
        }
        ipAddress = ipstr;
    }
    freeaddrinfo(res);

    std::cout << "Método: " << requisiton << std::endl;
    std::cout << "Caminho do recurso: " << resourcePath << std::endl;
    std::cout << "Versão HTTP: " << httpVersion << std::endl;
    std::cout << "IP: " << ipAddress << std::endl;
    std::cout << "Porta: " << port << std::endl;

    size_t  delimiterPath;

    delimiterPath = resourcePath.find("?");
    if (delimiterPath != std::string::npos){
        web.getPathResource = resourcePath.substr(0, delimiterPath);
        web.queryString = "QUERY_STRING=" + resourcePath.substr(delimiterPath + 1);
        resourcePath = web.getPathResource;
    }
    else{
        web.getPathResource = resourcePath;
        web.queryString = "QUERY_STRING=";
    }
    web.hostMessageReturn = ipAddress+":"+port;
    std::vector<std::string> cgiMap = getCgiItens(web);
    for(size_t i = 0; i < cgiMap.size(); ++i){
        size_t pos = resourcePath.rfind(cgiMap[i]);
        if (pos != std::string::npos){
            web.containsCgi = true;
            web.cgiInit = cgiMap[i + 1];
            Request::getCgiPath(web);
        }
    }
    Response::findLocationRoot(web, resourcePath);

    return (true);
}

std::string createResponseMessageError(std::string body){
    std::string body_size = Request::itoa(body.size() + 1);
    std::string response = "HTTP/1.1 404 Not Found\r\n"
                            "Content-Type: text/html\r\n"
                            "Content-Length: " + body_size + "\r\n"
                            "\r\n"
                            + body + "\n";
    return(response);
}

std::string Request::createErrorMessage(Server &web){
    std::string file = web.locationRoot + "/" + web.getItemFromServerMap(web, "Server " + web.hostMessageReturn, "error_page 404");
    std::ifstream filetoOpen(file.c_str());
    std::string content;

    if (filetoOpen.is_open()){
        std::string line;
        while(std::getline(filetoOpen, line)){
            content += line;
        }
        filetoOpen.close();
    }
    else
    {
        std::string fileDefaultPath = "./utils/error_page/404.html";
        std::ifstream filetoOpenDefault(fileDefaultPath.c_str());
        if (filetoOpenDefault.is_open()){
            std::string line;
            while(std::getline(filetoOpenDefault, line)){
                content += line;
            }
            filetoOpenDefault.close();
        }
    }
    return(content);
}

void Request::handleClient(Server web, int clientSock, Epoll *epoll, std::list<int> clientSockets, std::map<int, Send> &responses)
{
    Response responsed;
    std::string http_response;
    std::string pathGetRequestFile;
    std::string response;
    std::string header = "";
    std::string limitExcept;
    std::string checkResponse;
	char buffer[2];
	int bytesRead = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
	buffer[bytesRead] = 0;


	if (bytesRead > 0)
	{
        web.cgi.clientSock = clientSock;
        header += buffer;
        while (header.find("\r\n\r\n") == std::string::npos && bytesRead > 0)
        {
            bytesRead = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
            if (bytesRead > 0){
                buffer[bytesRead] = 0;
                header += buffer;
            }
        }
        if (bytesRead <= 0)
        {
            http_response = Response::errorType("Error 500", web);
            responses[clientSock].response = http_response;
            responses[clientSock].connection = "close";
            return ;
        }

		if (web.checkType(header))
			printf("message in format\n");
        else{
            http_response = Response::errorType("Error 400", web);
            responses[clientSock].response = http_response;
            responses[clientSock].connection = "close";
            return ;
        }

        http_response = web.checkRedirection(web);
        if (http_response != ""){
            responses[clientSock].response = http_response;
            responses[clientSock].connection = web.connection;
            return ;
        }

        web.checkAutoIndexActive(web);
        if (web.locationPath.empty())
            limitExcept = web.getItemFromServerMap(web, "Server " + web.hostMessageReturn, "limit_except");
        else
            limitExcept = web.getItemFromLocationMap(web, "Server " + web.hostMessageReturn, "limit_except " + web.locationPath);
        if (limitExcept == "wrong")
            limitExcept = "POST DELETE GET";
        if (header.substr(0, 6) == "DELETE" && (limitExcept.find("DELETE") != std::string::npos)){
            http_response = responsed.deleteResponse(web, web.getPathResource);
            checkResponse = Response::errorType(http_response, web);
            if(checkResponse != "OK")
                http_response = Response::errorType(http_response, web);
        }
        else if (header.substr(0, 3) == "GET" && (limitExcept.find("GET") != std::string::npos))
        {
		    pathGetRequestFile = web.getRequestPathFile();
		    http_response = Response::responseRequest(web, pathGetRequestFile, header);
            checkResponse = Response::errorType(http_response, web);
            if(checkResponse != "OK")
                http_response = Response::errorType(http_response, web);
        }
        else if (header.substr(0, 4) == "POST" && (limitExcept.find("POST") != std::string::npos))
        {
            Post    post;
            post.clientSock = clientSock;
            pathGetRequestFile = web.getRequestPathFile();
            http_response = post.postResponse(web, pathGetRequestFile, header);
            checkResponse = Response::errorType(http_response, web);
            if(checkResponse != "OK")
                http_response = Response::errorType(http_response, web);
        }
        else{
            http_response = Response::errorType("Error 405", web);
        }
        responses[clientSock].response = http_response;
        responses[clientSock].connection = web.connection;
	}
	else if (bytesRead == 0)
	{
		std::cout << "Client disconnected" << std::endl;
		epoll_ctl(epoll->epoll_fd, EPOLL_CTL_DEL, clientSock, NULL);
		close(clientSock);
		clientSockets.remove(clientSock);
	}
	else
	{
		std::cout << "Error occurred while receiving data" << std::endl;
        epoll_ctl(epoll->epoll_fd, EPOLL_CTL_DEL, clientSock, NULL);
		close(clientSock);
		clientSockets.remove(clientSock);
	}
}
