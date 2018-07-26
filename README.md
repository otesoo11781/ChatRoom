# NCTU_NP_HW1
NCTU NP homework1 , a simple chat room.

### SERVER
When executing the server , the first argument is a PORT of this application.

### CLIENT
When executing the client , the first and second argument is the IP ADDRESS and PORT of the SERVER , respectively.
#### COMMAND
##### who
>Indicate NAME , IP ADDRESS and PORT of users online

##### name <NEW USERNAME>
>Change the username
	
>rules:
>1. not anonymous
>2. unique
>3. 2~12 Englist letters
	
##### tell <USERNAME> <MESSAGE>
>Send private message to another user
	
>rules:
>1. anonymous cannot send the receive message
>2. the receiver must exist
	
##### yell <MESSAGE>
>broadcast message to other users
	
##### exit
>close the client
