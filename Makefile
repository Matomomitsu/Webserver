NAME = webserver
CC = c++
FLAGS = -Wall -Wextra -Werror -Wshadow -std=c++98
LEAK = -fsanitize=address
SOURCE = main.cpp ./src/server.cpp ./parser/parser.cpp ./src/webserver.cpp ./src/Sockets.cpp ./src/Epoll.cpp ./src/Response.cpp ./src/Request.cpp ./src/Post.cpp\

REMOVE = rm -rf

all: $(NAME)

$(NAME): $(SOURCE)
	$(CC) $(FLAGS) -g $(SOURCE) -o $(NAME)

clean:
	$(REMOVE) $(NAME)

re: clean all
