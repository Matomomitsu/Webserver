/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Sockets.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtomomit <mtomomit@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/08 16:05:48 by mtomomit          #+#    #+#             */
/*   Updated: 2023/09/12 18:41:17 by mtomomit         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Sockets.hpp"

Sockets::Sockets(void){
}

Sockets::~Sockets(void){
}

Sockets & Sockets::operator=(Sockets const & rhs){
	if(this != &rhs){
	}
	return(*this);
}

Sockets::Sockets(Sockets const & src){
	*this = src;
	return ;
}

void	Sockets::handleError(std::string functionName, int epoll_fd, struct addrinfo *servinfo)
{
	std::cerr << functionName << " error: " << gai_strerror(errno) << std::endl;
	for (std::map<int, bool>::iterator it = (this->serverSockets).begin(); it != (this->serverSockets).end(); ++it)
	{
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, it->first, NULL);
		close(it->first);
	}
	if (servinfo != NULL && functionName != "getaddrinfo()")
	{
		freeaddrinfo(servinfo);
	}
	throw FunctionException();
}

void Sockets::setNonBlocking(int fd)
{
	fcntl(fd, F_SETFL, O_NONBLOCK, FD_CLOEXEC);
}

void	Sockets::createSockets(Server web, struct epoll_event *event, int epoll_fd)
{
	for (std::map< std::string, std::map<std::string, std::string> >::iterator outerIt = web.serverMap.begin(); outerIt != web.serverMap.end(); ++outerIt)
	{
		struct addrinfo *servinfo;
		struct addrinfo hints = {};
		int sock;
		int opt = 1;
		std::map<std::string, std::string> &server = outerIt->second;

		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		if (getaddrinfo(server.find("ip")->second.c_str(), server.find("port")->second.c_str(), &hints, &servinfo) != 0)
			handleError("getaddrinfo()", epoll_fd, servinfo);
		sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
		if (sock == -1)
			handleError("socket()", epoll_fd, servinfo);
		setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
		if (bind(sock, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
			handleError("bind()", epoll_fd, servinfo);
		setNonBlocking(sock);
		if (listen(sock, SOMAXCONN) == -1)
			handleError("listen()", epoll_fd, servinfo);
		event->events = EPOLLIN | EPOLLOUT;
		event->data.fd = sock;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, event) == -1)
			handleError("epoll_ctl()", epoll_fd, servinfo);
		this->serverSockets[sock] = true;
		freeaddrinfo(servinfo);
		std::cout << "New server socket fd:" << sock << std::endl;
	}
}

void	Sockets::closeSockets(int epoll_fd)
{
	for (std::map<int, bool>::iterator it = this->serverSockets.begin(); it != this->serverSockets.end(); ++it)
	{
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, it->first, NULL);
		close(it->first);
	}

	for (std::list<int>::iterator it = this->clientSockets.begin(); it != this->clientSockets.end(); ++it)
	{
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, *it, NULL);
		close(*it);
	}
}

const char *Sockets::FunctionException::what() const throw(){
	return ("");
};
