CC = g++
CC_FLAGS = -std=c++11
SOURCES_RUN = main.cpp mini_regexp.cpp
OUTPUT_RUN = mini_regexp
SOURCES_EXAMPLE = example.cpp mini_regexp.cpp
OUTPUT_EXAMPLE = regex_example

run:
	$(CC) $(CC_FLAGS) $(SOURCES_RUN) -o $(OUTPUT_RUN)
	./$(OUTPUT_RUN)

example:
	$(CC) $(CC_FLAGS) $(SOURCES_EXAMPLE) -o $(OUTPUT_EXAMPLE)
	./$(OUTPUT_EXAMPLE)

.PHONY: clean
clean:
	rm $(OUTPUT_RUN)
	rm $(OUTPUT_EXAMPLE)