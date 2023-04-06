source "/home/x/dust-0/emsdk/emsdk_env.sh"
emcc \
	models_first_person_maze.c \
	-o build_web/index.html \
	--shell-file index.html \
	--preload-file resources \
	-Os -Wall -DPLATFORM_WEB \
	../raylib/src/libraylib.a \
	-I. -I../raylib/src \
	-L. -L..../raylib/src \
	-s USE_GLFW=3 -s ASYNCIFY
