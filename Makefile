INST = /bin
CONFDIR = /etc/dlift

all: dlift

dlift: 	dlift.o
		gcc -o dlift dlift.o

dlift.o: 	dlift.c
		gcc -o dlift.o -c dlift.c

clean:		
		rm -rf *.o
install: 
		install -D -m 777 dlift $(INST)
		install -D -m 777 dlift.o $(INST)
		if test ! -d $(CONFDIR) ; then mkdir -p $(CONFDIR) ; fi
