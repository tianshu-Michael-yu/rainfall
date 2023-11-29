CXX := g++
CXXFLAGS := -O2 -std=c++11 -pthread

TARGET := rainfall_seq

all: $(TARGET)

rainfall_seq: rainfall_seq.o io_processing.o rainfall_simulation.o
	$(CXX) $(CXXFLAGS) $^ -o $@

rainfall_seq.o: rainfall_seq.cpp io_processing.h rainfall_simulation.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

io_processing.o: io_processing.cpp io_processing.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

rainfall_simulation.o: rainfall_simulation.cpp rainfall_simulation.h
	$(CXX) $(CXXFLAGS) -c $< -o $@


.PHONY: clean
clean:
	rm -f $(TARGET) *.o
