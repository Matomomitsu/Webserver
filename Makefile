NAME = webserver

# Compilation
CC = g++

# Compilation flags
CFLAGS = -Wall -Wextra -Werror -Wshadow -std=c++98

RM = rm -rf

SOURCES = main.cpp \
    src/server.cpp \
    parser/parser.cpp \
    src/webserver.cpp \
    src/Sockets.cpp \
    src/Epoll.cpp \
    src/Response.cpp \
    src/Request.cpp \
    src/Post.cpp \
    src/CGI.cpp \
    src/Send.cpp

SRCS_PATH = ./
SRCS = $(addprefix $(SRCS_PATH), $(SOURCES))

OBJS_PATH = ./objs/
OBJS = $(addprefix $(OBJS_PATH), $(SOURCES:.cpp=.o))

HEADER = src/Server.hpp \
    parser/Parser.hpp \
    src/Webserver.hpp \
    src/Sockets.hpp \
    src/Epoll.hpp \
    src/Response.hpp \
    src/Request.hpp \
    src/Post.hpp \
    src/CGI.hpp \
    src/Send.hpp

all: $(NAME)

$(NAME): $(OBJS) $(HEADER)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJS_PATH)%.o: $(SRCS_PATH)%.cpp $(HEADER)
	@mkdir -p $(OBJS_PATH)src
	@mkdir -p $(OBJS_PATH)parser
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS_PATH)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean
