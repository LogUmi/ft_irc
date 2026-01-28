NAME = ircserv
OBJDIR = obj/
SRCDIR = src/
INCDIR = inc/
FILEC =	main.cpp \
		Channel.cpp \
		Commands.cpp \
		Parser.cpp \
		Server.cpp \
		User.cpp
OBJS = $(patsubst %.cpp, $(OBJDIR)%.o, $(FILEC))
DEPS = $(OBJS:.o=.d)
FLAGS = -Wall -Werror -Wextra -g -std=c++98 -MMD -MP
CXX = c++

all:		$(NAME)

$(NAME):	$(OBJS)
			@$(CXX) $(FLAGS) $(OBJS) -o $(NAME)
			@echo "compilation completed, exec $(NAME) ready."

# Compilation des fichiers .o dans obj/
$(OBJDIR)%.o:	$(SRCDIR)%.cpp | $(OBJDIR)
				@$(CXX) $(FLAGS) -I$(INCDIR) -c $< -o $@

# Création du dossier obj/ si nécessaire
$(OBJDIR):
			@mkdir -p $(OBJDIR)

clean:
			@rm -rf	$(OBJS)
			@rm -rf	$(DEPS)
			@echo "Make clean command executed."

fclean:		clean
			@rm -rf $(NAME)
			@echo "Make fclean command executed."

re:			fclean all

.PHONY:		all clean fclean re

# Inclure les dépendances générées (.d) si elles existent
-include $(DEPS)
