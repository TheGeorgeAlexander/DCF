CXX = clang++
FLAGS = -std=c++17 -Wall -Wextra -Wpedantic -Werror -O3 -Iinclude
TARGET = dcf-test




.PHONY: test
test:
	$(CXX) $(FLAGS) -o $(TARGET) test/test.cpp


.PHONY: dist
dist:
	python3 build.py


.PHONY: clean
clean:
	rm -rf dist
	rm -f $(TARGET)
