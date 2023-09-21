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
