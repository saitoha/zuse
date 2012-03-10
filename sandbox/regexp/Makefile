
CFLAGS=-g -O0 -I../../src
LDFLAGS= -ldl
PROGRAM=regexp

$(PROGRAM): $(PROGRAM).o
	$(CXX) $< $(LDFLAGS) -o $@

.cpp.o:
	$(CXX) -c $(CFLAGS) $< -o $@

clean:
	$(RM) *.o $(PROGRAM)

run: $(PROGRAM)
	./$<


