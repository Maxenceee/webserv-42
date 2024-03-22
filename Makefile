OBJ_DIR			=	.objs
SOURCES_DIR		=	sources

SRCS			=	$(shell find $(SOURCES_DIR) -name "*.cpp")
OBJS			=	$(patsubst $(SOURCES_DIR)%.cpp, $(OBJ_DIR)%.o, $(SRCS))

HEADERS_DIR		=	includes
HEADERS			=	$(shell find $(HEADERS_DIR) -name "*.hpp") $(shell find $(SOURCES_DIR) -name "*.hpp")

RM				=	rm -f
CC				=	clang++
CFLAGS			=	-g3 -std=c++98 -I $(HEADERS_DIR) -I $(SOURCES_DIR) -Wall -Wextra -Werror
NAME			=	webserv

GREEN			=	\033[1;32m
BLUE			=	\033[1;34m
RED				=	\033[1;31m
YELLOW			=	\033[1;33m
DEFAULT			=	\033[0m
UP				=	"\033[A"
CUT				=	"\033[K"

all: $(NAME)

$(OBJ_DIR)/%.o: $(SOURCES_DIR)/%.cpp $(HEADERS) Makefile
	@mkdir -p $(@D)
	@echo "$(YELLOW)Compiling [$<]$(DEFAULT)"
	@$(CC) $(CFLAGS) -c $< -o $@
	@printf ${UP}${CUT}

$(NAME): $(OBJS)
	@$(CC) $(OBJS) -o $(NAME)
	@echo "$(GREEN)$(NAME) compiled!$(DEFAULT)"

clean:
	@echo "$(RED)Cleaning build folder$(DEFAULT)"
	@$(RM) -r $(OBJ_DIR)

fclean: clean
	@echo "$(RED)Cleaning $(NAME)$(DEFAULT)"
	@$(RM) $(NAME)

re:				fclean all

.PHONY:			all clean fclean re bonus