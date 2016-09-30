CXX = g++ -std=c++11
CXXFLAGS = -pg -Wall -Wextra

SRC = Solver.cpp
all:$(SRC)
	$(CXX) -O3 $(SRC) -o Solver

test:$(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o Solver_test
codingame:$(SRC)
	$(CXX) $(SRC) -o Solver_codingame
clean:
	rm -f Solver
	rm -f Solver_test
