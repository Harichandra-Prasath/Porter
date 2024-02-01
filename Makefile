run: build
	@./bin/Porterd /home/hcp_0/Downloads

build:
	@gcc -o ./bin/Porterd Porter.c