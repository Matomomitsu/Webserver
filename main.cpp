#include "./parser/Parser.hpp"
#include "./src/Server.hpp"
#include "./src/Webserver.hpp"
#include "./src/Sockets.hpp"
#include "./src/Request.hpp"
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
					Request::handleClient(web, fd, &epoll, sockets.clientSockets);
				}
			}
		}
	}
    sockets.closeSockets(epoll.epoll_fd);
    return(0);
}
