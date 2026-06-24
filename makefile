NAME     := webserv
SRCS     := main.cpp Client.cpp Server.cpp utils.cpp
OBJS     := $(SRCS:.cpp=.o)

CXX      := c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(OBJS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

