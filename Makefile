CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g3 -MMD -MP
INCLUDE = include/
NAME = ircserv

SRC = src/main.cpp \
	src/Bot.cpp \
	src/exec.cpp \
	src/utils.cpp \
	src/Server.cpp \
	src/Client.cpp \
	src/Channel.cpp \
	

OBJS_BASE = $(SRC:.cpp=.o)
DEPS_BASE = $(SRC:.cpp=.d)
OBJ_PATH = .obj/
OBJS = $(addprefix $(OBJ_PATH),$(OBJS_BASE))
DEPS = $(addprefix $(OBJ_PATH),$(DEPS_BASE))

all: $(OBJ_PATH) $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -I $(INCLUDE) -o $(NAME) $(OBJS)

$(OBJ_PATH)%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I $(INCLUDE) -c $< -o $@

$(OBJ_PATH):
	mkdir -p .obj/
	mkdir -p .obj/src/

clean:
	rm -rf ".vscode"
	rm -rf $(OBJ_PATH)
	rm -f $(OBJS) $(DEPS)

fclean: clean
	rm -f $(NAME)

re: fclean 
	${MAKE} all

-include $(DEPS)

.PHONY: all clean fclean re
