/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Send.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtomomit <mtomomit@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/18 19:58:35 by mtomomit          #+#    #+#             */
/*   Updated: 2023/09/19 12:24:47 by mtomomit         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <iostream>
# include <fstream>
# include <sstream>
# include <cstdio>
# include <list>
# include <map>
# include <sys/types.h>
# include <stdio.h>
# include <cstdlib>
# include <string>
# include <dirent.h>
# include "Epoll.hpp"

class Send {
	public:
		std::string response;
		std::string connection;

		void	sendResponse(int client_sock, Epoll *epoll, std::list<int> clientSockets);
	private:

};
