OPENSSL_FLAGS = $(shell pkg-config --libs --cflags openssl)
LIBJWT_FLAGS = $(shell pkg-config --libs --cflags libjwt)
JANSSON_FLAGS = $(shell pkg-config --libs --cflags jansson)
SRC = example.c \
	lib/http.c \
	lib/token.c \
	lib/utils.c

all: build run

build:
	gcc $(SRC) -o bin/example $(OPENSSL_FLAGS) $(LIBJWT_FLAGS) $(JANSSON_FLAGS)

build.debug:
	gcc $(SRC) -g -o bin/example $(OPENSSL_FLAGS) $(LIBJWT_FLAGS) $(JANSSON_FLAGS)

run:
	./bin/example

docker: docker.build docker.run

docker.build:
	docker build -t valgrind .

docker.run:
	docker run --rm -it valgrind make run

docker.run.valgrind:
	docker run --rm -it valgrind valgrind --track-origins=yes --leak-check=full /githubapp-token-refresh/bin/example