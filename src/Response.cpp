#include "Response.hpp"


std::string itoa(int num) {
	std::ostringstream oss;
	oss << num;
	return oss.str();
}

std::string handleCgi(const std::string &fullRequestPathResource, Server &web)
{
    int pipefd[2];
    pid_t pid;

    if (web.cgiInit == ""){
        return ("Error 400");
    }
    pipe(pipefd);
    pid = fork();
    if (pid == 0){
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        char* argv[] = { (char*)web.cgiInit.c_str(), (char*)fullRequestPathResource.c_str(), NULL };
        char* envp[] = { (char *)web.queryString.c_str(), NULL};
        if (execve(web.cgiInit.c_str(), argv, envp) == -1) {
            std::cerr << "Execve error: " << std::strerror(errno) << '\n';
            exit(1);
        }
    }
    else if (pid > 0){
        char buffer[1024];
        std::string body;
        ssize_t bytesRead;
        int status;

        close(pipefd[1]);
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[bytesRead] = '\0';
            body += buffer;
        }
        close(pipefd[0]);
        waitpid(pid, &status, 0);
        body = Response::createResponseMessage(body);
        return body;
    }
    else {
        std::cerr << "Fork error\n";
        return "Error 400";
    }
    return "Error 400";
}

std::string Response::checkLocationRoot(const std::vector<std::string>& webPathSegments, std::map <std::string, std::string>& location, Server &web) {
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

std::vector<std::string> Response::splitPath(const std::string& path, char delimiter) {
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

std::string  Response::findLocationRoot(Server &web, std::string RequestPathResource){
    std::map<std::string, std::map<std::string, std::string> >::iterator outerIt;

    for (outerIt = web.locationMap.begin(); outerIt != web.locationMap.end(); ++outerIt){
        if (outerIt->first == "Server " + web.hostMessageReturn){
            if (web.pathSegments.empty()){
                size_t  findContentTypeReturn;
                web.contentType = "";
                web.pathSegments = splitPath(RequestPathResource, '/');
                if (!web.pathSegments.empty()){
                    findContentTypeReturn = web.pathSegments[web.pathSegments.size() - 1].rfind(".");
                    if (findContentTypeReturn != std::string::npos)
                        web.contentType = web.pathSegments[web.pathSegments.size() - 1].substr(findContentTypeReturn + 1);
                }
            }
            if (web.locationRoot.empty())
                web.locationRoot = checkLocationRoot(web.pathSegments, outerIt->second, web);
            if (web.locationRoot.empty()){
                std::map<std::string, std::map<std::string, std::string> >::iterator serverIt;
                for (serverIt = web.serverMap.begin(); serverIt != web.serverMap.end(); ++serverIt){
                    if (serverIt->first == "Server " + web.hostMessageReturn)
                        web.locationRoot = serverIt->second["root"];
                }
            }
            if (web.locationPath != "/")
                RequestPathResource = web.locationRoot + RequestPathResource.substr(web.locationPath.length(), RequestPathResource.length() - web.locationPath.length());
            else
                RequestPathResource = web.locationRoot + RequestPathResource.substr(0, RequestPathResource.length());
        }
    }
    return(RequestPathResource);
}

std::string  Response::getResponseFile(std::string responseRequestFilePath, Server &web, std::string RequestPathResource){
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
                    closedir(directory);
                    return (getResponseFile(responseRequestFilePath, web, RequestPathResource));
                }
            }
        }
        closedir(directory);
        std::vector<std::string>().swap(web.pathSegments);
        return("Error 404");
    }
    response = createResponseMessage(content);
    closedir(directory);
    std::vector<std::string>().swap(web.pathSegments);
    return (response);
}

std::string  Response::responseRequest(Server &web, std::string RequestPathResource){
    std::string fullRequestPathResource;

    fullRequestPathResource = findLocationRoot(web, RequestPathResource);
    if(web.containsCgi)
        return(handleCgi(fullRequestPathResource, web));
    std::map<std::string, std::string> keyValueMap;
    std::string response;
    response = getResponseFile(fullRequestPathResource, web, RequestPathResource);
    return(response);
}

