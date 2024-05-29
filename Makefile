NAME = ft_ping

SRCS =	ft_ping.c send_ping.c dns_resolve.c

OBJS = $(SRCS:.c=.o)

CC = gcc

CFLAGS	= -Wall -Wextra -Werror -I.

RM = rm -f

.c.o =  ${CC} ${CFLAGS} -c $< -o ${$<.c=.o}

$(NAME): ${OBJS}

all: ${NAME}

clean: 
	${RM} ${OBJS}

fclean: clean
	${RM} ${NAME} 

re: fclean all
	echo `clear`

.PHONY: all clean fclean re