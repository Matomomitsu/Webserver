/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Sockets.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtomomit <mtomomit@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/08 16:05:51 by mtomomit          #+#    #+#             */
/*   Updated: 2023/08/23 13:54:14 by mtomomit         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKETS_HPP
# define SOCKETS_HPP

# include <iostream>
# include <cerrno>
# include <sys/epoll.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <fcntl.h>
# include <unistd.h>
# include <map>
# include <vector>
# include <string>
# include <csignal>
# include <cstdlib>
# include <list>
# include <netdb.h>
# include <sstream>
# include "./Server.hpp"

class Sockets {
	public:
		Sockets(void);
		virtual ~Sockets(void);
		Sockets & operator=(Sockets const & rhs);
		Sockets(Sockets const & src);

		void	createSockets(Server web, struct epoll_event *event, int epoll_fd);
		void 	setNonBlocking(int fd);
		void	closeSockets(int epoll_fd);

		std::list<int> clientSockets;
		std::map<int, bool> serverSockets;

		class	FunctionException : public std::exception{
			public:
				virtual const char *what() const throw();
		};

	private:
		void	handleError(std::string functionName, int epollFd, addrinfo *servinfo);
};
