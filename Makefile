CFLAGS+=$(shell libftdi-config --cflags) -pthread -O0 -ggdb3
LDFLAGS+=$(shell libftdi-config --libs) -pthread

all: ft232-servo
