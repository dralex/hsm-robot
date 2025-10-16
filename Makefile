test: main.o network.o telemetry.o commands.o robot.o qhsm.o
	g++ main.o network.o telemetry.o commands.o robot.o qhsm.o -o test

%.o: %.cpp
	g++ -c -g3 -DDEBUG -pedantic -Wall $< -o $@

%.o: %.c
	gcc -c -g3 -DDEBUG -pedantic -Wall $< -o $@
