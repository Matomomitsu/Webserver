#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <vector>
# include <fcntl.h>
# include <fstream>
# include <sstream>
# include <cstdio>
# include <vector>
# include <map>
# include <sys/types.h>
# include <stdio.h>
# include <netdb.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <string>
# include <dirent.h>
# include "Server.hpp"
# include "Response.hpp"
# include "Epoll.hpp"


class Request {
	public:
		static bool  checkGetRequest( Server &web, const std::string& message, std::string method);
		static void handleClient(Server web, int client_sock, Epoll *epoll, std::list<int> clientSockets);
		static std::string itoa(int num);
	private:

};

#endif
