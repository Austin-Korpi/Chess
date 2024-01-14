#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# You will need GLFW (http://www.glfw.org):
# Linux:
#   apt-get install libglfw-dev
# Mac OS X:
#   brew install glfw
# MSYS2:
#   pacman -S --noconfirm --needed mingw-w64-x86_64-toolchain mingw-w64-x86_64-glfw
#
# Also needed:
#	https://github.com/oneapi-src/oneTBB/blob/master/INSTALL.md

#CXX = g++
#CXX = clang++

ENGINE = engine.app
GUI = chess.app
HEADLESS = chess.script
IMGUI_DIR = imgui
SRC_DIR = src
INCLUDE_DIR = inc
LIB_DIR = lib
SOURCES =  $(SRC_DIR)/Game.cpp $(SRC_DIR)/Pieces.cpp $(SRC_DIR)/Opening.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
GUI_SOURCES = $(SOURCES) $(SRC_DIR)/main.cpp
HEADLESS_SOURCES = $(SRC_DIR)/Game.cpp $(SRC_DIR)/Pieces.cpp $(SRC_DIR)/Headless.cpp
ENGINE_SOURCES = $(SRC_DIR)/Game.cpp $(SRC_DIR)/Pieces.cpp $(SRC_DIR)/Engine.cpp $(SRC_DIR)/Opening.cpp
GUI_OBJS = $(addprefix bin/, $(addsuffix .o, $(basename $(notdir $(GUI_SOURCES)))))
HEADLESS_OBJS = $(addprefix bin/, $(addsuffix .o, $(basename $(notdir $(HEADLESS_SOURCES)))))
ENGINE_OBJS = $(addprefix bin/, $(addsuffix .o, $(basename $(notdir $(ENGINE_SOURCES)))))
UNAME_S := $(shell uname -s)
LINUX_GL_LIBS = -lGL

CXXFLAGS = -std=c++11 -I$(IMGUI_DIR) -I$(INCLUDE_DIR) -I$(LIB_DIR)
CXXFLAGS += -g -O3 -Wall -Wformat #-pg
LIBS = -pthread -ltbb #-pg

##---------------------------------------------------------------------
## OPENGL ES
##---------------------------------------------------------------------

## This assumes a GL ES library available in the system, e.g. libGLESv2.so
# CXXFLAGS += -DIMGUI_IMPL_OPENGL_ES2
# LINUX_GL_LIBS = -lGLESv2

##---------------------------------------------------------------------
## BUILD FLAGS PER PLATFORM
##---------------------------------------------------------------------

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS += $(LINUX_GL_LIBS) `pkg-config --static --libs glfw3`

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
	LIBS += -L/usr/local/lib -L/opt/local/lib -L/opt/homebrew/lib
	#LIBS += -lglfw3
	LIBS += -lglfw

	CXXFLAGS += -I/usr/local/include -I/opt/local/include -I/opt/homebrew/include
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(OS), Windows_NT)
	ECHO_MESSAGE = "MinGW"
	LIBS += -lglfw3 -lgdi32 -lopengl32 -limm32

	CXXFLAGS += `pkg-config --cflags glfw3`
	CFLAGS = $(CXXFLAGS)
endif

##---------------------------------------------------------------------
## BUILD RULES
##---------------------------------------------------------------------

bin/%.o:$(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

bin/%.o:$(IMGUI_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

bin/%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

all: $(GUI)
	@echo Build complete for $(ECHO_MESSAGE)

$(GUI): $(GUI_OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

$(HEADLESS): $(HEADLESS_OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

$(ENGINE): $(ENGINE_OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(GUI) $(HEADLESS_OBJS) $(GUI_OBJS)

headless: $(HEADLESS)

engine: $(ENGINE)
