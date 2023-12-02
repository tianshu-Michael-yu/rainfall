CXX := g++
CXXFLAGS := -O3 -std=c++20 

TARGET := rainfall_seq rainfall_pt

all: $(TARGET)

rainfall_pt: rainfall_pt.o io_processing.o rainfall_simulation_pt.o
	$(CXX) $(CXXFLAGS) $^ -o $@

rainfall_seq: rainfall_seq.o io_processing.o rainfall_simulation.o
	$(CXX) $(CXXFLAGS) $^ -o $@

rainfall_pt.o: rainfall.cpp io_processing.h rainfall_simulation.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

rainfall_seq.o: rainfall.cpp io_processing.h rainfall_simulation.h
	$(CXX) -DSEQ $(CXXFLAGS) -c $< -o $@

io_processing.o: io_processing.cpp io_processing.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

rainfall_simulation.o: rainfall_simulation.cpp rainfall_simulation.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

rainfall_simulation_pt.o: rainfall_simulation_pt.cpp rainfall_simulation.h
	$(CXX) $(CXXFLAGS) -c $< -o $@


.PHONY: clean
clean:
	rm -f $(TARGET) *.o
