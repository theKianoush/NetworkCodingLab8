// By: Kianoush Ranjbar

// so what this does is
// if you are in location1 and you send a message to location2
// it will save that message in a globalBuffer
// and every 20 seconds it will forward that globalBuffer to all machines
// it will decrement the TTL every 20 seconds until TTL == 0

// this program does not handle duplicates

#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <math.h> // needed for sqrt() and pow() functions
#include <search.h>
#include <stdbool.h>




#define STDIN 0
#define CONFIG_FILE "config.file"
#define BUF_SIZE 1024
#define MAX_MSG_LEN 1000
#define MAX_INPUTS 100
#define MAXLINE 2048

char globalBuffer[MAXLINE];




int global_location; // myLocation
int sendersLoc; 	 // sendersLocation
int globalTTL;
char *fromPort;
char *fromPort2;
int globalSeqNumber;
char *locationExtract;
int globalMOVE;





void removeFromPort(char *s);

int copyMade = 0;


int main(int argc, char *argv[])
{
  int sd, newsd; /* socket descriptor */
  int i;  /* loop variable */
  struct sockaddr_in server_address; /* my address */
  struct sockaddr_in from_address;  /* address of sender */
  char bufferReceived[1000]; // used in recvfrom
  char serverIP[20]; 
  int portNumber; // get this from command line
  int configPortNumber, location = 0; 
  int rc; // always need to check return codes!
  socklen_t fromLength;
  int flags = 0; // used for recvfrom
  fd_set socketFDS; // the socket descriptor set
  int maxSD; // tells the OS how many sockets are set
    
	

  
  int TTL = 5;
  globalTTL = TTL;
    
  int seqNumber = 1;
  globalSeqNumber = seqNumber;

    
    
   fromPort = argv[1];    
 
    
   const int rows = 5; // rows = 7 for test
   const int cols = 3; // cols = 6 for test
   
   //printf("Enter numbers of rows: ");
   //scanf("%d", &rows);
   //getchar();      // so it doesnt interfere with select function

   //printf("Enter number of columns: ");
   //scanf("%d", &cols);
   //getchar();

   
    
   printf("There are %d rows and %d columns for the grid\n", rows, cols);
 
   int grid[rows][cols];

    // Initialize the grid with sequential numbers
    int num = 1;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            grid[i][j] = num;
            num++;
        }
    }
  
        
    
  //--------------------------------------------------------------------
    // display config file stuff
    
        FILE *config_file = fopen(CONFIG_FILE, "r");   // if config file is specified as parameter, open it
    if (config_file == NULL) {
        perror("Error opening config file");
        exit(EXIT_FAILURE);
    }
  
	
	    // first display my location 
    while (fscanf(config_file, "%s %d %d", serverIP, &configPortNumber, &location) == 3) {
        
        char str[10];
        sprintf(str, "%d", configPortNumber);
        
        if (strstr(str, argv[1])){
            
        printf("My Location: %d\n", location);
	global_location = location;      // save my location in a global variable so we can include in the buffer message

        }
    }
    
        rewind(config_file);

	
    // then print contents of the file
    while (fscanf(config_file, "%s %d %d", serverIP, &portNumber, &location) == 3){
				printf("IPaddresses '%s', port '%d', location '%d'\n", serverIP, portNumber, location );
	}
		
	 fclose(config_file);    // close the config file
  
  //---------------------------------------------------------------------
  /* first, decide if we have the right number of parameters */
  if (argc < 2){
    printf("Error: enter <Port Number> as parameter\n");
    exit (1);
  }

//------------------------------------------------------------------------
  sd = socket(AF_INET, SOCK_DGRAM, 0); /* create a socket */

  /* always check for errors */
  if (sd == -1){ /* means some kind of error occured */
    perror ("Error creating the socket");
    exit(1); /* just leave if wrong number entered */
  }

  /* now fill in the address data structure we use to sendto the server */
  for (i=0;i<strlen(argv[1]); i++){
    if (!isdigit(argv[1][i]))
      {
	printf ("Error: the port number must be a numerical integer\n");
	exit(1);
      }
  }
