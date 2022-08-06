.PHONY: all clean mrproper

CXX_STYLE ::= Mozilla
CXX ?= g++
CXXFLAGS += -std=gnu++11 -O4 -DNDEBUG
FMT=clang-format -i --style=$(CXX_STYLE)

CANDIDATES ::= nlohmannjson
SRCS ::= $(shell find . -name 3rdparty -prune -o \( -type f -name '*.cc' -o -name '*.cpp' -o -name '*.cxx' -o -name '*.c++' \) -print)
HDRS ::= $(shell find . -name 3rdparty -prune -o \( -type f -name '*.h' -o -name '*.hpp' -o -name '*.hxx' -o -name '*.h++' \) -print)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

all:
	@for d in $(CANDIDATES); do \
		CXXFLAGS="$(CXXFLAGS)" LDFLAGS="$(LDFLAGS)" $(MAKE) --no-print-directory -C $$d; \
	done

clean:
	@for d in $(CANDIDATES); do \
		CXXFLAGS="$(CXXFLAGS)" LDFLAGS="$(LDFLAGS)" $(MAKE) --no-print-directory -C $$d clean; \
	done
	@echo "C++: clean done."

mrproper: clean
	@for d in $(CANDIDATES); do \
		CXXFLAGS="$(CXXFLAGS)" LDFLAGS="$(LDFLAGS)" $(MAKE) --no-print-directory -C $$d mrproper; \
	done
	@echo "C++: mrproper done."

format:
	@if ! type clang-format > /dev/null; then \
		echo "C++: clang-format is not installed, skipping format" >&2; \
	fi
	@echo HDRS=$(HDRS)
	@echo SRCS=$(SRCS)
	@-$(FMT) $(SRCS) $(HDRS)