void Response::addIndex(Server &web, std::string &path){
    std::string currentPath;
    std::string matchingLocationPath;
    std::string fileToReturn = "index.html";
    std::string temp;

    if (web.locationRoot != web.getItemFromServerMap(web, "Server " + web.hostMessageReturn, "root")){
        temp = web.getItemFromLocationMap(web, "Server " + web.hostMessageReturn, "index " + web.locationPath);
        if (temp != "wrong")
            fileToReturn = temp;
    }
    else{
        fileToReturn = web.getItemFromServerMap(web, "Server " + web.hostMessageReturn, "index");
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

std::string  Response::createResponseMessage(std::string body){
    std::string body_size = itoa(body.size() + 1);
    std::string response = "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/html\r\n"
                            "Content-Length: " + body_size + "\r\n"
                            "\r\n"
                            + body + "\n";
    return(response);
}

std::string Response::createResponseMessageWithError(std::string body, std::string erro, std::string messageErro){
    std::string body_size = Request::itoa(body.size() + 1);
    std::string response = "HTTP/1.1 "+ erro +" "+messageErro+"\r\n"
                            "Content-Type: text/html\r\n"
                            "Content-Length: " + body_size + "\r\n"
                            "\r\n"
                            + body + "\n";
    return(response);
}


std::string  getResponseFileDefault(std::string responseFileDefault){
    std::ifstream file(responseFileDefault.c_str());
    std::string content;
    std::string response;
    DIR* directory = opendir(responseFileDefault.c_str());

    if (file.is_open() && !directory){
        std::string line;
        while(std::getline(file, line)){
            content += line;
        }
        file.close();
    }
    else
        return("Error 404");
    return (response);
}

std::string Response::deleteResponse(Server &web, std::string pathToDelete){
    std::string rootPath = findLocationRoot(web, pathToDelete);
    const char* filename = rootPath.c_str();
    std::string response;

    if (access(filename, F_OK) != -1){
        std::cout << "Arquivo no caminho " << rootPath << " Localizado." << std::endl;
        if (std::remove(filename) == 0) {
            std::cout << "Arquivo no caminho " << rootPath << " deletado." << std::endl;
            std::string body = getResponseFileDefault("./utils/deleteSucces.html");
            response = Response::createResponseMessage(body);
            return(response);
        } else {
            response = "Error 403";
            return(response);
        }
    }
    else
    {
        std::cout << "O arquivo não existe." << std::endl;
        response = "Error 404";
        std::cout << response << std::endl;
    }
    return(response);
}

std::string getErrorReturn(std::string path){
    std::ifstream filetoOpen(path.c_str());
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
        std::cout <<  "File Not Found, Please Check The path to default file of return" << std::endl;
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

std::string Response::errorType(std::string erro){
    std::string body;
    std::string content;

    if (erro == "Error 404"){
        body = getErrorReturn("./utils/error_page/404.html");
        content = Response::createResponseMessageWithError(body, "404", "Not Found");
        return (content);
    }
    else if(erro == "Error 403"){
        body = getErrorReturn("./utils/error_page/403.html");
        content = Response::createResponseMessageWithError(body, "403", "Forbidden");
        return (content);
    }
    else if(erro == "Error 400"){
        body = getErrorReturn("./utils/error_page/400.html");
        content = Response::createResponseMessageWithError(body, "400", "Bad Request");
        return (content);
    }
    else if(erro == "Error 411"){
        body = getErrorReturn("./utils/error_page/411.html");
        content = Response::createResponseMessageWithError(body, "411", "Length Required");
        return (content);
    }
    else if(erro == "Error 413"){
        body = getErrorReturn("./utils/error_page/413.html");
        content = Response::createResponseMessageWithError(body, "413", "Request Entity too Large");
        return (content);
    }
    else if(erro == "Error 415"){
        body = getErrorReturn("./utils/error_page/415.html");
        content = Response::createResponseMessageWithError(body, "415", "Unsupported Media Type");
        return (content);
    }
    else{
        content = "OK";
        return (content);
    }
    return(content);
}
