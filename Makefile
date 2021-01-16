# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: apsaint- <apsaint-@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2018/11/06 16:34:32 by pharatyk          #+#    #+#              #
#    Updated: 2021/01/16 12:02:25 by pharatyk         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = 			woody_woodpacker

OBJ_DIR  =		./obj/

SRC_WWP_DIR =	./src/
SRC_WWP = 		main.c

SRCS  =	$(addprefix $(SRC_WWP_DIR), $(SRC_WWP))

OBJ  =	$(addprefix $(OBJ_DIR), $(SRC_WWP:.c=.o))

CC = gcc -g -Wall -Wextra -Werror -Wuninitialized
CFLAGS  = -I ./libft/includes -I ./inc/

####################_OPTIONS_########################
DINO=0
STYLE=1
CLEANSONORE=1
#####################################################

all: $(NAME)

$(NAME): lib $(OBJ)
	@$(CC) -o $(NAME) $(OBJ) libft/libft.a
	@printf " _/\nwoody  [done]\n"

lib:
	@make -C ./libft/

clean:
	@make clean -C ./libft/
	@rm -f $(OBJ)
	@printf "clean  [done]\n"

fclean: clean
	@make fclean -C ./libft/
	@rm -f $(NAME)
	@printf "fclean [done]\n"

$(OBJ_DIR)%.o: $(SRC_WWP_DIR)%.c
	@mkdir -p $(OBJ_DIR)
	@printf ">"
	@$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
re: fclean all

.PHONY: all clean fclean re lib $(NAME)
