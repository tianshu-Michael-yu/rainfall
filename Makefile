CXX := g++
CXXFLAGS := -O2 -std=c++11 -pthread

TARGET := hello_world fibonacci_sequence fibonacci_seq_fut fibonacci_task fibonacci_async rainfall_seq

all: $(TARGET)

rainfall_seq: rainfall_seq.o io_processing.o rainfall_simulation.o
	$(CXX) $(CXXFLAGS) $^ -o $@

rainfall_seq.o: rainfall_seq.cpp io_processing.h rainfall_simulation.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

io_processing.o: io_processing.cpp io_processing.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

rainfall_simulation.o: rainfall_simulation.cpp rainfall_simulation.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

hello_world: hello_world.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

fibonacci_sequence: fibonacci_sequence.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

fibonacci_seq_fut: fibonacci_seq_fut.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

fibonacci_task: fibonacci_task.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

fibonacci_async: fibonacci_async.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) *.o
