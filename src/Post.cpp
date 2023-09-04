/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Post.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtomomit <mtomomit@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/25 20:02:57 by mtomomit          #+#    #+#             */
/*   Updated: 2023/09/04 19:02:22 by mtomomit         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Post.hpp"

void Post::getContentTypeData(std::string &header)
{
    std::size_t findContent;
    std::size_t findBoundary;

    findContent = header.find("Content-Type: ");
    if (findContent != std::string::npos)
        contentType = header.substr(findContent);
    else
        throw UnsupportedMediaType();
    contentType = contentType.substr(0, contentType.find("\r\n"));
    findBoundary = contentType.find("boundary=");
    if (findBoundary != std::string::npos){
        mainBoundary = contentType.substr(findBoundary);
        contentType = contentType.substr(14, findBoundary - 16);
        mainBoundary = mainBoundary.substr(9, mainBoundary.length() - 9);
    }
    else{
        mainBoundary = "";
        contentType = contentType.substr(14);
    }
    if (contentType == "multipart/form-data" && mainBoundary == "")
        throw BadRequest();
}

void Post::getLength(std::string header, Server &web)
{
    std::size_t findLength;
    std::string stringLength;
    std::string clientMaxBodySize;

    findLength = header.find("Content-Length: ");
    if (findLength != std::string::npos)
        stringLength = header.substr(findLength);
    else
        throw LengthRequired();
    stringLength = stringLength.substr(0, stringLength.find("\r\n"));
    stringLength = stringLength.substr(16);
    contentLength = atol(stringLength.c_str());
    if (web.locationPath.empty())
        clientMaxBodySize = web.getItemFromServerMap(web, "Server " + web.hostMessageReturn, "client_max_body_size");
    else
        clientMaxBodySize = web.getItemFromLocationMap(web, "Server " + web.hostMessageReturn, "client_max_body_size " + web.locationPath);
    if (clientMaxBodySize != "wrong")
    {
        if (contentLength > static_cast<long unsigned int>(atol(clientMaxBodySize.c_str())))
            throw RequestEntityTooLarge();
    }
}

void Post::getBoundaryHeaderData(std::string &body, std::size_t &bytesReadTotal, std::string &fullRequestPathResource)
{
    std::size_t findContentDisposition;
    std::size_t findFilename;
    std::string header;
    char        buffer[2];
    int         bytesRead;
    size_t      headerEnd;

    headerEnd = body.find("\r\n\r\n");
    while (headerEnd == std::string::npos)
    {
        bytesRead = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        bytesReadTotal += bytesRead;
        buffer[bytesRead] = 0;
        body += buffer;
        headerEnd = body.find("\r\n\r\n");
    }
    findContentDisposition = body.find("Content-Disposition: ");
    if (findContentDisposition == std::string::npos)
        throw BadRequest();
    contentDisposition = body.substr(findContentDisposition);
    contentDisposition = contentDisposition.substr(0, contentDisposition.find("\r\n"));
    findFilename = contentDisposition.find("filename=");
    if (findFilename != std::string::npos)
    {
        filename = contentDisposition.substr(findFilename + 10);
        filename = filename.substr(0, filename.find("\""));
        std::ofstream file((fullRequestPathResource + filename).c_str(), std::ios::out | std::ios::binary);
        file.close();
    }
    else{
        filename = "";
        std::ofstream file((fullRequestPathResource + "file").c_str(), std::ios::out | std::ios::binary);
        file.close();
    }
    body = body.substr(headerEnd + 4);
}

void	Post::copyToFile(const std::string &fullRequestPathResource, std::size_t limiter, std::string &body)
{
    if (filename != ""){
        std::ofstream file((fullRequestPathResource + filename).c_str(), std::ios::out | std::ios::binary | std::ios_base::app);
        if (file.is_open()){
            file.write(body.c_str(), limiter);
            file.close();
        }
    }
    else{
        std::ofstream file((fullRequestPathResource + "file").c_str(), std::ios::out | std::ios::binary | std::ios_base::app);
        if (file.is_open()){
            file.write(body.c_str(), limiter);
            file.close();
        }
    }
}

void	Post::handleBoundary(std::string fullRequestPathResource)
{
    char buffer[1024];
    std::string body = "";
    int bytesRead = 0;
    bytesRead = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
    while (bytesRead == -1)
        bytesRead = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
    size_t bytesReadTotal = bytesRead;
    std::string bodyEnd = "";
    size_t  findBoundary;

    buffer[bytesRead] = 0;
    body = buffer;
    if (body.substr(0, mainBoundary.size() + 2) != ( "--" + mainBoundary) )
        throw BadRequest();
    while (bytesReadTotal != contentLength || body != ""){
        body = bodyEnd + body;
        bodyEnd = "";
        findBoundary = body.find("--" + mainBoundary);
        if (findBoundary != std::string::npos){
            if (findBoundary != 0)
                this->copyToFile(fullRequestPathResource, findBoundary - 2, body);
            body = body.substr(findBoundary + mainBoundary.size() + 2);
            if (body != "--\r\n")
                this->getBoundaryHeaderData(body, bytesReadTotal, fullRequestPathResource);
            else
                body = "";
            bytesRead = body.size();
        }
        else
        {
            if (static_cast<long unsigned int>(bytesRead) > mainBoundary.size())
                bodyEnd = body.substr(bytesRead - mainBoundary.size() - 2);
            else
                bodyEnd = body;
            this->copyToFile(fullRequestPathResource, bytesRead - mainBoundary.size() - 2, body);
            bytesRead = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
            if (bytesRead != -1)
            {
                buffer[bytesRead] = 0;
                bytesReadTotal += bytesRead;
                body = buffer;
            }
            else
            {
                body = "";
                bytesRead = 0;
            }
        }
    }
}

void	Post::getBinaryContentDisposition(std::string &fullRequestPathResource, std::string &header)
{
    std::size_t findFilename;
    std::size_t findContentDispostion;

    findContentDispostion = header.find("Content-Disposition: ");
    if (findContentDispostion != std::string::npos)
    {
        contentDisposition = header.substr(0, header.find("Content-Disposition: "));
        contentDisposition = contentDisposition.substr(0, contentDisposition.find("\r\n"));
        findFilename = contentDisposition.find("filename=");
    }
    else
        findFilename = findContentDispostion;
    if (findFilename != std::string::npos)
    {
        filename = contentDisposition.substr(findFilename + 10);
        filename = filename.substr(0, filename.find("\""));
        std::ofstream file((fullRequestPathResource + filename).c_str(), std::ios::out | std::ios::binary);
        file.close();
    }
    else
    {
        std::ofstream file((fullRequestPathResource + filename).c_str(), std::ios::out | std::ios::binary);
        file.close();
        filename = "";
    }
}

void	Post::handleBinary(const std::string &fullRequestPathResource)
{
    char buffer[2048];
    std::string body = "";
    int bytesRead = 0;
    size_t bytesReadTotal = 0;

    while (bytesReadTotal != contentLength){
        bytesRead = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead != -1)
        {
            bytesReadTotal += bytesRead;
            body.insert(body.end(), buffer, buffer + bytesRead);
            copyToFile(fullRequestPathResource, body.size(), body);
            body.clear();
        }
    }
}

std::string  Post::createResponseMessage(std::string &fullRequestPathResource){
    std::string response = "HTTP/1.1 201 Created\r\n"
                            "Content-Type: text/plain\r\n"
                            "Location: " + fullRequestPathResource + "\r\n"
                            "Content-Length: 0\r\n"
                            "\r\n";
    return(response);
}

std::string Post::postResponse(Server &web, std::string RequestPathResource, std::string header)
{
    std::string fullRequestPathResource;
    std::string response;

    fullRequestPathResource = Response::findLocationRoot(web, RequestPathResource) + "/";
    DIR* directory = opendir(fullRequestPathResource.c_str());
    if (!directory)
        return ("Error 400");
    else
        closedir(directory);
    try{
        this->getContentTypeData(header);
        this->getLength(header, web);
        if (this->contentType == "multipart/form-data")
            this->handleBoundary(fullRequestPathResource);
        else
        {
            this->getBinaryContentDisposition(fullRequestPathResource, header);
            this->handleBinary(fullRequestPathResource);
        }
    }
    catch(std::exception &e){
        return (e.what());
    }

    return (this->createResponseMessage(fullRequestPathResource));
}


const char *Post::BadRequest::what() const throw(){
	return ("Error 400");
}

const char *Post::NotFound::what() const throw(){
	return ("Error 404");
}

const char *Post::LengthRequired::what() const throw(){
	return ("Error 411");
}

const char *Post::RequestEntityTooLarge::what() const throw(){
	return ("Error 413");
}

const char *Post::UnsupportedMediaType::what() const throw(){
	return ("Error 415");
}
