main: main.cpp
	g++ main.cpp ./glad/src/glad.c ./include/tiny_obj_loader.h ./include/stb_image.h -o t.elf -std=c++11 -Wall -I./glad/include -lGL -lGLU -lGLEW -lglfw -ldl -lfreetype

cube: cube.cpp
	g++ cube.cpp ./glad/src/glad.c ./include/stb_image.h  -o cube.elf -std=c++11 -Wall -I./glad/include -lGL -lGLU -lGLEW -lglfw -ldl

ft: ft.cpp
	g++ ft.cpp ./glad/src/glad.c -o ft.elf -std=c++11 -Wall -I./glad/include -lGL -lGLU -lGLEW -lglfw -ldl -lfreetype

tex: mainTex.cpp
	g++ mainTex.cpp ./glad/src/glad.c ./include/tiny_obj_loader.h ./include/stb_image.h -o mtex.elf -std=c++11 -Wall -I./glad/include -lGL -lGLU -lGLEW -lglfw -ldl

