# Declaration of variables
CC = g++
CC_FLAGS = -std=c++11

# File names
EXEC = DB_test.exe
SOURCES = $(wildcard *.cpp) \
          $(wildcard fileio/*.cpp) \
          $(wildcard bufmanager/*.cpp) \
          $(wildcard utils/*.cpp) \
          $(wildcard yacc_lex/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

# Main target
$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC)

# To obtain object files
%.o: %.cpp
	$(CC) -c $(CC_FLAGS) $< -o $@

# To remove generated files
clean:
	rm -f $(EXEC) $(OBJECTS)
