CC=gcc

OPTS=-c -Wall

SOURCES=$(wildcard *.c)

OBJECTS=$(SOURCES:.c=.o)

LIBS=-lxdo

EXECUTABLE=joystick-control

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(LINK.o) $^ -o $@ $(LIBS)

clean:
	rm $(EXECUTABLE) $(OBJECTS)

install:
	cp $(EXECUTABLE) finish-joystick-control /usr/bin