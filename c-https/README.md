github-token-refresh in C
=========================

## Docker

Since C libraries are little more platform specific than python and ruby, this subfolder contains a `Dockerfile` you can use to run this example in a containerized environment (Ubuntu 18.04 in this case).

Run `make docker` to build the image and run the example in a container. Logs will print to stdout.

## MacOS

On MacOS with brew, you can install the dependencies with `brew install libjwt jansson openssl pkg-config`.

With these installed, you should be able to run:
```sh
$ make build # compile the binary
$ make run # run the binary
```

There are example binaries committed (`example.linux` and `example.macos`). The binary you'll produce when running `make build` will create the binary `bin/example`. Just run `make run` to avoid this confusion.

## Leaks
Valgrind reports approx 900B leaking but I didn't see it go over that number running for well over an hour.