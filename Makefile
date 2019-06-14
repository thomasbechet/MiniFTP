CC=gcc
CFLAGS=-W -Wall -ansi -pedantic
SRC=src

all: sftp_srv sftp_clt

sftp_srv: sftp_srv.o tea.o tcp_connection.o diffie_hellman.o request.o request_put.o request_get.o request_dir.o file_transfer.o utility.o
	$(CC) $(CFLAGS) -o sftp_srv sftp_srv.o tea.o tcp_connection.o diffie_hellman.o request.o request_put.o request_dir.o request_get.o file_transfer.o utility.o
sftp_clt: sftp_clt.o tea.o tcp_connection.o diffie_hellman.o request.o request_put.o request_get.o request_dir.o file_transfer.o utility.o
	$(CC) $(CFLAGS) -o sftp_clt sftp_clt.o tea.o tcp_connection.o diffie_hellman.o request.o request_put.o request_get.o request_dir.o file_transfer.o utility.o


sftp_srv.o: $(SRC)/sftp_srv.c $(SRC)/tea.h $(SRC)/tcp_connection.h
	gcc -c $(SRC)/sftp_srv.c
sftp_clt.o: $(SRC)/sftp_clt.c $(SRC)/tea.h $(SRC)/tcp_connection.h
	gcc -c $(SRC)/sftp_clt.c

tea.o: $(SRC)/tea.c $(SRC)/tea.h
	gcc -c $(SRC)/tea.c

tcp_connection.o: $(SRC)/tcp_connection.c $(SRC)/tcp_connection.h
	gcc -c $(SRC)/tcp_connection.c

diffie_hellman.o: $(SRC)/diffie_hellman.c $(SRC)/diffie_hellman.h
	gcc -c $(SRC)/diffie_hellman.c

request.o: $(SRC)/request.c $(SRC)/request.h
	gcc -c $(SRC)/request.c

request_put.o: $(SRC)/request_put.c $(SRC)/request.h
	gcc -c $(SRC)/request_put.c

request_get.o: $(SRC)/request_get.c $(SRC)/request.h
	gcc -c $(SRC)/request_get.c

request_dir.o: $(SRC)/request_dir.c $(SRC)/request.h
	gcc -c $(SRC)/request_dir.c

file_transfer.o: $(SRC)/file_transfer.c $(SRC)/file_transfer.h
	gcc -c $(SRC)/file_transfer.c

utility.o: $(SRC)/utility.c $(SRC)/utility.h
	gcc -c $(SRC)/utility.c

.PHONY: clean

clean:
	rm -f *.o