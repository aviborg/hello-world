# What to call the final executable
TARGET = helloWorld.out

OBJS= helloWorldRest.o

# What compiler to use
CC = g++

CFLAGS = -c -Wall

LDFLAGS = -L/home/openhabian/jsmn -I/home/openhabian/jsmn

# We need -lcurl for the curl stuff
LIBS = -lcurl -ljsmn

# Link the target with all objects and libraries
$(TARGET) : helloWorldRest.o
	$(CC)  -o $(TARGET) $(OBJS) $(LDFLAGS) $(LIBS)
		
$(OBJS) : helloWorldRest.c
	$(CC) $(CFLAGS) $<