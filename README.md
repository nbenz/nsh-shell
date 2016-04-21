#nsh-shell  

A simple shell in C.  

##Usage  

* Standard UNIX commands  
* Builtins  
* Chainable piping  
* Network access  

####Builtin Commands  

* cd   
* exit  

##Remote Access  

###Server  
`make server` -- compile server  
`server` -- starts server with defaults in defs.h    

####Options    
`-p` -- specify the port on which the server should listen    

###Client  
`make client` -- compile client  
`client` -- run client with defaults in defs.h  
`exit` -- (while client is running) end client/server session  

####Options  
`-h` -- specify the host to which the client should connect  
`-p` -- specify the port on which the client should connect  
