s:s.o
	gcc -o s s.o `mysql_config --cflags --libs`
s.o:server.c
	gcc -c -o s.o server.c `mysql_config --cflags --libs`
c:c.o
	gcc -o c c.o `mysql_config --cflags --libs`
c.o:client.c
	gcc -c -o c.o client.c `mysql_config --cflags --libs`
c_c:c_c.o
	gcc -o c_c c_c.o `mysql_config --cflags --libs`
c_c.o:client_c.c
	gcc -c -o c_c.o client_c.c `mysql_config --cflags --libs`

server:server.c
	gcc server.c -o server `mysql_config --cflags --libs`
client:client.c
	gcc client.c -o client `mysql_config --cflags --libs`
client_c:client_c.c
	gcc client_c.c -o client_c `mysql_config --cflags --libs`

clean:
	rm *.o s c

