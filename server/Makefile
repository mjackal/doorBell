cc = gcc
srv : srv.o login.o
	$(cc) -o srv srv.o login.o
srv.o : srv.c
	$(cc) -c srv.c
login.o : login.c
	$(cc) -c login.c

clean:
	rm *.o 
