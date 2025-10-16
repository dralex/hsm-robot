HAL_DIR := hal
HAL_SOURCES := $(wildcard $(addsuffix /*.cpp, $(HAL_DIR)))
HAL_OBJ := $(patsubst %.cpp, %.o, $(HAL_SOURCES))

TEST_OBJ = test.o

test: $(TEST_OBJ) $(HAL_OBJ)
	g++ $(HAL_OBJ) $(TEST_OBJ) -o test

%.o: %.cpp
	g++ -c -g3 -DDEBUG -pedantic -Wall -I$(HAL_DIR) $< -o $@

%.o: %.c
	gcc -c -g3 -DDEBUG -pedantic -Wall -I$(HAL_DIR) $< -o $@
