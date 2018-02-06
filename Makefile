CC = g++
CC_FLAGS = -std=c++11
SOURCES = main.cpp mini_regex.cpp
OUTPUT = mini_regexp

run: $(OUTPUT)
	./$(OUTPUT)

$(OUTPUT):
	$(CC) $(CC_FLAGS) $(SOURCES) -o $(OUTPUT)

.PHONY: clean
clean:
	rm $(OUTPUT)