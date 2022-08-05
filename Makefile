.PHONY: all setup clean mrproper sanity-checks format

UNAME = ${shell uname}

ifeq ($(UNAME),)
$(error Unknown OS "$(UNAME)", currently only *nixes are supported.)
endif

LANGUAGES ::= c cpp

clean:
	@find -name '__pycache__' -type d | xargs rm -rf
	@find -name '*.pyc' -type f -delete
	@for d in $(LANGUAGES); do $(MAKE) --no-print-directory -C $$d clean; done

mrproper: clean
	@rm -rf .venv
	@for d in $(LANGUAGES); do $(MAKE) --no-print-directory -C $$d mrproper; done

format: setup
	. .venv/bin/activate && \
	black jury
	@for d in $(LANGUAGES); do $(MAKE) --no-print-directory -C $$d format; done

setup:
	@python3 -m venv .venv && \
	. .venv/bin/activate && \
	python -m pip install -U wheel pip > /dev/null && \
	pip install -r requirements.txt > /dev/null && \
	pip install pytest > /dev/null

sanity-checks: setup
	. .venv/bin/activate && \
	pytest --hypothesis-seed=42 --hypothesis-show-statistics jury/jsongen.py
