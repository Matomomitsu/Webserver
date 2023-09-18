#include "CGI.hpp"
#include "Response.hpp"

size_t hexToNumber(const std::string& hexString) {
    unsigned int result;
    std::stringstream ss;
    ss << std::hex << hexString;
    ss >> result;
    return static_cast<size_t>(result);
}

void CGI::getTransferEncoding(std::string &header)
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

void CGI::getContentType(std::string &header)
{
    std::size_t findContent;

    findContent = header.find("Content-Type: ");
    if (findContent != std::string::npos)
        contentType = header.substr(findContent);
    else{
		contentType = "CONTENT_TYPE=";
		return ;
	}
        ;
    contentType = contentType.substr(0, contentType.find("\r\n"));
    contentType = "CONTENT_TYPE=" + contentType.substr(14);
}

void CGI::getLength(std::string &header, Server &web)
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

void    CGI::execCgi(const std::string &fullRequestPathResource, Server &web, int *pipefd, int *pipe2fd)
{
    std::string cLength;
    std::string cType;
    std::string fullPath;
    std::string requestMethod;

    fullPath = "PATH_INFO=" + fullRequestPathResource;
    cLength = "CONTENT_LENGTH=" + Request::itoa(contentLength);
    requestMethod = "REQUEST_METHOD=" + web.method;
    close(pipefd[0]);
    close(pipe2fd[1]);
    dup2(pipefd[1], STDOUT_FILENO);
    dup2(pipe2fd[0], STDIN_FILENO);
    close(pipe2fd[0]);
    close(pipefd[1]);
    char* argv[] = { (char*)web.cgiInit.c_str(), (char*)fullRequestPathResource.c_str(), NULL };
    char* envp[] = { (char *)web.queryString.c_str(), (char *)requestMethod.c_str(), (char *)cLength.c_str(), (char *)contentType.c_str(), (char *)fullPath.c_str(), NULL};
    if (execve(web.cgiInit.c_str(), argv, envp) == -1) {
        std::cerr << "Execve error: " << std::strerror(errno) << '\n';
        exit(1);
    }
}

std::vector<char>   CGI::handlePost(int &bytesReadInt)
{
    std::vector<char> body;
    size_t bytesReadTotal = 0;
    std::vector<char> vecBuffer(1024);
    size_t hexNumber = 1;

    if (transferEncoding != "chunked"){
        while (bytesReadTotal != contentLength && bytesReadInt > 0){
            bytesReadInt = recv(clientSock, vecBuffer.data(), vecBuffer.size() - 1, 0);
            if (bytesReadInt > 0){
                bytesReadTotal += bytesReadInt;
                body.insert(body.end(), vecBuffer.begin(), vecBuffer.begin() + bytesReadInt);
            }
        }
    }
    else{
        std::string hexString = "";
        char    buffer[2] = {0};
        size_t  chunkBytes = 0;
        size_t  missingBytes = 0;

        contentLength = 0;
        while (hexNumber > 0 && bytesReadInt > 0)
        {
            while (hexString.find("\r\n") == std::string::npos && bytesReadInt > 0)
            {
                bytesReadInt = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
                if (bytesReadInt > 0){
                    hexString += buffer;
                }
            }
            hexNumber = hexToNumber(hexString);
            if (hexNumber > vecBuffer.size())
                missingBytes = vecBuffer.size();
            else
                missingBytes = hexNumber;
            while (bytesReadInt > 0 && chunkBytes != hexNumber)
            {
                bytesReadInt = recv(clientSock, vecBuffer.data(), missingBytes, 0);
                if (bytesReadInt > 0){
                    missingBytes -= bytesReadInt;
                    chunkBytes += bytesReadInt;
                    contentLength += bytesReadInt;
                    body.insert(body.end(), vecBuffer.begin(), vecBuffer.begin() + bytesReadInt);
                }
            }
            if (bytesReadInt > 0)
            {
                bytesReadInt = recv(clientSock, vecBuffer.data(), 2, 0);
                if (bytesReadInt < 2 && bytesReadInt > 0)
                    bytesReadInt = recv(clientSock, vecBuffer.data(), 1, 0);
            }
            hexString.clear();
            chunkBytes = 0;
        }
    }
    return (body);
}

std::string    CGI::receiveOutput(Server &web, int *pipefd, int *pipe2fd, pid_t &pid, std::vector<char> &body)
{
    char                buffer[1024];
    std::string         output;
    size_t              bytesRead;
    int                 status = 0;
    std::string         contentTypeOutput;
    size_t              findContentTypeOutput;
    size_t              bytesWritten;

    close(pipe2fd[0]);
    bytesWritten = write(pipe2fd[1], body.data(), body.size());
    close(pipe2fd[1]);
    if (bytesWritten != contentLength)
    {
        close(pipefd[1]);
        close(pipefd[0]);
        waitpid(pid, &status, 0);
        return ("Error 500");
    }
    close(pipefd[1]);
    while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
    {
        buffer[bytesRead] = '\0';
        output += buffer;
    }
    close(pipefd[0]);
    waitpid(pid, &status, 0);
    if (status != 0)
        return ("Error 500");
    findContentTypeOutput = output.find("Content-Type");
    if (findContentTypeOutput != std::string::npos){
        contentTypeOutput = output.substr(findContentTypeOutput);
        contentTypeOutput = contentTypeOutput.substr(0, contentTypeOutput.find("\r\n"));
        web.contentType = contentTypeOutput.substr(contentTypeOutput.find(":") + 1);
    }
    output = output.substr(output.find("\r\n\r\n") + 4);
    output = Response::createResponseMessage(web, output);
    return output;
}

std::string CGI::handleCgi(const std::string &fullRequestPathResource, Server &web, std::string &header)
{
    int pipefd[2];
    int pipe2fd[2];
    pid_t pid;
    std::vector<char>   body;
    int                 bytesReadInt = 1;

    try{
        if (access(fullRequestPathResource.c_str(), F_OK) == -1)
            throw NotFound();
        this->getContentType(header);
        this->getLength(header, web);
        if (web.method == "POST")
        {
            this->getTransferEncoding(header);
            if (contentType != "CONTENT_TYPE=application/x-www-form-urlencoded" && \
                contentType.substr(0, 32) != "CONTENT_TYPE=multipart/form-data")
                throw UnsupportedMediaType();
            body = handlePost(bytesReadInt);
            if (bytesReadInt < 1)
                throw InternalServerError();
        }
    }
    catch(std::exception &e){
        std::cout << e.what() << std::endl;
        return (e.what());
    }
    if (web.cgiInit == "")
        return ("Error 400");
    pipe(pipefd);
    pipe(pipe2fd);
    pid = fork();
    if (pid == 0)
        this->execCgi(fullRequestPathResource, web, pipefd, pipe2fd);
    else if (pid > 0)
        return (this->receiveOutput(web, pipefd, pipe2fd, pid, body));
    else
        std::cerr << "Fork error\n";
    return "Error 400";
}

const char *CGI::BadRequest::what() const throw(){
	return ("Error 400");
}

const char *CGI::NotFound::what() const throw(){
	return ("Error 404");
}

const char *CGI::LengthRequired::what() const throw(){
	return ("Error 411");
}

const char *CGI::RequestEntityTooLarge::what() const throw(){
	return ("Error 413");
}

const char *CGI::UnsupportedMediaType::what() const throw(){
	return ("Error 415");
}

const char *CGI::InternalServerError::what() const throw(){
	return ("Error 500");
}
