exec = test.out
sources = $(wildcard src/*.c)
objects = $(sources:.c=.o)
flags = -I./../src/include/ -I./external/libjson/src/include -g -Wall -lm -ldl -fPIC -rdynamic -L./ -lfjb -ljson


$(exec): $(objects) libfjb.a
	gcc $(objects) $(flags) -o $(exec)

libfjb.a: libjson.a
	cd .. && make libfjb.a && mv libfjb.a tests/.
	ar rcs $@ $^

libjson.a:
	cd ../external/libjson ; make ; mv ./libjson.a ../../tests/.

%.o: %.c include/%.h
	gcc -c $(flags) $< -o $@

clean:
	-rm *.out
	-rm *.o
	-rm *.a
	-rm src/*.o

lint:
	clang-tidy src/*.c src/include/*.h
