# Makefile for CS455 Project 1

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
INCLUDES = $(shell pkg-config --cflags opencv4)
LIBS = $(shell pkg-config --libs opencv4)

# Target executables
TARGET = detection
IMAGE_TARGET = image
EQUALIZATION_TARGET = equalization
HISTOGRAM_TARGET = histogram

# Source files
SOURCES = detection.cpp
IMAGE_SOURCES = image.cpp
EQUALIZATION_SOURCES = equalization.cpp
HISTOGRAM_SOURCES = histogram.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)
IMAGE_OBJECTS = $(IMAGE_SOURCES:.cpp=.o)
EQUALIZATION_OBJECTS = $(EQUALIZATION_SOURCES:.cpp=.o)
HISTOGRAM_OBJECTS = $(HISTOGRAM_SOURCES:.cpp=.o)

# Default target
all: $(TARGET) $(IMAGE_TARGET) $(EQUALIZATION_TARGET) $(HISTOGRAM_TARGET)

# Build executables
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LIBS)

$(IMAGE_TARGET): $(IMAGE_OBJECTS)
	$(CXX) $(IMAGE_OBJECTS) -o $(IMAGE_TARGET) $(LIBS)

$(EQUALIZATION_TARGET): $(EQUALIZATION_OBJECTS)
	$(CXX) $(EQUALIZATION_OBJECTS) -o $(EQUALIZATION_TARGET) $(LIBS)

$(HISTOGRAM_TARGET): $(HISTOGRAM_OBJECTS)
	$(CXX) $(HISTOGRAM_OBJECTS) -o $(HISTOGRAM_TARGET) $(LIBS)

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Clean only generated files
clean:
	rm -f $(OBJECTS) $(IMAGE_OBJECTS) $(EQUALIZATION_OBJECTS) $(HISTOGRAM_OBJECTS)
	rm -f $(TARGET) $(IMAGE_TARGET) $(EQUALIZATION_TARGET) $(HISTOGRAM_TARGET)
	rm -f *_f1_binary.bmp *_f2_labeled.bmp *_histogram.bmp
	rm -f *_original.bmp *_negative.bmp *_equalized_original.bmp *_equalized_negative.bmp
	rm -f *_hist_original.bmp *_hist_negative.bmp *_hist_equalized_original.bmp *_hist_equalized_negative.bmp

# Run programs
run: $(TARGET)
	./$(TARGET)

run-image: $(IMAGE_TARGET)
	./$(IMAGE_TARGET)

run-equalization: $(EQUALIZATION_TARGET)
	./$(EQUALIZATION_TARGET)

run-histogram: $(HISTOGRAM_TARGET)
	./$(HISTOGRAM_TARGET)

# Phony targets
.PHONY: all clean run run-image run-equalization run-histogram
