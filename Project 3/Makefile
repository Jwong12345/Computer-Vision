CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2
TARGET = solution
SOURCES = solution.cpp

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET) *.bmp

run: $(TARGET)
	./$(TARGET)

.PHONY: clean run
