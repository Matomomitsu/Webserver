/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Epoll.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtomomit <mtomomit@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/08 16:06:01 by mtomomit          #+#    #+#             */
/*   Updated: 2023/08/08 16:12:49 by mtomomit         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Epoll.hpp"

Epoll::Epoll(void){
	this->epoll_fd = epoll_create(1);
}

Epoll::~Epoll(void){
}

Epoll & Epoll::operator=(Epoll const & rhs){
	if(this != &rhs){
		this->epoll_fd = rhs.epoll_fd;
		this->event = rhs.event;
	}
	return(*this);
}

Epoll::Epoll(Epoll const & src){
	*this = src;
	return ;
}
