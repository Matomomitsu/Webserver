/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Post.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtomomit <mtomomit@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/25 20:02:57 by mtomomit          #+#    #+#             */
/*   Updated: 2023/09/25 14:03:52 by mtomomit         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Post.hpp"

static size_t hexToNumber(const std::string& hexVector) {
    unsigned int result;
    std::stringstream ss;

    for (std::string::const_iterator it = hexVector.begin(); it != hexVector.end(); ++it) {
        ss << *it;
    }
    ss << std::hex;
    ss >> result;
    return static_cast<size_t>(result);
}

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
    else{
        contentLength = 0;
        return ;
    }
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

void Post::getTransferEncoding(std::string header)
{
    std::size_t findTransferEncoding;
    std::string stringTransferEncoding;
    std::string clientMaxBodySize;

    findTransferEncoding = header.find("Transfer-Encoding: ");
    if (findTransferEncoding != std::string::npos)
        stringTransferEncoding = header.substr(findTransferEncoding);
    else if (contentLength == 0)
        throw LengthRequired();
    else
        return ;
    stringTransferEncoding = stringTransferEncoding.substr(0, stringTransferEncoding.find("\r\n"));
    transferEncoding = stringTransferEncoding.substr(19);
    if (transferEncoding.find("chunked") == std::string::npos && contentLength == 0)
        throw LengthRequired();
}

void Post::getBoundaryHeaderData(std::string &body, std::size_t &bytesReadTotal, std::string &fullRequestPathResource)
{
    std::size_t findContentDisposition;
    std::size_t findFilename;
    std::string header;
    char        buffer[2];
    int         bytesRead = 1;
    std::string::iterator headerEnd;
    std::string doubleCRLF="\r\n\r\n";

    headerEnd = std::search(body.begin(), body.end(), doubleCRLF.begin(), doubleCRLF.end());
    while (headerEnd == body.end() && bytesRead > 0)
    {
        bytesRead = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead > 0){
            bytesReadTotal += bytesRead;
            body.insert(body.end(), buffer, buffer + bytesRead);
            headerEnd = std::search(body.begin(), body.end(), doubleCRLF.begin(), doubleCRLF.end());
        }
    }
    if (headerEnd == body.end() || bytesRead <= 0)
        throw InternalServerError();
    std::string contentDispositionMarker = "Content-Disposition: ";
    findContentDisposition = std::search(body.begin(), body.end(), contentDispositionMarker.begin(), contentDispositionMarker.end()) - body.begin();
    if (findContentDisposition == body.size())
        throw BadRequest();
    contentDisposition = std::string(body.begin() + findContentDisposition, body.end());
    contentDisposition = contentDisposition.substr(0, contentDisposition.find("\r\n"));
    std::cout << contentDisposition << std::endl;
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
    body = std::string(headerEnd + 4, body.end());
}

void Post::copyToFile(const std::string &fullRequestPathResource, std::size_t limiter, std::string &body)
{
    if (filename != ""){
        std::ofstream file((fullRequestPathResource + filename).c_str(), std::ios::out | std::ios::binary | std::ios_base::app);
        if (file.is_open()){
            file.write(body.data(), limiter);
            file.close();
        }
    }
    else{
        std::ofstream file((fullRequestPathResource + "file").c_str(), std::ios::out | std::ios::binary | std::ios_base::app);
        if (file.is_open()){
            file.write(body.data(), limiter);
            file.close();
        }
    }
}

void    Post::getFileData(std::string::iterator &findBoundary, std::string &body, std::vector<char> &buffer, size_t &bytesReadTotal, int &bytesRead)
{
    std::string mainBoundaryVec(mainBoundary.begin(), mainBoundary.end());
    size_t  bytesReadFile;

    bytesReadFile = bytesRead;
    while (findBoundary == body.end() && bytesRead > 0)
    {
        bytesRead = recv(clientSock, buffer.data(), buffer.size() - 1, 0);
        if (bytesRead > 0)
        {
            buffer[bytesRead] = 0;
            bytesReadTotal += bytesRead;
            bytesReadFile += bytesRead;
            body.insert(body.end(), buffer.begin(), buffer.begin() + bytesRead);
            if (bytesReadFile > mainBoundaryVec.size())
                findBoundary = std::search(body.begin() + bytesReadFile - mainBoundaryVec.size() - 4, body.end(), mainBoundaryVec.begin(), mainBoundaryVec.end());
            else
                findBoundary = std::search(body.begin(), body.end(), mainBoundaryVec.begin(), mainBoundaryVec.end());
        }
    }
    if (findBoundary == body.end() || bytesRead <= 0)
        throw InternalServerError();
}

void Post::handleBoundary(std::string fullRequestPathResource)
{
    if (contentLength == 0)
        throw LengthRequired();
    std::vector<char> buffer(1024);
    std::string body;
    int bytesRead = 0;
    bytesRead = recv(clientSock, buffer.data(), buffer.size() - 1, 0);
    if (bytesRead <= 0)
        throw InternalServerError();
    size_t bytesReadTotal = bytesRead;
    std::string::iterator findBoundary;

    std::string mainBoundaryVec(mainBoundary.begin(), mainBoundary.end());
    buffer[bytesRead] = 0;
    body.insert(body.end(), buffer.begin(), buffer.begin() + bytesRead);
    std::string CRLF = "\r\n";
    if (!std::equal(body.begin() + 2, body.begin() + mainBoundaryVec.size(), mainBoundaryVec.begin()))
        throw BadRequest();
    while (bytesReadTotal != contentLength || !body.empty()){
        findBoundary = std::search(body.begin(), body.end(), mainBoundaryVec.begin(), mainBoundaryVec.end());
        if (findBoundary != body.end()){
            if (findBoundary != body.begin() + 2){
                this->copyToFile(fullRequestPathResource, findBoundary - body.begin() - 4, body);
            }
            body = std::string(findBoundary + mainBoundaryVec.size() + 2, body.end());
            if (!std::equal(body.begin(), body.begin() + 2, CRLF.begin()))
                this->getBoundaryHeaderData(body, bytesReadTotal, fullRequestPathResource);
            else
                body.clear();
            bytesRead = body.size();
        }
        else
            getFileData(findBoundary, body, buffer, bytesReadTotal, bytesRead);
    }
}


void	Post::getBinaryContentDisposition(std::string &fullRequestPathResource, std::string &header)
{
    std::size_t findFilename;
    std::size_t findContentDispostion;

    findContentDispostion = header.find("Content-Disposition: ");
    if (findContentDispostion != std::string::npos)
    {
        contentDisposition = header.substr(header.find("Content-Disposition: "));
        contentDisposition = contentDisposition.substr(0, contentDisposition.find("\r\n"));
        findFilename = contentDisposition.find("filename=");
    }
    else
        findFilename = findContentDispostion;
    if (findFilename != std::string::npos)
    {
        filename = contentDisposition.substr(findFilename + 10);
        filename = filename.substr(0, filename.find("\""));
        std::ofstream file((fullRequestPathResource + filename).c_str(), std::ios::binary);
        file.close();
    }
    else
    {
        std::ofstream file((fullRequestPathResource + "file").c_str(), std::ios::binary);
        file.close();
        filename = "";
    }
}

void Post::handleBinary(const std::string &fullRequestPathResource, Server &web)
{
    std::vector<char> buffer(1024);
    std::string body;
    int bytesRead = 1;
    size_t bytesReadTotal = 0;

    if (transferEncoding != "chunked"){
        while (bytesReadTotal != contentLength && bytesRead > 0){
            bytesRead = recv(clientSock, buffer.data(), buffer.size() - 1, 0);
            if (bytesRead > 0)
            {
                bytesReadTotal += bytesRead;
                body.insert(body.end(), buffer.begin(), buffer.begin() + bytesRead);
                copyToFile(fullRequestPathResource, body.size(), body);
                body.clear();
            }
        }
        if (bytesReadTotal != contentLength || bytesRead <= 0)
                throw InternalServerError();
    }
    else{
        std::string hexVector;
        std::vector<char>    vecBuffer(2);
        size_t  chunkBytes = 0;
        size_t  missingBytes = 0;
        size_t hexNumber = 1;
        std::string CRLF = "\r\n";
        std::string clientMaxBodySize;

        contentLength = 0;
        if (web.locationPath.empty())
            clientMaxBodySize = web.getItemFromServerMap(web, "Server " + web.hostMessageReturn, "client_max_body_size");
        else
            clientMaxBodySize = web.getItemFromLocationMap(web, "Server " + web.hostMessageReturn, "client_max_body_size " + web.locationPath);
        while (hexNumber > 0 && bytesRead > 0)
        {
            while (std::search(hexVector.begin(), hexVector.end(), CRLF.begin(), CRLF.end()) == hexVector.end())
            {
                bytesRead = recv(clientSock, vecBuffer.data(), vecBuffer.size() - 1, 0);
                vecBuffer[1] =  '\0';
                if (bytesRead > 0){
                    hexVector.insert(hexVector.end(), vecBuffer.begin(), vecBuffer.begin() + bytesRead);
                }
            }
            hexNumber = hexToNumber(hexVector);
            missingBytes = hexNumber;
            while (bytesRead > 0 && chunkBytes != hexNumber)
            {
                if (missingBytes < buffer.size())
                    bytesRead = recv(clientSock, buffer.data(), missingBytes, 0);
                else
                    bytesRead = recv(clientSock, buffer.data(), buffer.size(), 0);
                if (bytesRead > 0){
                    missingBytes -= bytesRead;
                    chunkBytes += bytesRead;
                    contentLength += bytesRead;
                    if (clientMaxBodySize != "wrong")
                    {
                        if (contentLength > static_cast<long unsigned int>(atol(clientMaxBodySize.c_str())))
                            throw RequestEntityTooLarge();
                    }
                    body.insert(body.end(), buffer.begin(), buffer.begin() + bytesRead);
                    copyToFile(fullRequestPathResource, body.size(), body);
                    body.clear();
                }
            }
            if (bytesRead > 0)
            {
                bytesRead = recv(clientSock, buffer.data(), 2, 0);
                if (bytesRead < 2 && bytesRead > 0)
                    bytesRead = recv(clientSock, buffer.data(), 1, 0);
            }
            hexVector.clear();
            chunkBytes = 0;
        }
        if (bytesRead <= 0)
            throw InternalServerError();
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

    fullRequestPathResource = Response::findLocationRoot(web, RequestPathResource);
    DIR* directory = opendir(fullRequestPathResource.c_str());
    if (!directory){
        if (web.containsCgi)
            return (web.cgi.handleCgi(fullRequestPathResource, web, header));
        else
            return ("Error 400");
    }
    else
        closedir(directory);
    fullRequestPathResource = fullRequestPathResource + "/";
    try{
        this->getContentTypeData(header);
        this->getLength(header, web);
        this->getTransferEncoding(header);
        if (this->contentType == "multipart/form-data")
            this->handleBoundary(fullRequestPathResource);
        else
        {
            this->getBinaryContentDisposition(fullRequestPathResource, header);
            this->handleBinary(fullRequestPathResource, web);
        }
    }
    catch(std::exception &e){
        std::cout << e.what() << std::endl;
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

const char *Post::InternalServerError::what() const throw(){
	return ("Error 500");
}
