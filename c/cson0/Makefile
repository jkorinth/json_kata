.PHONY: all clean mrproper

CFLAGS += -fPIE

all: cson0

clean:
	@rm -f *.o

mrproper:
	@rm -f cson0

cson0: cson0.o cson0_parser.o cson0_trace.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
