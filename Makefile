NAME = webserver
CC = c++
FLAGS = -Wall -Wextra -Werror -Wshadow -std=c++98 
LEAK = -fsanitize=address
SOURCE = main.cpp ./src/server.cpp ./parser/parser.cpp ./src/webserver.cpp\

REMOVE = rm -rf

all: $(NAME) 

$(NAME): $(SRC)
	$(CC) $(FLAGS) $(LEAK) -g $(SOURCE) -o $(NAME) 

clean:
	$(REMOVE) $(NAME)

re: clean all