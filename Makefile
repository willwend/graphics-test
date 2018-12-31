test: test.cpp
	g++ test.cpp SOIL/lib/libSOIL.a --std=c++11 -o test -I include -L lib -l SDL2-2.0.0 -l GLEW.2.1.0 -framework OpenGL -framework CoreFoundation -Wno-deprecated

mandelbrot: mandelbrot.cpp
	g++ mandelbrot.cpp --std=c++11 -o mandelbrot -I include -L lib -l SDL2-2.0.0 -l GLEW.2.1.0 -framework OpenGL -framework CoreFoundation -Wno-deprecated
