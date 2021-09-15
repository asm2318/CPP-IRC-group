SRCS	=	src/main.cpp		src/Server.cpp		src/Client.cpp\
		src/Exception.cpp	src/TextHolder.cpp	src/Channel.cpp

INCL	=	src/Server.hpp		src/Client.hpp		src/Exception.hpp\
		src/TextHolder.hpp	src/Channel.hpp

OBJS	=	${SRCS:.cpp=.o}

NAME	=	ircserv

CC	=	clang++

FLAGS	=	#-Wall -Wextra -Werror -std=c++98

RM	=	-rm -f

$(NAME):	${OBJS} $(INCL)
	${CC} ${FLAGS} ${OBJS} -o ${NAME}

all:	${NAME}

.cpp.o:
	${CC} ${FLAGS} -c $< -o ${<:.cpp=.o}

clean:	
	${RM} src/*.o	

fclean:	clean
	${RM} ${NAME}

re:	fclean all

.PHONY:	all clean fclean re