CC = g++ -std=c++11 -O3# compiler collection
OBJECTS = test1.o hw2_test.o
EXECUTABLE = test.out
DEBUG_FLAGS =# if you need to debug just add -g
COMPILATION_FLAGS = -Wall -Werror -Wconversion -Wextra -DNDEBUG
BUILD_O_FILE = $(CC) $(DEBUG_FLAGS) $(COMPILATION_FLAGS) -c
# macro for compiling cxx file respectively to the name of the o file requested
BUILD_O_FILE_RESPECTIVELY = $(BUILD_O_FILE) $*.cxx -o $@

 # Linking all the object files into one executable
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(DEBUG_FLAGS) -pthread -o $@ $(OBJECTS)
	
test1.o: test1.cxx
	$(BUILD_O_FILE_RESPECTIVELY)

hw2_test.o: hw2_test.cxx
	$(BUILD_O_FILE_RESPECTIVELY)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)
	
