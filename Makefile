# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: paulo <paulo@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2018/11/06 16:34:32 by pharatyk          #+#    #+#              #
#    Updated: 2021/03/01 15:38:07 by paulo            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = 			woody_woodpacker
ASM_NAME = 		payload

OBJ_DIR  =		./obj/C/
ASM_OBJ_DIR  =	./obj/ASM/

SRC_WWP_DIR =	./woody_src/
SRC_WWP = 		infector.c\
				main.c\
				patcher.c\
				write.c

SRC_ASM_DIR =	./asm_src/
SRC_ASM =		parasite.asm\
				no_enc_parasite.asm\

SRCS      =	$(addprefix $(SRC_WWP_DIR), $(SRC_WWP))
ASM_SRCS  =	$(addprefix $(SRC_ASM_DIR), $(SRC_ASM))

OBJ  =	$(addprefix $(OBJ_DIR), $(SRC_WWP:.c=.o))
ASM_OBJ  =	$(addprefix $(ASM_OBJ_DIR), $(SRC_ASM:.asm=.bin))

CC = gcc -g #-Wall -Wextra -Werror -Wuninitialized
CFLAGS  = -I ./libft/includes -I ./inc/

####################_OPTIONS_########################
DINO=0
STYLE=1
CLEANSONORE=1
#####################################################

all: $(NAME)

# $(ASM_NAME): $(ASM_OBJ)
# 	@ld -o $(ASM_NAME) $(ASM_OBJ)

$(NAME): lib $(OBJ) $(ASM_OBJ)
	@$(CC) -o $(NAME) $(OBJ) libft/libft.a #$(ASM_OBJ)
	@printf " _/\nwoody  [done]\n"

lib:
	@make -C ./libft/

clean:
	@make clean -C ./libft/
	@rm -f $(OBJ)
	@rm -f $(ASM_OBJ)
	@printf "clean  [done]\n"

fclean: clean
	@make fclean -C ./libft/
	@rm -f $(NAME)
	@rm -f $(ASM_NAME)
	@printf "fclean [done]\n"

$(OBJ_DIR)%.o: $(SRC_WWP_DIR)%.c
	@mkdir -p $(OBJ_DIR)
	@printf ">"
	@$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@


$(ASM_OBJ_DIR)%.bin: $(SRC_ASM_DIR)%.asm
	@mkdir -p $(ASM_OBJ_DIR)
	@printf ">"
	@nasm -f bin $< -o $@
# 	@nasm -f elf64 -g $< -o $@
re: fclean all

.PHONY: all clean fclean re lib $(NAME) $(ASM_NAME)

