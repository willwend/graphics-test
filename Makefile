all:
	g++ test.cpp -o test -I include -L lib -l SDL2-2.0.0 -l GLEW.2.1.0
