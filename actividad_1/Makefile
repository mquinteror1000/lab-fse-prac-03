CC = gcc
CFLAGS = -Wall -Wextra -O2 -I ../drivers/lm75b
TARGET = logger_lm75b
SOURCES = logger_lm75b.c ../drivers/lm75b/lm75b.c
.PHONY: all clean
all: $(TARGET)
$(TARGET) : $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)
clean :
	rm -f $(TARGET)
