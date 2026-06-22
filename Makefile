CXX = g++
CXXFLAGS = -std=c++20 -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit

SRCS = main.cpp PhysicsBody.cpp Projectile.cpp Cannon.cpp Target.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = sim

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean
