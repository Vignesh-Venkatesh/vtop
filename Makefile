CXX=g++
EXEC=vtop

all: run

run:
	$(CXX) ./main.cpp -o ./$(EXEC) && ./$(EXEC)

clean:
	rm -rf ./$(EXEC)
