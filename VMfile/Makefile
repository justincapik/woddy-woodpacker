# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: apsaint- <apsaint-@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2018/11/06 16:34:32 by pharatyk          #+#    #+#              #
#    Updated: 2021/01/15 12:27:18 by jucapik          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = 			woody_woodpacker

OBJ_DIR  =		./obj/

SRC_WWP_DIR =	./src/
SRC_WWP = 		main.c\

SRCS  =	$(addprefix $(SRC_WWP_DIR), $(SRC_WWP))

OBJ  =	$(addprefix $(OBJ_DIR), $(SRC_WWP:.c=.o))

CC = gcc -g -Wall -Wextra -Werror -Wuninitialized
CFLAGS  = -I ./libft/includes -I ./inc/


CURR_NBR = 0
NBR_SRCS = 9
COLS=$(shell tput cols)

####################_OPTIONS_########################
DINO=0
STYLE=1
CLEANSONORE=1
#####################################################

all: $(NAME) $(NAME2)

$(NAME): lib $(OBJ)
	@$(CC) -o $(NAME) $(OBJ) libft/libft.a

lib:
	@make -C ./libft/

clean:
	@make clean -C ./libft/

fclean: clean
	@make fclean -C ./libft/
	@rm -f $(NAME)

$(OBJ_DIR)%.o: $(SRC_WWP_DIR)%.c
	@mkdir -p $(OBJ_DIR)
	@$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@
re: fclean all

.PHONY: all clean fclean re lib $(NAME)
