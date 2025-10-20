TARGET = berloga

HAL_DIR := hal
GEN_DIR := generator
GRAPH_DIR := graph
GEN_SCRIPT := gencode.py

HAL_HEADERS := $(wildcard $(addsuffix /*.h, $(HAL_DIR)))
HAL_SOURCES := $(wildcard $(addsuffix /*.cpp, $(HAL_DIR)))
HAL_OBJ := $(patsubst %.cpp, %.o, $(HAL_SOURCES))
GEN_HEADERS := $(wildcard $(addsuffix /*.h, $(GEN_DIR)))
GEN_SOURCES := $(wildcard $(addsuffix /*.cpp, $(GEN_DIR)))
GEN_OBJ := $(patsubst %.cpp, %.o, $(GEN_SOURCES)) $(GEN_DIR)/qhsm.o

$(TARGET): $(GEN_OBJ) $(HAL_OBJ)
	g++ $(HAL_OBJ) $(GEN_OBJ) -lm -o $(TARGET)

TEST_OBJ = test.o
test: $(TEST_OBJ) $(HAL_OBJ)
	g++ $(HAL_OBJ) $(TEST_OBJ) -o test

%.o: %.cpp
	g++ -c -g3 -D__DEBUG__ -pedantic -Wall -I$(HAL_DIR) -I$(GEN_DIR) $< -o $@
%.o: %.c
	gcc -c -g3 -D__DEBUG__ -pedantic -Wall -I$(HAL_DIR) -I$(GEN_DIR) $< -o $@

install:
	cp -f $(TARGET) /usr/local/bin/

rebuild:
	cd $(GEN_DIR) && python3 $(GEN_SCRIPT) ../$(GRAPH_DIR)/$(TARGET).graphml && cd ..

clean:
	rm -f *.o *~ $(HAL_DIR)/*.o $(GEN_DIR)/*.o

.PHOHY: $(TARGET) test install clean rebuild
