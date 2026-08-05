LATEXMK ?= latexmk
SOURCE := guia-instalacion.tex
PDF := guia-instalacion.pdf

.PHONY: all clean

all: $(PDF)

$(PDF): $(SOURCE)
	$(LATEXMK) -pdf -interaction=nonstopmode -halt-on-error $(SOURCE)

clean:
	$(LATEXMK) -C $(SOURCE)
