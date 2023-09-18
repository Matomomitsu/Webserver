/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtomomit <mtomomit@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/09/13 21:20:09 by mtomomit          #+#    #+#             */
/*   Updated: 2023/09/17 19:17:27 by mtomomit         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
# define CGI_HPP

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
# include <cstdlib>
# include <algorithm>
# include <sys/types.h>
# include <sys/wait.h>

class Server;

class CGI {
	public:
		std::string	contentType;
		std::size_t	contentLength;
		int			clientSock;
		std::string	transferEncoding;

		void				getContentType(std::string &header);
		void				getLength(std::string &header, Server &web);
		void				getTransferEncoding(std::string &header);
		std::string			handleCgi(const std::string &fullRequestPathResource, Server &web, std::string &header);
		std::vector<char>	handlePost(int &bytesReadInt);
		void				execCgi(const std::string &fullRequestPathResource, Server &web, int *pipefd, int *pipe2fd);
		std::string			receiveOutput(Server &web, int *pipefd, int *pipe2fd, pid_t &pid, std::vector<char> &body);

		class	BadRequest : public std::exception{
			public:
				virtual const char *what() const throw();
		};

		class	NotFound : public std::exception{
			public:
				virtual const char *what() const throw();
		};

		class	UnsupportedMediaType : public std::exception{
			public:
				virtual const char *what() const throw();
		};

		class	LengthRequired : public std::exception{
			public:
				virtual const char *what() const throw();
		};

		class	RequestEntityTooLarge : public std::exception{
			public:
				virtual const char *what() const throw();
		};

		class	InternalServerError : public std::exception{
			public:
				virtual const char *what() const throw();
		};

	private:

};




#endif
