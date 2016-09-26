CXX = g++ -std=c++11 -O3
CXXFLAGS = -pg -Wall -Wextra

SRC = Solver.cpp
all:$(SRC)
	$(CXX) $(SRC) -o Solver

test:$(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o Solver_test
clean:
	rm -f Solver
	rm -f Solver_test
