# tsam5

Assignment 5: The Botnet Saves the World

## Group A5_6 (P3_GROUP_6)

- Kristófer Fannar Björnsson | kristoferb21@ru.is
- Logi Sigurðarson | logis21@ru.is

## Environment

OS -> MacOS 13 (ventura)
CPU -> M1

## Running the code

Currently, our main implementation has a server in `selectServer.cpp`, and a client in `selectClient.cpp`.
You can use `make` to get both the server and client up and running.

To run the server with `make`:
`make selectServer`

To run the client with `make`:
`make selectClient`

Both will compile the code and run the executables with hardcoded program arguments. This is because `make` doesn't seem to have a nice way to pass program arguments.
