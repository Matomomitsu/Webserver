#include "./parser/Parser.hpp"
#include "./src/Server.hpp"
#include "./src/Webserver.hpp"
#include "./src/Sockets.hpp"
#include "./src/Epoll.hpp"
#include <signal.h>
#include <cstdlib>

/*void sigIntHandlerFunc(int sig){
    (void)sig;
    std::cout << "\nFinishing the WebServer ...\n";
    //webserver.finish();
    exit(0);
}*/

bool stop_server = false;

void signalHandler(int signal_num){
	(void)signal_num;
	std::cout << "Program interrupted by user" << std::endl;
	stop_server = true;
}

std::string testeCaminhoDoRecurso;

void handleClient(Server web, int client_sock, Epoll *epoll, std::list<int> clientSockets)
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
		std::string http_response = web.responseRequest(web, pathGetRequestFile);

		if (http_response == "Error 404"){
            std::string response = "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: 14\r\n\r\n404 Not Found\n";
             send(client_sock, response.c_str(), response.length(), 0);
        }
        else
            send(client_sock, http_response.c_str(), http_response.length(), 0);
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


int main (int argc, char *argv[]){

    Parser parser;
    Server web;
    Sockets			sockets;
	Epoll			epoll;

     if(argc != 2){
       std::cout << "Erro no arquivo de entrada" << std::endl;
       return(-1);
    }

    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = signalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    web = parser.parserFile(argv[1]);
    //run(web);
	try{
		sockets.createSockets(web, &epoll.event, epoll.epoll_fd);
	}
	catch(std::exception &e){
		stop_server = true;
	}
	while (!stop_server)
	{
		struct epoll_event events[MAX_EVENTS];
		int num_events = epoll_wait(epoll.epoll_fd, events, MAX_EVENTS, -1);

		for (int i = 0; i < num_events; i++)
		{
			int fd = events[i].data.fd;

			if (sockets.serverSockets[fd])
			{
					struct sockaddr_in client_addr;
					socklen_t client_addr_len = sizeof(client_addr);
					int client_sock = accept(fd , (struct sockaddr*)&client_addr, &client_addr_len);
					if (client_sock == -1)
					{
						std::cout << "Error to accept new socket" << std::endl;
						break;
					}

					sockets.setNonBlocking(client_sock);
					epoll.event.events =  EPOLLIN;
					epoll.event.data.fd = client_sock;
					epoll_ctl(epoll.epoll_fd, EPOLL_CTL_ADD, client_sock, &epoll.event);
					sockets.clientSockets.push_front(client_sock);

					std::cout << "New client connection accepted. Socket: " << client_sock << std::endl;
			}
			else
			{
				if (events[i].events & EPOLLIN)
				{
					handleClient(web, fd, &epoll, sockets.clientSockets);
				}
			}
		}
	}
    sockets.closeSockets(epoll.epoll_fd);
    return(0);
}
