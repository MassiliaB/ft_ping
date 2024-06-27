NAME = ft_ping

SRCS =	src/ft_ping.c src/send_ping.c src/dns_resolve.c

OBJS = $(SRCS:.c=.o)

LDFLAGS = -lm

CC = gcc

CFLAGS = -Wall -Wextra -Werror -I.

RM = rm -f

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS) $(LDFLAGS)

all: $(NAME)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re