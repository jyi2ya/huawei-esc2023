debug: main.cpp
	g++ main.cpp -o debug -Wall -Wextra -Wshadow -Og -g -fsanitize=address -DLOCAL_LIGEN

test: debug
	./debug > result.txt
	perl validator.pl 0.in result.txt

release: main.cpp
	g++ main.cpp -o release -O2

bench: release
	hyperfine './release < 0.in'

.PHONY: clean
clean:
	rm -f main debug release *.exe
