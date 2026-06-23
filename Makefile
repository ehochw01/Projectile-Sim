CXX = g++
CXXFLAGS = -std=c++20 -Iinclude -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit

SRCDIR = src
SRCS = $(SRCDIR)/main.cpp $(SRCDIR)/PhysicsBody.cpp $(SRCDIR)/Projectile.cpp $(SRCDIR)/Cannon.cpp $(SRCDIR)/Target.cpp $(SRCDIR)/Debris.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = sim

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: clean