//------------------------------------------------------------------------
// make ip address
    
  portNumber = strtol(argv[1], NULL, 10); /* many ways to do this */

  if ((portNumber > 65535) || (portNumber < 0)){
    printf ("Error: you entered an invalid port number out of the range of 0-65535\n");
    exit (1);
  }

  fromLength = sizeof(struct sockaddr_in);

  server_address.sin_family = AF_INET; /* use AF_INET addresses */
  server_address.sin_port = htons(portNumber); /* convert port number */
  server_address.sin_addr.s_addr = INADDR_ANY; /* any adapter */
  
//------------------------------------------------------------------------
  /* the next step is to bind to the address */
  rc = bind (sd, (struct sockaddr *)&server_address,
	     sizeof(struct sockaddr ));
  
  if (rc < 0){
    perror("Error binding to the socket");
    exit (1);
  }

//------------------------------------------------------------------------

    
  while(1){ 
    
    
    memset (bufferReceived, 0, 1000); // zero out the buffers in C
	
		struct timeval timeout;
        timeout.tv_sec = 20;
        timeout.tv_usec = 0;

	FD_ZERO(&socketFDS);// NEW                                 
    FD_SET(sd, &socketFDS); //NEW - sets the bit for the initial sd socket
    FD_SET(STDIN, &socketFDS); // NEW tell it you will look at STDIN too
      
    if (STDIN > sd) { // figure out what the max sd is. biggest number
        maxSD = STDIN;
    }
    else {
        maxSD = sd;
    }

      
      printf("\nEnter a message: ");
      fflush(stdout); // flush the output buffer
	  
	  
      //rc = select(maxSD + 1, &socketFDS, NULL, NULL, NULL); // NEW block until something arrives
      rc = select(maxSD + 1, &socketFDS, NULL, NULL, &timeout); // NEW block until something arrives
      printf("\n\nselect popped rc is %d\n", rc);

if (rc == 0){ // had a timeout!
//reSend(&messages,sd, Partners, myPort, myLoc);
printf("Contents of globalBuffer: %s\n", globalBuffer);
printf ("TIMEOUT!!!\n\n");


int theTTL;
// first extract the TTL from the buffer message          
char* locationTTL = strstr(globalBuffer, "TTL:");

if (locationTTL != NULL) {
    // advance the pointer to the beginning of the value
    locationTTL += strlen("TTL:");

    // extract the value as an integer
     theTTL = atoi(locationTTL);
    
    // decrement the TTL value
    theTTL--;
    
    // update the TTL value in the bufferReceived string
    char newTTL[10]; // assuming the TTL value is less than 10 digits long
    sprintf(newTTL, "%d", theTTL);
    memcpy(locationTTL, newTTL, strlen(newTTL));
}

//printf("theTTL:%d\n", theTTL);

if(theTTL == 0){
	//printf("its OVEERRR!!!");
	memset(globalBuffer, 0, sizeof(globalBuffer));
	}
// if theTTL == 0


// forward with the proper rules to all machines
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


 if (strlen(globalBuffer) > 0 && strstr(globalBuffer, "version")) {
	 
 // remove TTL, and Locations
  char output[1000] = "";
    char* token = strtok(globalBuffer, " ");
	    int first = 1;


    while (token != NULL) {
        if (strncmp(token, "SendersLocation:", 16) != 0 &&
            strncmp(token, "MyLocation:", 11) != 0 &&
            strncmp(token, "TTL:", 4) != 0) {
				
            if (!first) strcat(output, " ");
            strcat(output, token);
            first = 0;
        }
        token = strtok(NULL, " ");
    }

 		  printf("buffer before countdown-forwarding:%s\n\n", output);	// new message has transfered from bufferReceived to output

 
 
	//-------------------------------------------
                 
    int numServers = 0;     // increment through severs in config file
    int addTTL = 1;  // we only need to append the senders location to the buffer message one time
		
    fopen(CONFIG_FILE, "r");

    while (fscanf(config_file, "%s %d %d", serverIP, &configPortNumber, &location) == 3){   // starting with the first line of the config file 
        numServers++;   // these will incrmenet each line of the config file
        
        newsd = socket(AF_INET, SOCK_DGRAM, 0); /* create a socket */
        if (newsd == -1){
            printf("Error creating socket\n");
            exit(1);
        }
        
		// set the destination and port address
        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET; /* use AF_INET addresses */
        server_address.sin_port = htons(configPortNumber); /* convert port number */
        server_address.sin_addr.s_addr = inet_addr(serverIP); /* convert IP addr */
        

	//-------------------------------------------
	
	// if move is in the ouput 
	// use globalMOVE instead of global_location
	        char global_loc2[4];
        sprintf(global_loc2, "%d", global_location);     // convert the senders location to a string so we can append it
        
	

		char sendersloc[] = " SendersLocation:";


    char ttl_str[] = " TTL:";
        char main_ttl[4];
        sprintf(main_ttl, "%d", theTTL);
        
        if(addTTL){  // if the buf is not empty, we will use this to append senders loc to all strings only once
            

            strcat(ttl_str, main_ttl);
            strcat(output, ttl_str);       // then attach the TTL
			
			
			strcat(sendersloc, global_loc2);
            strcat(output, sendersloc);           //  attach the SendersLocation
            
            
            addTTL = 0;
        }
     //-------------------------------------------
        
		 printf("Sending to location: %d, on port: %d, message: '%s'\n", location, configPortNumber, output);         //print what we are sending to server
         sendto(newsd, output, strlen(output), 0, (struct sockaddr *) &server_address, sizeof(server_address)); // send it


         close(newsd);      // close the sockets
    }
            fclose(config_file);    // close the config file
			
						 memset(globalBuffer, 0, sizeof(globalBuffer));
						 memset(output, 0, sizeof(output));
						 copyMade = 0;

			
                    
 }
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
continue;
}

  //------------------------------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------------------------
      
  
     if (FD_ISSET(STDIN, &socketFDS)){ // means i received something from the keyboard. 
     
         
    
		// prepare message to send to all servers in config file
		char buf[BUF_SIZE];
		memset (buf, '\0', 100);
		fgets(buf, BUF_SIZE, stdin);
		size_t len = strlen(buf);
		
		if(len > 0 && buf[len-1] == '\n'){
			buf[len-1] = '\0';
		}
		printf("\n");

		    if (!strcmp(buf, "STOP")){
      printf ("you asked me to end, so i will end\n");
      exit (1);
    }
      
       
    // now send the message to all servers in config file  
    int numServers = 0;     // increment through severs in config file
    int addSendersLoc = 1;  // we only need to append the senders location to the buffer message one time
		
    fopen(CONFIG_FILE, "r");

    while (fscanf(config_file, "%s %d %d", serverIP, &configPortNumber, &location) == 3){   // starting with the first line of the config file 
        numServers++;   // these will incrmenet each line of the config file
        
        newsd = socket(AF_INET, SOCK_DGRAM, 0); /* create a socket */
        if (newsd == -1){
            printf("Error creating socket\n");
            exit(1);
        }
        
		// set the destination and port address
        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET; /* use AF_INET addresses */
        server_address.sin_port = htons(configPortNumber); /* convert port number */
        server_address.sin_addr.s_addr = inet_addr(serverIP); /* convert IP addr */
        

	//-------------------------------------------
	char loc[] = " SendersLocation:";
        char global_loc[4];
        sprintf(global_loc, "%d", global_location);     // convert the senders location to a string so we can append it
        
        
    char fromPort_str[] = " fromPort:";

        
    char ttl_str[] = " TTL:";
        char main_ttl[4];
        sprintf(main_ttl, "%d", TTL);	// convert ttl to string so we can attach it
        
    char seq_str[] = " seqNumber:";
        char main_seq[4];
        sprintf(main_seq, "%d", globalSeqNumber);
		        
        
        if(addSendersLoc && strlen(buf) > 0){  // if the buf is not empty, we will use this to append senders loc to all strings only once
            
            
            strcat(loc, global_loc);
            strcat(buf, loc);           // first attach the SendersLocation
            
            strcat(fromPort_str,fromPort);	// attach fromPort
            strcat(buf,fromPort_str);
            
            strcat(ttl_str, main_ttl);
            strcat(buf, ttl_str);       // then attach the TTL
            
            strcat(seq_str, main_seq);	// attach the seqNumber
            strcat(buf, seq_str);
            
            addSendersLoc = 0;
        }
     //-------------------------------------------
        
		 printf("Sending to location: %d, on port: %d, message: '%s'\n", location, configPortNumber, buf);         //print what we are sending to server
         sendto(newsd, buf, strlen(buf), 0, (struct sockaddr *) &server_address, sizeof(server_address)); // send it

        

         close(newsd);      // close the sockets
    }
            fclose(config_file);    // close the config file
                
    // increment sequence number after message is sent, so when a new message is sent it will have 1 higher seqNumber 
         globalSeqNumber++;
         if (globalTTL == 0) {
    globalTTL = TTL; // reset globalTTL to original value
}
         
    }
  
          
      
  //------------------------------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------------------------
  
      if (FD_ISSET(sd, &socketFDS)){   // if we get something from the network


      rc = recvfrom(sd, bufferReceived, sizeof(bufferReceived), flags,
(struct sockaddr *)&from_address, &fromLength);
      printf ("I received %d bytes from the network\n",rc);
	  
          
          
          
          // when we receive the message, scan the msg for TTL and put that in the global variable
      //----------------------    
          // first extract the TTL from the buffer message          
        char* locationTTL = strstr(bufferReceived, "TTL:");
   
    if (locationTTL != NULL) {
        // advance the pointer to the beginning of the value
        locationTTL += strlen("TTL:");

        // extract the value as an integer
        int theTTL = atoi(locationTTL);
        globalTTL = theTTL;   // put it in a global var so I can use it outside of this if statement
        
            }
     
       //----------------------   
          
  
          // detach the from port and attach it back later, because the if loop below gets confused if the message has two port numbers in it
          char* locationFromPort = strstr(bufferReceived, "fromPort:");
   
if (locationFromPort != NULL) {
    locationFromPort += strlen("fromPort:");
    char theFromPort[6] = {0}; // Allocate space for a 4-digit z value plus null terminator
    strncpy(theFromPort, locationFromPort, 5); // Copy 4 characters from the locationTTL pointer into theTTL array
    fromPort2 = theFromPort; 
}
          removeFromPort(bufferReceived);
		  
//send-path=send-path=send-path=send-path=send-path=send-path=send-path=send-path=send-path=send-path
//send-path=send-path=send-path=send-path=send-path=send-path=send-path=send-path=send-path=send-path
//send-path=send-path=send-path=send-path=send-path=send-path=send-path=send-path=send-path=send-path
		  
		  
	// Declare a character array to hold the path
    char send_path[100];
	
	// locate SendersLocation in the bufferReceived
char* SendersLocationExtract = strstr(bufferReceived, "SendersLocation:");
if (SendersLocationExtract != NULL) {
    SendersLocationExtract += strlen("SendersLocation:"); 
    char theLocation[3] = {0}; 
    int i;
    for (i = 0; i < 2 && isdigit(SendersLocationExtract[i]); i++) { 
        theLocation[i] = SendersLocationExtract[i];
    }
    locationExtract = theLocation; 
}
  
  
  
  int intLocation = atoi(locationExtract);
  
// convert locationExtract to its equivalent port number
	    fopen(CONFIG_FILE, "r");
		int foundPortNumber = -1; // initialize the port number to an invalid value

while (fscanf(config_file, "%s %d %d", serverIP, &portNumber, &location) == 3){
    if (location == intLocation) { // if the location matches the target location
        foundPortNumber = portNumber; // store the port number in a variable
        break; // stop looping
    }
}

fclose(config_file); // close the config file
	
	// convert the portNumber to a string 
	
	        char port_string[6];
        sprintf(port_string, "%d", foundPortNumber);	// convert ttl to string so we can attach it
		
		//printf("portSTRING:%s \n\n", port_string);
	
	
// put the location in the send-path
if (!strstr(send_path, port_string)) {

    strcat(send_path, port_string);
	strcat(send_path, ",");
						  
}

//send-path=send-path=send-path=send-path=send-path=send-path=send-path=send-path=send-path=send-path
//send-path=send-path=send-path=send-path=send-path=send-path=send-path=send-path=send-path=send-path
//send-path=send-path=send-path=send-path=send-path=send-path=send-path=send-path=send-path=send-path

// MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---
// MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---
// MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---


          
          // extract move's value and put in global variable
        char* locationMOVE = strstr(bufferReceived, "move:");
   
    if (locationMOVE != NULL) {
        locationMOVE += strlen("move:");

        int theMOVE = atoi(locationMOVE);
        globalMOVE = theMOVE;   
        
            }

		  		  
		  			char my_loc[] = " MyLocation:";
					char my_loc_str1[4];
		  
		  		 if (strstr(bufferReceived, my_loc) == NULL) {
					 
					 		// if globalMOVE is empty fill it with global_location
				if(globalMOVE == 0){
					
		sprintf(my_loc_str1, "%d", global_location);    
        strcat(my_loc, my_loc_str1);
        strcat(bufferReceived, my_loc);
		
				}
					 
					 else{
        sprintf(my_loc_str1, "%d", globalMOVE);    

        strcat(my_loc, my_loc_str1);
        strcat(bufferReceived, my_loc);
					 }
		
		
		
				 }
				  
		  

      //------------------------
// MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---
// MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---
// MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---MOVE---

          
    // grab the sendersLocation from the buffered message and use it to find coordinates to do distance calculated
        char* locationString = strstr(bufferReceived, "SendersLocation:");

    if (locationString != NULL) {
        // advance the pointer to the beginning of the value
        locationString += strlen("SendersLocation:");

        // extract the value as an integer
        int sendersLocation = atoi(locationString);
        sendersLoc = sendersLocation;   // put it in a global var so I can use it outside of this if statement
    }
     

int x1, y1, x2, y2;
if (globalMOVE == 0) {

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (grid[i][j] == global_location) {  // find coordinates of myLocation
                x2 = i;
                y2 = j;
            }
            if (grid[i][j] == sendersLoc) {     // find coordinates of sendersLocation
                x1 = i;
                y1 = j;
              
            }
        }
    } 
          
}

