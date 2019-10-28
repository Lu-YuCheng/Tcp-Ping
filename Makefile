all:
	gcc -o server server.c
	gcc -o client client.c
	gcc -o server_pthread -lpthread server_pthread.c
clean:
	rm client server server_pthread
