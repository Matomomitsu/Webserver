/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtomomit <mtomomit@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/08 16:05:46 by mtomomit          #+#    #+#             */
/*   Updated: 2023/08/23 13:53:37 by mtomomit         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EPOLL_HPP
# define EPOLL_HPP

# include <iostream>
# include <cerrno>
# include <sys/epoll.h>
# include <sys/socket.h>
# include <netinet/in.h>
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

#define MAX_EVENTS 40

class Epoll {
	public:
		Epoll(void);
		virtual ~Epoll(void);
		Epoll & operator=(Epoll const & rhs);
		Epoll(Epoll const & src);

		int epoll_fd;
		struct epoll_event event;

	private:

};
