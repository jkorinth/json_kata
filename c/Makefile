.PHONY: clean mrproper format

clean:
	@echo "C: clean done."

mrproper: clean
	@echo "C: mrproper done."

format:
	@if ! type clang-format > /dev/null; then \
	  echo "C: clang-format is not installed, skipping format" >&2; \
	fi
	@-clang-format ./*.{c,h}

