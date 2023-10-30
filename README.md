# tsam5

Assignment 5: The Botnet Saves the World

## Group A5_6 (P3_GROUP_6)

- Kristófer Fannar Björnsson | kristoferb21@ru.is
- Logi Sigurðarson | logis21@ru.is

## Environment

OS -> MacOS 13 (ventura)
CPU -> M1

## Assignment

### 1. 

The client and server were implemented with all commands described in the assignment. We have separate ports for server and client, and decided to store connected servers and clients in sets.
To handle client and server commands, we implemented the functionality for those in the serverCommands.cpp and clientCommands.cpp files. 

The server log can be found within server.log, but we only really started using it on sunday. We also completed many bonus tasks, which are described below.

The code itself can be found within the tar file.


### 2. 

The wireshark trace of our client server communication can be found in client-server.pcapng
We took this to mean all the communication between server and client.

In addition to the necessary client commands, we also created the "CONNECT" command, which takes as arguments an ip address and a port, comma separated. That can also be seen in the wireshark trace.

### 3.

We were successfully connected to by Instructor servers multiple times throughout the week of working on the assignment. We could find multiple instances in the server.log file, but we're thinking you keep a record of this as well.

### 4.

We successfully received messages from around 16 servers in total (which we logged), which can be found in the log files. We used command line commands described in the bonus section to find out the exact amount.

### 5.

We successfully sent messages to 26 servers. This can be found with the command:

`grep -iEo -A 1 "SEND_MSG,P3_GROUP_[0-9]+,P3_GROUP_6" server.log | grep -B 1 "Successfully" | grep "P3_GROUP_6" | sort | uniq | wc -l`

The above command greps all lines in the log where our group sent a message, and then checks whether the line below it reads "Successfully". This was necessary because in three cases, the group wasn't found in our connections, and our backup plan was to then send the SEND_MSG command to the instructor servers, hoping that they would send the message. We can't know whether those messages were received, so this will have to do.

### Bonus

The max number of bonus points is 5, which we have obtained. They are described below.

#### Akureyri (0.5 pts)

Group 19 connected to us first on sunday, and again on monday. The group told us that they were from Akureyri, and the conversation can be found within the log files (server.log).

We stored the conversation in akureyri.log, with the command:

`grep -E "SENDMSG,P3_Group_19,P3_GROUP_6|P3_Group_19:" server.log > akureyri.log`


#### Groups connected to (1 pts / 5 groups)

We connected to various groups throughout the assignment's duration. As we didn't start logging until sunday, we can only use the data from that time. 

To find out how many groups we sent or received messages from, we used this script:

`grep -io "P3_GROUP_[0-9]\+:" server.log | sort | uniq | wc -l`

Note that adding the colon in the end of the grep string made sure that we only grepped lines where groups were sending messages, but not connecting.
This can be confirmed by removing the `-o` flag and skipping the last three commands (sort, uniq and wc).

In total, we send or received messages from 16 servers, which should account to 3 bonus points. The list of servers can be printed by omitting the `wc -l` command.
We are quite sure that P3_GROUP_699 is a duplicate of another server, likley group 69. With that discounted, we still have 15 server messages, and still reach the 3 bonus points.

#### Decode hash (2 pts)

In total, we received 6 number hashes from the number server. By running the hash through a MD5 decrypter, we found the secrets and logged them in hashed_msg.txt.

We can't be sure how many points these 6 number hashes result in, as it isn't mentioned.

#### Port forwarding (2 pts)

We opened up for port forwarding on Kristófer's router, ip 157.157.63.135, port 4006. 
This task was first started early on in the project, and this was the router which had the infamous password "password", from [this piazza post](https://piazza.com/class/llmkmqion5w282/post/321).

To port forward, Kristófer opened up port 4006 on the router to map to port 4006 on his mac, which was assigned the inner NAT address 10.0.0.3. 
To make sure this wouldn't change, Kristófer reserved the address 10.0.0.3 to his mac and locked it, so that the address would be reserved for his mac.

While debugging and getting the router port to open up to outside connections worked generally, which we tested using a simple node express server, we didn't manage to connect to the tcp server this way. We're not sure why it worked generally, but not for this server specifically, but we're hoping to get an answer on [this piazza thread](https://piazza.com/class/llmkmqion5w282/post/360).


## Running the code

Currently, our main implementation has a server in `server.cpp`, and a client in `runClient.cpp`.
You can use `make` to get both the server and client up and running.

To run the server with `make`: (preconfigured ports)
`make server`

To compile the server with `make`:
`make scompile`


To run the client with `make`: (preconfigured ports)
`make client`

To compile the client with `make`:
`make ccompile`

To dynamically find the ip that your server runs on:
`make findMyIp`
