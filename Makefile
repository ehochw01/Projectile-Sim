CXX = g++
CXXFLAGS = -std=c++20 -Iinclude -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -lraylib -framework OpenGL -framework Cocoa -framework IOKit

SRCDIR = src
SRCS = $(SRCDIR)/main.cpp $(SRCDIR)/PhysicsBody.cpp $(SRCDIR)/Projectile.cpp $(SRCDIR)/Cannon.cpp $(SRCDIR)/Target.cpp $(SRCDIR)/Debris.cpp
OBJS = $(SRCS:.cpp=.o)
HEADERS = $(wildcard include/*.h)
TARGET = sim

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# headers are listed as a prerequisite so editing a .h (e.g. a Constants.h value)
# forces every .o to recompile, not just the .cpp that happens to be newer
$(SRCDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

# Auto-rebuild and rerun the sim whenever a source or header changes.
# Requires entr: brew install entr
watch:
	@command -v entr >/dev/null 2>&1 || { echo "entr not found. Install it with: brew install entr"; exit 1; }
	@echo "Watching $(SRCDIR)/ and include/ — save a file to rebuild & rerun. Ctrl-C to quit."
	@ls $(SRCDIR)/*.cpp include/*.h | entr -r sh -c '$(MAKE) && ./$(TARGET)'

.PHONY: clean watch
