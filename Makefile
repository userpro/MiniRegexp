CC = g++
CC_FLAGS = -std=c++11
LIB_SOURCES = mini_regexp.cpp re_parser.cpp re_lexer.cpp re_vm.cpp
SOURCES_RUN = main.cpp $(LIB_SOURCES)
OUTPUT_RUN = main_regexp.out
SOURCES_EXAMPLE = example.cpp $(LIB_SOURCES)
OUTPUT_EXAMPLE = regex_example.out

run:
	$(CC) $(CC_FLAGS) $(SOURCES_RUN) -o $(OUTPUT_RUN)
	./$(OUTPUT_RUN)

example:
	$(CC) $(CC_FLAGS) $(SOURCES_EXAMPLE) -o $(OUTPUT_EXAMPLE)
	./$(OUTPUT_EXAMPLE)

.PHONY: clean
clean:
	rm *.out
