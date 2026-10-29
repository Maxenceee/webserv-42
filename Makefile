NAME            := webserv

SOURCES_DIR     := sources
HEADERS_DIR     := includes
OBJ_DIR         := .objs

CC              := g++
RM              := rm -rf

CXXFLAGS        := -Wall -Wextra -Werror -std=c++98 -g3
DEPSFLAGS       := -MMD -MP
INCLUDES        := -I $(HEADERS_DIR) -I $(SOURCES_DIR)
LIBS            := -lpthread -lcrypto -lssl

CFLAGS          := $(CXXFLAGS) $(DEPSFLAGS) $(INCLUDES)

SRCS            := $(shell find $(SOURCES_DIR) -name "*.cpp" 2>/dev/null)
OBJS            := $(patsubst $(SOURCES_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
ALL_DEPS        := $(OBJS:.o=.d)

TOTAL_FILES     := $(words $(OBJS))
CURRENT_FILE    := 0

BOLD            := \033[1m
DIM             := \033[2m
CYAN            := \033[36m
GREEN           := \033[32m
YELLOW          := \033[33m
BLUE            := \033[34m
MAGENTA         := \033[35m
RED             := \033[31m
RESET           := \033[0m

define update_progress
	$(eval CURRENT_FILE=$(shell expr $(CURRENT_FILE) + 1))
	$(eval PERCENT=$(shell expr $(CURRENT_FILE) \* 100 / $(TOTAL_FILES)))
	$(eval PROGRESS=$(shell expr $(CURRENT_FILE) \* 20 / $(TOTAL_FILES)))
	$(eval REMAINING=$(shell expr 20 - $(PROGRESS)))
	@printf "\r\033[K   $(GREEN)✔$(RESET) %b %-50s$(RESET)\n" "$(1)" "$(2)"
	@printf "   [$(GREEN)%s$(DIM)%s$(RESET)] %3d%%" \
		"$(shell printf '%0.s#' $$(seq 1 $(PROGRESS) 2>/dev/null))" \
		"$(shell printf '%0.s-' $$(seq 1 $(REMAINING) 2>/dev/null))" \
		"$(PERCENT)"
endef

define get_file_size
$$(du -h $(1) 2>/dev/null | awk '{print $$1}')
endef

all: $(NAME)

$(OBJ_DIR)/%.o: $(SOURCES_DIR)/%.cpp Makefile
	@mkdir -p $(@D)
	$(call update_progress,$(CYAN)Compiling C++$(RESET), $<)
	@$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	@printf "\r\033[K\n   $(BOLD)$(BLUE)Linking $(NAME)...$(RESET)\n"
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME) $(LIBS)
	@if [ -f $(NAME) ]; then \
		printf "   $(GREEN)✔ $(NAME) compiled successfully!$(RESET) $(DIM)(%s)$(RESET)\n\n" "$(call get_file_size,$(NAME))"; \
	else \
		printf "   $(RED)✖ Failed to create $(NAME)$(RESET)\n\n"; \
		exit 1; \
	fi

-include $(ALL_DEPS)

clean:
	@printf "   $(RED)➤ Cleaning objects...$(RESET)\n"
	@$(RM) $(OBJ_DIR)

fclean: clean
	@printf "   $(RED)➤ Removing executable...$(RESET)\n"
	@$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re