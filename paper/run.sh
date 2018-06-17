#!/bin/bash

rm senior-project.pdf

cd tex
pdflatex main.tex
pdflatex main.tex

mv main.pdf ../senior-project.pdf

cd ..
wsl-open senior-project.pdf
