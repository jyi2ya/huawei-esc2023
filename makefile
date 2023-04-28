debug: main.cpp
	g++ main.cpp -o debug -Wall -Wextra -Wshadow -Og -g -fsanitize=address -DLOCAL_LIGEN

test: debug
	./debug

release: main.cpp
	g++ main.cpp -o release -O2

bench: release
	hyperfine './release < 0.in'

.PHONY: clean
clean:
	rm -f main debug release *.exe
