oid Request::handleClient(Server web, int client_sock, Epoll *epoll, std::list<int> clientSockets)
{
    Response responsed;
    std::string http_response;
    std::string pathGetRequestFile;
    std::string response;
    std::string header = "";
    std::string limitExcept;
    bool deleteRequest = false;
	char buffer[2];
	int bytesRead = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
	buffer[bytesRead] = 0;


	if (bytesRead > 0)
	{
        header += buffer;
        while (header.find("\r\n\r\n") == std::string::npos && bytesRead != -1)
        {
            bytesRead = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
            buffer[bytesRead] = 0;
            header += buffer;
        }
		std::cout << "Received message from client: " << header;

		if (web.checkType(header)) // Parte de validação da mensagem
			printf("message in format\n");
        //else
            //fazer exceção
        if (web.locationPath.empty())
            limitExcept = web.getItemFromServerMap(web, "Server " + web.hostMessageReturn, "limit_except");
        else
            limitExcept = web.getItemFromLocationMap(web, "Server " + web.hostMessageReturn, "limit_except " + web.locationPath);
        if (limitExcept == "wrong")
            limitExcept = "POST DELETE GET";

        if (header.substr(0, 6) == "DELETE" && (limitExcept.find("DELETE") != std::string::npos)){
            http_response = responsed.deleteResponse(web, web.getPathResource);
            deleteRequest = true;
            if (http_response == "Error 404"){;
                response = Response::createResponseMessageWithError("Error 404", "Not Found");
                deleteRequest = false;
            }
            else if (http_response == "Error 403"){;
                response = Response::createResponseMessageWithError("Error 403", "Forbidden");
                 std::string body = Request::createErrorMessage(web);
                response = createResponseMessageError(body);
                deleteRequest = false;
            }
            else
                http_response = Response::createResponseMessage(http_response);
        }

        else if (header.substr(0, 3) == "GET" && (limitExcept.find("GET") != std::string::npos))
        {
		    pathGetRequestFile = web.getRequestPathFile();
		    http_response = Response::responseRequest(web, pathGetRequestFile);
        }
        else if (header.substr(0, 4) == "POST" && (limitExcept.find("POST") != std::string::npos))
        {
            Post    post;
            post.clientSock = client_sock;
            pathGetRequestFile = web.getRequestPathFile();
            http_response = post.postResponse(web, pathGetRequestFile, header);
            if (http_response == "Error 404")
                response = Response::createResponseMessageWithError("Error 404", "Not Found");
        }
        if (http_response.empty())
            http_response = "Error 404"; // Trocar para 405
		if (http_response == "Error 404" && !deleteRequest){
            std::string body = Request::createErrorMessage(web);
            response = createResponseMessageError(body);
             send(client_sock, response.c_str(), response.length(), 0);
        }
        else
            send(client_sock, http_response.c_str(), http_response.length(), 0);
        web.pathSegments.clear();
        web.locationPath.clear();
        web.locationRoot.clear();
        char maxbuffer[5];
        while (bytesRead != -1)
            bytesRead = recv(client_sock, maxbuffer, sizeof(maxbuffer) - 1, 0);
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
