CXX = g++

CXXFLAGS = -Wall -std=c++17


TARGET = redchart


SRCS = main.cpp handler.cpp


OBJS = $(SRCS:.cpp=.o)


all: $(TARGET)


$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)