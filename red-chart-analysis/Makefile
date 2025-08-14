CXX = g++
CXXFLAGS = -std=c++17 -Wall -DSDL_DISABLE_IMMINTRIN_H \
    -I./modules/imgui \
    -I./modules/imgui/backends \
    `pkg-config --cflags sdl3`

LDFLAGS = `pkg-config --libs sdl3` -lGL -ldl -lpthread

SRC = \
    handler.cpp \
    modules/imgui/imgui.cpp \
    modules/imgui/implot.cpp \
    modules/imgui/implot_items.cpp \
    modules/imgui/implot_demo.cpp \
    modules/imgui/imgui_draw.cpp \
    modules/imgui/imgui_widgets.cpp \
    modules/imgui/imgui_tables.cpp \
    modules/imgui/imgui_demo.cpp \
    modules/imgui/backends/imgui_impl_sdl3.cpp \
    modules/imgui/backends/imgui_impl_opengl3.cpp

all: gui

gui: gui.cpp $(SRC)
	$(CXX) $(CXXFLAGS) gui.cpp $(SRC) $(LDFLAGS) -o gui

clean:
	rm -f gui
