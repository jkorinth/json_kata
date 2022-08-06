.PHONY: all clean mrproper

all: nlohmannjson

clean:
	@rm -f *.o

mrproper:
	@rm -f nlohmannjson

nlohmannjson: nlohmannjson.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
