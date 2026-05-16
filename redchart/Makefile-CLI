CXX = g++


CXXFLAGS = -Wall -std=c++17 -I/usr/include/python3.13
LDFLAGS = -lpython3.13


TARGET = build/redchart


SRCS = main.cpp handler.cpp


OBJS = $(SRCS:.cpp=.o)


all: $(TARGET)


$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)