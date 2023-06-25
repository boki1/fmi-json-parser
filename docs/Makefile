OUT=out/
INPUT=doc.tex

all:
	mkdir -p $(OUT)
	pdflatex -output-directory=$(OUT) doc.tex
	biber --output-directory=$(OUT) doc
	pdflatex -output-directory=$(OUT) doc.tex
	ln -sf $(OUT)/doc.pdf ./doc.pdf

clean:
	rm -rf $(OUT) doc.pdf

.PHONY: all clean