else{       // find my location's coordinates on the grid to calucalte distance between sender   
             
			 
		       // find my location's coordinates on the grid to calucalte distance between sender   
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (grid[i][j] == globalMOVE) {  // find coordinates of myLocation
                x2 = i;
                y2 = j;
            }
            if (grid[i][j] == sendersLoc) {     // find coordinates of sendersLocation
                x1 = i;
                y1 = j;
              
            }
        }
    } 	 
				 
			 

	}
    // Calculate the Euclidean distance between the positions of myLocation and sendersLocation   
          
    int row_dist = abs(x2 - x1);
    int col_dist = abs(y2 - y1);
    double total_dist = sqrt(pow(row_dist, 2) + pow(col_dist, 2));
    int rounded_down = (int) floor(total_dist);

    printf("Distance between you and the sender is %d\n", rounded_down);    

	   //------------------------    
	               
              // if distance is greater than 2, dont print the message, disregard it
     if (rounded_down > 2) {
        printf("--------------------------------------------------\n");
        printf("The message is out of reach, it is disregarded\n");
        printf("--------------------------------------------------\n");
        memset(bufferReceived, 0, sizeof(bufferReceived));
		
		
		
		
}   
              
     

     
else{
    
        
	  char *version_string = "version:8";	
	  
if (strstr(bufferReceived, argv[1]) && strstr(bufferReceived, version_string)) {  // only print the string if version3 and port number is included in message

    
    //-----------------------
       if (rounded_down > 2) {
    printf("***Darn, this was for me, but it was too far for me to hear\n");
}     
     //----------------------   
    
   

	
    char fromPort_str[] = " fromPort:";		// put fromPort back on so it can bypass this if statement
    strcat(fromPort_str,fromPort2);
    strcat(bufferReceived,fromPort_str);
    
    // save the buffer to a new variable
    char new_bufferReceived[200];
    strcpy(new_bufferReceived, bufferReceived);
    
//zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz
int z = 0;

// Find the end of the string
while (send_path[z] != '\0') {
    z++;
}

// If the last character is a comma, remove it
if (send_path[z-1] == ',') {
    send_path[z-1] = '\0'; // Replace the comma with null terminator
}
	char sendpath_str [] = " send-path:";
	strcat(sendpath_str, send_path);
	strcat(bufferReceived,sendpath_str);
//zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz


  char *ptr;  // create pointer to received message so we can parse it properly
  ptr = strtok(bufferReceived, " ");
  
	printf("--------------------------------------------------\n");
    printf("name:value\n");      
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // this is where we tokenize and print the string
  while (ptr !=NULL){
    

    int len = strcspn(ptr, "\"");
                      
    if (ptr[len] == '\"'){
        char quote[1000];
        strcpy(quote, ptr);
        ptr = strtok(NULL, " ");
        
        while (ptr != NULL && ptr[strcspn(ptr, "\"")] != '\"'){
            strcat(quote, " ");
            strcat(quote, ptr);
            ptr = strtok(NULL, " ");
        }
        if (ptr != NULL) {
            strcat(quote, " ");
            strcat(quote, ptr);
        printf ("'%s'\n",quote);

        }
    }
      
  else {

       printf ("'%s'\n",ptr);

    }
        ptr = strtok(NULL, " ");


    }
			printf("--------------------------------------------------\n");
    
	
    // if new_bufferReceived does not include ACK then do the below
    if (strstr(new_bufferReceived, "type:ACK") == NULL) {
    // the message was for me and I received it, but now i must send back an ack, to the sender
    
    
    //remove msg field
    char* msg_ptr = strstr(new_bufferReceived, "msg:\""); // search for "msg" key
    if (msg_ptr != NULL) {
        char* end_quote_ptr = strchr(msg_ptr+5, '\"'); // locate end of quoted string
        if (end_quote_ptr != NULL) {
            memmove(msg_ptr, end_quote_ptr+2, strlen(end_quote_ptr+1)+1); // remove key, value, and extra space
        }
    }
	
	
	// remove move field
//char* move_ptr = strstr(new_bufferReceived, "move:");
//if (move_ptr != NULL) {
//    char* end_ptr = strchr(move_ptr, ' '); // locate end of field value
//    if (end_ptr != NULL) {
//        memmove(move_ptr, end_ptr+1, strlen(end_ptr)); // remove field and extra space
//    }
//}

    
    // add ACK field 
            char type_ACK[] = " type:ACK";
            strcat(new_bufferReceived, type_ACK);         
			
			
// trying to make the send-path on the ACK side work			
//---------------------------------------------------		
	// zero out the send path, it attaches it self earlier 
	// remove the send-path field
	char* send_path_ptr = strstr(new_bufferReceived, "send-path:");
if (send_path_ptr != NULL) {
    char* end_field_ptr = strchr(send_path_ptr, ' '); // locate end of send-path field
    if (end_field_ptr != NULL) {
        memmove(send_path_ptr, end_field_ptr+1, strlen(end_field_ptr+1)+1); // remove field, value, and extra space
    }
}
//	    memset(send_path, 0, sizeof(send_path));
//---------------------------------------------------
          


	
	    // now swap l numbers  
		// fromPort = sendersLocation// toPort = myLocation
char* sendersLocation_ptr = strstr(new_bufferReceived, "SendersLocation:");
char* myLocation_ptr = strstr(new_bufferReceived, "MyLocation:");
if (sendersLocation_ptr != NULL && myLocation_ptr != NULL) {
    // swap the values of SendersLocation and MyLocation
    char temp[10];
        strncpy(temp, sendersLocation_ptr + 16, 1);
        strncpy(sendersLocation_ptr + 16, myLocation_ptr + 11, 1);
        strncpy(myLocation_ptr + 11, temp, 1);
}

			  
    
    // now swap port numbers  
    char* toPort_ptr = strstr(new_bufferReceived, "toPort:");
    char* fromPort_ptr = strstr(new_bufferReceived, "fromPort:");
    if (toPort_ptr != NULL && fromPort_ptr != NULL) {
        // swap the values of toPort and fromPort
        char temp[20];
        strncpy(temp, toPort_ptr + 7, 5);
        strncpy(toPort_ptr + 7, fromPort_ptr + 9, 5);
        strncpy(fromPort_ptr + 9, temp, 5);
    }


    
    // now send the ACK back to the sender

    //extract toPort value and save in an int
        int toPort_val2 = 0;
        char* toPort_ptr2 = strstr(new_bufferReceived, "toPort:");
    if (toPort_ptr2 != NULL) {
        sscanf(toPort_ptr2 + 7, "%d", &toPort_val2);
    }
    
    char serverIP[] = "127.0.0.1";

    // now send the ACK back
        newsd = socket(AF_INET, SOCK_DGRAM, 0); /* create a socket */
    
        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET; /* use AF_INET addresses */
        server_address.sin_port = htons(toPort_val2); /* convert port number */
        server_address.sin_addr.s_addr = inet_addr(serverIP); /* convert IP addr */
        
        printf("Sending ACK back to the sender on port: %d\n", toPort_val2);         //print what we are sending to server
        sendto(newsd, new_bufferReceived, strlen(new_bufferReceived), 0, (struct sockaddr *) &server_address, sizeof(server_address)); // send it
    }

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}
    
      else if (globalTTL > 1){     
        printf("--------------------------------------------------\n");
          printf("Recieved a message not destined for me!\n");
          printf("So I will decrement the TTL, and forward the message to all peers\n");
        printf("--------------------------------------------------\n");
          
		  
		 // dont forward
		 if(strstr(bufferReceived, "move:")){
			 memset(bufferReceived, 0, sizeof(bufferReceived));

}


        if (copyMade == 0) {
            strcpy(globalBuffer, bufferReceived);
            copyMade = 1;
        }

          
        // 1.   
      //----------------------    
          // first extract the TTL from the buffer message          
        char* locationTTL = strstr(bufferReceived, "TTL:");
   
    if (locationTTL != NULL) {
        // advance the pointer to the beginning of the value
        locationTTL += strlen("TTL:");

        // extract the value as an integer
        int theTTL = atoi(locationTTL);
        globalTTL = theTTL;   // put it in a global var so I can use it outside of this if statement
        
            }
     
       //----------------------   
          
        // 2.  
          globalTTL--;  // decrement the ttl
          
          
				


        // 3. 
          // remove the old ttl from the message, remove SendersLocations and MyLocation 
		  // because the location from the machine that sent the message will be the SendersLocation of the new message
		  

	// remove move field
//char* move_ptr = strstr(bufferReceived, "move:");
//if (move_ptr != NULL) {
//    char* end_ptr = strchr(move_ptr, ' '); // locate end of field value
//    if (end_ptr != NULL) {
//        memmove(move_ptr, end_ptr+1, strlen(end_ptr)); // remove field and extra space
//    }
//}

 
 // remove TTL, and Locations
  char output[1000] = "";
    char* token = strtok(bufferReceived, " ");
	    int first = 1;


    while (token != NULL) {
        if (strncmp(token, "SendersLocation:", 16) != 0 &&
            strncmp(token, "MyLocation:", 11) != 0 &&
            strncmp(token, "TTL:", 4) != 0) {
				
            if (!first) strcat(output, " ");
            strcat(output, token);
            first = 0;
        }
        token = strtok(NULL, " ");
    }

 		  printf("buffer before forwarding:%s\n\n", output);	// new message has transfered from bufferReceived to output

 
 
	//-------------------------------------------
                 
    int numServers = 0;     // increment through severs in config file
    int addTTL = 1;  // we only need to append the senders location to the buffer message one time
		
    fopen(CONFIG_FILE, "r");

    while (fscanf(config_file, "%s %d %d", serverIP, &configPortNumber, &location) == 3){   // starting with the first line of the config file 
        numServers++;   // these will incrmenet each line of the config file
        
        newsd = socket(AF_INET, SOCK_DGRAM, 0); /* create a socket */
        if (newsd == -1){
            printf("Error creating socket\n");
            exit(1);
        }
        
		// set the destination and port address
        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET; /* use AF_INET addresses */
        server_address.sin_port = htons(configPortNumber); /* convert port number */
        server_address.sin_addr.s_addr = inet_addr(serverIP); /* convert IP addr */
        

	//-------------------------------------------
	
	// if move is in the ouput 
	// use globalMOVE instead of global_location
	        char global_loc2[4];
        sprintf(global_loc2, "%d", global_location);     // convert the senders location to a string so we can append it
        
	

		char sendersloc[] = " SendersLocation:";


    char ttl_str[] = " TTL:";
        char main_ttl[4];
        sprintf(main_ttl, "%d", globalTTL);
        
        if(addTTL){  // if the buf is not empty, we will use this to append senders loc to all strings only once
            

            strcat(ttl_str, main_ttl);
            strcat(output, ttl_str);       // then attach the TTL
			
			
			strcat(sendersloc, global_loc2);
            strcat(output, sendersloc);           //  attach the SendersLocation
            
            
            addTTL = 0;
        }
     //-------------------------------------------
        
		 printf("Sending to location: %d, on port: %d, message: '%s'\n", location, configPortNumber, output);         //print what we are sending to server
         sendto(newsd, output, strlen(output), 0, (struct sockaddr *) &server_address, sizeof(server_address)); // send it


         close(newsd);      // close the sockets
    }
            fclose(config_file);    // close the config file
                    
          


          
      } // end of forwarding 
	  
	  
	   // reset vars used to make distance calculations
          x2 = 0;
          y2 = 0;
          x1 = 0;
          y1 = 0;
          rounded_down = 0;
          total_dist = 0;
          row_dist = 0;
          col_dist = 0;
          
    } // end of network input
   
  }//end of network input
      
  } // end of program while loop    

	return 0;    

} // end of main()




void removeFromPort(char *s) {
   char *ptr = strstr(s, "fromPort:"); // find the substring to remove
   if (ptr != NULL) {
      char *endptr = ptr + strlen("fromPort:"); // move the end pointer to the number after the fromPort:
      while (*endptr >= '0' && *endptr <= '9') {
         endptr++; // advance the end pointer until it reaches the end of the number
      }
      endptr++; // move the end pointer one space ahead to skip any whitespace characters
      strcpy(ptr, endptr); // copy the characters after the number over the fromPort: substring
   }
}
