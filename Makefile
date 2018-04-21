all:
	mkdir output
	g++ -o CodeSmith main.cc parser.cc `pkg-config gtk+-3.0 gtksourceview-3.0 --cflags --libs gtk+-3.0 gtksourceview-3.0`
clean:
	rm -rf CodeSmith
	rm -rf output/kws-output.txt
	rm -rf output/clang-output.txt
