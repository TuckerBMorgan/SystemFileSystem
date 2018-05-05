CC = clang
CFLAGS = -g -Wall 
PROGRAM = file
OBJECTS = main.o read.o write.o directory.o globals.o
filesystem: $(OBJECTS) 
			$(CC) $(CFLAGS) -o $(PROGRAM) $(OBJECTS) 
clean: rm -f $(PROGRAM) $(OBJECTS)

main.o: read.h directory.h write.h