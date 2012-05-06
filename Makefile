CFLAGS = -Wformat-nonliteral -ggdb -O0 -Wall -Werror `pkg-config --cflags gtk+-3.0` -ggdb -O0
LDFLAGS = -pthread `pkg-config --libs gtk+-3.0`
OBJECTS = $(SOURCES:.c=.o)
BUILD = gsfm
SOURCES = main.c \
		  view.c \
		  chrome.c

# use custom c compiler, if defined
ifndef CC
CC = clang
endif

all: $(SOURCES) $(BUILD)

$(BUILD): $(OBJECTS)
	@echo "LD" $@
	@$(CC) $(LDFLAGS) $(OBJECTS) -o $@
.c.o:
	@echo "CC" $<
	@$(CC) $(CFLAGS) -c $< -o $@

install:
	cp $(BUILD) /usr/bin/

clean:
	rm -rf *.o
	rm -rf $(BUILD)
