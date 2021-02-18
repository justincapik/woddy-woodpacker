# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: kemartin <kemartin@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2019/07/18 15:47:39 by kemartin          #+#    #+#              #
#    Updated: 2021/02/18 15:34:13 by pharatyk         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = libft.a
SRC_DIR = ./src/
OBJ_DIR = ./obj/
SRC_NAME =	csp_specifier.c\
			ft_memcpy.c\
			ft_strlen.c\
			dioux_specifier.c\
			ft_memdel.c\
			ft_strmap.c\
			diu_arg_detect.c\
			ft_memmove.c\
			ft_strmapi.c\
			f_specifier.c\
			ft_memset.c\
			ft_strncat.c\
			ft_atof.c\
			ft_printfd.c\
			ft_strncpy.c\
			ft_atoi.c\
			ft_putchar.c\
			ft_strndup.c\
			ft_bzero.c\
			ft_putchar_fd.c\
			ft_strnequ.c\
			ft_ftoa.c\
			ft_putendl.c\
			ft_strnew.c\
			ft_get_next_line.c\
			ft_putendl_fd.c\
			ft_strnstr.c\
			ft_isalnum.c\
			ft_putnbr.c\
			ft_strrchr.c\
			ft_isalpha.c\
			ft_putnbr_fd.c\
			ft_strsplit.c\
			ft_isascii.c\
			ft_putstr.c\
			ft_strstr.c\
			ft_isdigit.c\
			ft_putstr_fd.c\
			ft_strsub.c\
			ft_isprint.c\
			ft_strcat.c\
			ft_strtrim.c\
			ft_isspace.c\
			ft_strchr.c\
			ft_tolower.c\
			ft_itoa.c\
			ft_strclr.c\
			ft_toupper.c\
			ft_lstadd.c\
			ft_strcmp.c\
			ft_unitoa.c\
			ft_lstdel.c\
			ft_strcpy.c\
			moinsspluss.c\
			ft_lstdelone.c\
			ft_strdel.c\
			ol.c\
			ft_lstiter.c\
			ft_strdup.c\
			ox_arg_detect.c\
			ft_lstmap.c\
			ft_strequ.c\
			signs.c\
			ft_lstnew.c\
			ft_striter.c\
			silly_detect.c\
			ft_memalloc.c\
			ft_striteri.c\
			silly_detect_2.c\
			ft_memccpy.c\
			ft_strjoin.c\
			struct_tools.c\
			ft_memchr.c\
			ft_strjoin_free.c\
			tools.c\
			ft_memcmp.c\
			ft_strlcat.c\
			widthower.c\
			ft_printf.c\
			ft_strncmp.c\
			ft_abs.c\
			lifetime.c

CC = gcc -g -Wall -Wextra -Werror -Wuninitialized
CFLAGS = -I ./includes/

SRCS = $(addprefix $(SRC_DIR), $(SRC_NAME))
OBJ = $(addprefix $(OBJ_DIR), $(SRC_NAME:.c=.o))

####################_OPTIONS_########################
DINO=1
STYLE=1
CLEANSONORE=0
#####################################################

$(NAME): $(OBJ)
	@ar rcs $(NAME) $(OBJ)
	@printf " _/\nlibft  [done]\n"

all: $(NAME)

clean:
	@rm -f $(OBJ)

fclean: clean
	@rm -f $(NAME)

re: fclean all

$(OBJ_DIR)%.o : $(SRC_DIR)%.c
	@mkdir -p $(OBJ_DIR)
	@printf ">"
	@$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

.PHONY: all clean fclean re
