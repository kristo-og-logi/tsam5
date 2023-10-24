### Server

Server should listen on one TCP port for servers, and another for clients.

#### Server Commands

- QUERYSERVERS,<GROUP_ID> (Responds with SERVERS)
- SERVERS
- KEEPALIVE,<No. of msgs>
- FETCH_MSGS,<GROUP_ID>
- SEND_MSG,<TO_GROUP_ID>,<FROM_GROUP_ID>,<MESSAGES>
- STATUSREQ,<FROM_GROUP_ID>
- STATUSRESP,<FROM_GROUP_ID>,<TO_GROUP_ID>, <SERVER, msgs held>

#### Code structure

##### Server

- handleServer: handles connected servers, encapsulated server lated things

### Bugs

- send() missing first (0x02) and last (0x03) byte, fix make custom function.
