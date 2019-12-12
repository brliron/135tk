CXX	= g++
LD	= g++

ifeq ($(OS),Windows_NT)
NAME	= th135arc-alt.exe
else
NAME	= th135arc-alt
endif

SRCS	= main.cpp \
	  TFPK.cpp \
	  FnList.cpp \
	  FilesList.cpp \
	  Rsa.cpp \
	  OS.cpp \

OBJS	= $(SRCS:.cpp=.o)

CXXFLAGS = -Wall -Wextra -std=c++2a
ifeq ($(OS),Windows_NT)
CXXFLAGS += -municode
endif

CXXFLAGS += -IMIRACL

LDFLAGS	= MIRACL/miracl.a -lz -g

all: $(NAME)

$(NAME): $(OBJS)
	$(LD) $(OBJS) -o $(NAME) $(LDFLAGS)

clean:
	rm $(NAME) $(OBJS)
