/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Post.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mtomomit <mtomomit@student.42sp.org.br>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/08/25 20:02:59 by mtomomit          #+#    #+#             */
/*   Updated: 2023/08/31 15:03:32 by mtomomit         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef POST_HPP
# define POST_HPP

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
# include "Server.hpp"
# include "Response.hpp"

class Post {
	public:
		std::string	contentType;
		std::string	mainBoundary;
		std::string	contentDisposition;
		std::string	filename;
		std::size_t	contentLength;
		std::ofstream currentFile;
		int			clientSock;
		std::string	postResponse(Server &web, std::string RequestPathResource, std::string header);
		void		getContentTypeData(std::string header);
		void		getLength(std::string header, Server &web);
		void		handleBoundary(std::string fullRequestPathResource);
		void		getBoundaryHeaderData(std::string &body, std::size_t &bytesReadTotal, std::string &fullRequestPathResource);
		void		copyToFile(std::string &fullRequestPathResource, std::size_t limiter, std::string &body);
		std::string	createResponseMessage(std::string &fullRequestPathResource);

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

	private:

};

#endif
