CC = g++
CC_FLAGS = -std=c++11
LIB_SOURCES = mini_regexp.cpp mini_regexp_parser.cpp mini_regexp_lexer.cpp mini_regexp_vm.cpp
SOURCES_RUN = main.cpp $(LIB_SOURCES)
OUTPUT_RUN = mini_regexp
SOURCES_EXAMPLE = example.cpp $(LIB_SOURCES)
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
