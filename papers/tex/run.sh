#/bin/bash

pdflatex main.tex
pdflatex main.tex

mv main.pdf ../senior-project.pdf

wsl-open ../senior-project.pdf
