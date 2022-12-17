main:
	g++ -std=c++17 main.cpp -o main -I include -L lib -l SDL2-2.0.0

clean:
	rm -rf main