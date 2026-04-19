CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -Iinclude

SRC = src/main.c src/debate.c src/provider_ollama.c src/http_client.c

OUT = debate_cli

all:
	$(CC) $(CFLAGS) -o $(OUT) $(SRC) -lcurl

clean:
	rm -f $(OUT)