debug: main.cpp
	g++ main.cpp -o debug -Wall -Wextra -Wshadow -Og -g -fsanitize=address -DLOCAL_LIGEN

test: debug
	./debug

release: main.cpp
	g++ main.cpp -o release -O2

.PHONY: clean
clean:
	rm -f main debug release *.exe
