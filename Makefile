CXX = g++
CXXFLAGS = -lpthread

TARGETS = transpose transpose_csv transpose_malloc

all: $(TARGETS)

transpose: transpose.cc
	$(CXX) $(CXXFLAGS) -o $@ $<

transpose_csv: transpose_csv.cc
	$(CXX) $(CXXFLAGS) -o $@ $<

transpose_malloc: transpose_malloc.cc
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)

.PHONY: all clean
