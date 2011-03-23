### $Id: Makefile,v 1.1.1.1 2000/06/08 00:10:01 afikes Exp $

DOCXX = doc++

TARGET = osuif

all:	html

.PHONY: html
html:
	$(DOCXX) -nd -p -c -d html $(TARGET).dxx

tex:
	$(DOCXX) -v -t -o $(TARGET).tex -ep a4wide $(TARGET).dxx
	latex $(TARGET)
	latex $(TARGET)

ps:	tex
	dvips -o $(TARGET).ps $(TARGET).dvi

clean:
	rm -rf html/ *.tex *.ps *.dvi *.gif *.class gifs.db
