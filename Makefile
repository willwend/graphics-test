all:
	g++ test.cpp --std=c++11 -o test -I include -L lib -l SDL2-2.0.0 -l GLEW.2.1.0 -framework OpenGL -Wno-deprecated
