SRCS	=	src/main.cpp		src/Server.cpp		src/Client.cpp\
		src/Exception.cpp	src/TextHolder.cpp	src/Channel.cpp

SRC_BOT	=	src/main_bot.cpp	src/Bot.cpp

INCL	=	src/Server.hpp		src/Client.hpp		src/Exception.hpp\
		src/TextHolder.hpp	src/Channel.hpp

INC_BOT	=	src/Bot.hpp

OBJS	=	${SRCS:.cpp=.o}

OBJ_BOT	=	${SRC_BOT:.cpp=.o}

NAME	=	ircserv

NME_BOT	=	bot

CC	=	clang++

FLAGS	=	-Wall -Wextra -Werror -std=c++98

RM	=	-rm -f

$(NAME):	${OBJS} $(INCL) ${NME_BOT}
	${CC} ${FLAGS} ${OBJS} -o ${NAME}

$(NME_BOT):	${OBJ_BOT} $(INC_BOT)
	${CC} ${FLAGS} ${OBJ_BOT} -o ${NME_BOT}

all:	${NAME}

.cpp.o:
	${CC} ${FLAGS} -c $< -o ${<:.cpp=.o}

clean:	
	${RM} src/*.o	

fclean:	clean
	${RM} ${NAME} ${NME_BOT}

re:	fclean all

.PHONY:	all clean fclean re