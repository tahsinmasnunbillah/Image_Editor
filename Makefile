TARGET = Image_Editor

SRC = main.c gui.c image.c process.c
OBJ = $(SRC:.c=.o)

CC = gcc
CFLAGS = -Wall -g

OS := $(shell uname -s)

ifeq ($(OS), Darwin)
    IUP_INCLUDE = ./iup/include
    IUP_LIB = ./iup/lib/Mac
    ARCH = -arch arm64
    CFLAGS += $(ARCH) -I$(IUP_INCLUDE) $(shell pkg-config --cflags gtk+-3.0)
    LIBS = $(IUP_LIB)/libiup.a $(shell pkg-config --libs gtk+-3.0) -lm
    RM = rm -f

else
    IUP_INCLUDE = ./iup/include
    IUP_LIB = ./iup/lib/Linux
    CFLAGS += -I$(IUP_INCLUDE) $(shell pkg-config --cflags gtk+-3.0 2>/dev/null || pkg-config --cflags gtk+-2.0)
    LIBS = $(IUP_LIB)/libiup.a $(shell pkg-config --libs gtk+-3.0 2>/dev/null || pkg-config --libs gtk+-2.0) -lm -ldl -lX11
    RM = rm -f
endif

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) $(OBJ) $(TARGET)