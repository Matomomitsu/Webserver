/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Send.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtomomit <mtomomit@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/18 19:58:41 by mtomomit          #+#    #+#             */
/*   Updated: 2023/09/19 15:28:38 by mtomomit         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Send.hpp"

void	Send::sendResponse(int clientSock, Epoll *epoll, std::list<int> clientSockets){
	std::size_t	bytesWritten;

	bytesWritten = send(clientSock, this->response.c_str(), this->response.length(), 0);
	if (this->connection == "close" || bytesWritten <= 0){
		std::cout << "Connection closed" << std::endl;
		close(clientSock);
		epoll_ctl(epoll->epoll_fd, EPOLL_CTL_DEL, clientSock, NULL);
		clientSockets.remove(clientSock);
	}
	this->response.clear();
	this->connection.clear();
}
