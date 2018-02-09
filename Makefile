all: src/server.c src/client.c
	@mkdir -p u_files downloads db
	@gcc -o server src/server.c -lpthread
	@gcc -o client src/client.c
	
clean: 
	@$(RM) server
	@$(RM) client