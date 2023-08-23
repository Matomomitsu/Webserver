#include "Request.hpp"

std::string Request::itoa(int num) {
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

bool  Request::checkGetRequest( Server &web, const std::string& message, std::string method)
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
    struct addrinfo hints = {}, *res;
    char ipstr[INET6_ADDRSTRLEN];

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(ipAddress.c_str(), NULL, &hints, &res) != 0)
    {
        std::cerr << "getaddrinfo() error" << std::endl;
        return false;
    }
    struct sockaddr_in *ip = (struct sockaddr_in *)res->ai_addr;
    void *addr = &(ip->sin_addr);
    inet_ntop(res->ai_family, addr, ipstr, sizeof(ipstr));
    ipAddress = ipstr;
    freeaddrinfo(res);
    // Imprimir os resultados
    std::cout << "Método: " << requisiton << std::endl;
    std::cout << "Caminho do recurso: " << resourcePath << std::endl;
    std::cout << "Versão HTTP: " << httpVersion << std::endl;
    std::cout << "IP: " << ipAddress << std::endl;
    std::cout << "Porta: " << port << std::endl;

    web.getPathResource = resourcePath;
    web.hostMessageReturn = ipAddress+":"+port;

    return (true);
}

std::string  createResponseMessageError(std::string body){
    std::string body_size = Request::itoa(body.size() + 1);
    std::string response = "HTTP/1.1 404 Not Found\r\n"
                            "Content-Type: text/html\r\n"
                            "Content-Length: " + body_size + "\r\n"
                            "\r\n"
                            + body + "\n";
    return(response);
}

std::string createErrorMessage(Server &web){
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

void Request::handleClient(Server web, int client_sock, Epoll *epoll, std::list<int> clientSockets)
{
	char buffer[1024];
	int bytesRead = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
	buffer[bytesRead] = 0;
	if (bytesRead > 0)
	{
		std::string message(buffer, bytesRead);
		std::cout << "Received message from client: " << message;

		if (web.checkType(message)) // Parte de validação da mensagem
			printf("message in format\n");
        //else
            //fazer exceção

		std::string pathGetRequestFile = web.getRequestPathFile();
		std::string http_response = Response::responseRequest(web, pathGetRequestFile);


		if (http_response == "Error 404"){
            std::string body = createErrorMessage(web);
            std::string response = createResponseMessageError(body);
             send(client_sock, response.c_str(), response.length(), 0);
        }
        else
            send(client_sock, http_response.c_str(), http_response.length(), 0);
        web.pathSegments.clear();
        web.locationPath.clear();
        web.locationRoot.clear();
	}
	else if (bytesRead == 0)
	{
		std::cout << "Client disconnected" << std::endl;
		epoll_ctl(epoll->epoll_fd, EPOLL_CTL_DEL, client_sock, NULL);
		close(client_sock);
		clientSockets.remove(client_sock);
	}
	else
	{
		std::cout << "Error occurred while receiving data" << std::endl;
	}
}
