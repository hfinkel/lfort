GXX := llvm-g++-4.2
LFORTXX := lfort++

all: one

test.cc: gen.cc
	g++ gen.cc -o gen
	./gen >test.cc

test-gcc.sum: test.cc
	time $(GXX) test.cc -o test-gcc.s -S
	$(GXX) test-gcc.s -o test-gcc
	./test-gcc >test-gcc.sum

test-lfort.sum: test.cc
	time $(LFORTXX) test.cc -o test-lfort.s -S
	$(LFORTXX) test-lfort.s -o test-lfort
	./test-lfort >test-lfort.sum

one: test-gcc.sum test-lfort.sum
	cmp test-gcc.sum test-lfort.sum

clean:
	rm -f gen test-gcc test-lfort test.cc test-gcc.sum test-lfort.sum test-gcc.s test-lfort.s
