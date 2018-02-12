/*
Copyright (C) Olivier Lambert

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/

/*	DAEMON PART :
/*	20070404, O. Lambert							*/
/*	Based on some Internet source				                */
/* 	Plus Linux system programming for SIGNALS             		        */


/*	SERIAL PART :
/*	20070404, O. Lambert							*/
/*	Based on T. Poncery and Linux Serial Programming Howto                  */
/* 	Plus Linux system programming for SIGIO               		        */


/* ALL INCLUDE */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <termios.h> 
#include <fcntl.h>
#include <unistd.h>


/* ALL DEFINE */

#define MYPORT 		1025			// the port users will be connecting to
#define LOGFILE		"/var/log/dlift.log"	// log file
#define CONFILE		"/etc/dlift/dlift.conf"	// conf file, read it !
#define BACKLOG 	100			// how many pending connections queue will hold
#define BAUDRATE B9600				// Speed of COM
#define _POSIX_SOURCE 	1
#define FALSE 		0			// boolean
#define TRUE 		1			// boolean


// MEGALIFT LANGUAGE
#define ACKLIFT		"30"			// MM260 > PC : aswer of megalift after an order
#define ENDOF		"40"			// MM260 > PC : end of work for megalift
#define STATUS		"35"			// MM260 > PC : aswer of megalift after a status query
#define IMSTART		"05"			// PC > MM260 : immediate start command
#define TXTSTR		"20"			// PC > MM260 : text string command
#define ACKPC		"30"			// PC > MM260 : reception acknowledgement
#define STATUSQUERY	"45"			// PC > MM260 : status query
#define CONTROL		"46"			// PC > MM260 : control sequence
#define TERMINIT	"60"			// PC > MM260 : terminal init





/////// GLOBAL VAR ///////

int testmode=0	;		//Testmode : if 0, not in test

char port[50],serial_port1[50],serial_port2[50],megalift01[50],megalift02[50],currentmegalift[50];

/* Signal Handler for IO */
void signal_handler_IO (int status);   
int wait_flag=TRUE;              /* TRUE as far there is no signal */
	
/* Return value, to understand what happened in seriel function */
char retvalue [128];

// useful for loop

volatile int STOP=FALSE;


void catch_alarm(int sig_num)
{
    printf("Operation timed out. Exiting...\n\n");
    strcpy(retvalue,"Délais d'attente dépassé !");
    STOP=TRUE;
}

/////// SERIAL PART ///////


char serial(char param[])
{
signal(SIGALRM, catch_alarm);

/* string builded char by char */	
char  mess[128];

/* Number of command in the datagram PC > MM260 */
char  order[3];

/* Number of command in the datagram MM260 > PC */
char  command[3];


/* Some variables */
int fd, res, lg;
char c, code;





// TO DO : parameters here


// TERMIOS PART //

 struct termios oldtio,newtio; // old and new conf for restore
 struct sigaction saio;        // define signal action

//fflush(stdout);
printf("Starting capture on serial port ...\n");
lg = strlen(param);

// Which serial port is connected to Megalift 01 ?
if  (strncmp(megalift01,"serial_port1",12)==0) {strcpy(megalift01,serial_port1);}
else {strcpy(megalift01,serial_port2);}

// Which serial port is connected to Megalift 02 ?
if  (strncmp(megalift02,"serial_port1",12)==0) {strcpy(megalift02,serial_port1);}
else {strcpy(megalift02,serial_port2);}



// Which LIFT is called ?
// note : strlen -1 is for avoid to copy the '\n' char.

if 	(strncmp(param,"02",2)==0)	{printf("Megalift Number 2 is called \n");strncpy(currentmegalift,megalift02,(strlen(megalift02)-1));}
else if (strncmp(param,"01",2)==0)	{printf("Megalift Number 1 is called \n");strncpy(currentmegalift,megalift01,(strlen(megalift01)-1));}
else					{printf("Error on address \n");strcpy(retvalue,"Erreur d'adresse d'envoi");return *retvalue;}


strncpy(order,&param[2],2);

printf("chaine order : %s \n",&order);

printf("SERIAL PORT CALLED : %s \n",currentmegalift);

//printf("longueur currentmegalift : %d \n",strlen(currentmegalift));
/* Open COM1 in NONBLOCK mode etc. See man termios */
 fd = open(currentmegalift, O_RDWR | O_NOCTTY | O_NONBLOCK);
/* Trying to open, error if not good */

 if (fd <0) {perror(currentmegalift); strcpy(retvalue,"Erreur lors de l'ouverture du port série");return *retvalue; }


/* Signal Handler before opening COM */
 saio.sa_handler = signal_handler_IO;
 sigemptyset(& ( saio.sa_mask)); // useful, take care in "Linux Serial Programming Howto", it's incorrect !
 saio.sa_flags = 0;
 saio.sa_restorer = NULL;
 sigaction(SIGIO,&saio,NULL);

 /* Permit to receive a SIGIO */
 fcntl(fd, F_SETOWN, getpid());

 /* Permit to ASYNC */
 fcntl(fd, F_SETFL, FASYNC);

 tcgetattr(fd,&oldtio); /* Saving current conf */

/* FLAGS : see termios man */

newtio.c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON );
newtio.c_oflag &= ~OPOST;
newtio.c_lflag &= ~( IXOFF | ISIG | IEXTEN );
newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD ;
newtio.c_cc[VMIN]=1;
newtio.c_cc[VTIME]=30;

tcflush(fd, TCIFLUSH); // empty the in buffer
tcsetattr(fd,TCSANOW,&newtio); // Apply new parameters


printf("Sending :%s \n",param);
write(fd,param,lg);
// what king of datagram is send by PC ? 
// First, we use the order field in the datagram





fflush(stdout);
bzero(param,lg);


/* loop for serial talking */
alarm(45);
while (STOP==FALSE)
	{

	//pause(); // Waiting for a COM signal...
	//printf("test !!\n");
	//printf("wait flag : %s \n",wait_flag);
	/* wait_flag = FALSE after received SIGIO */

 	if (wait_flag==FALSE) 	
		{	
			
		res = read(fd,&c,1);
		if (res!=1) {printf("VALEUR DE RES DU READ : %d\n",res);strcpy(retvalue,"Confirmation reçue");return *retvalue;}
		code = c;
		printf("Data Received :");		
		fflush(stdout);

			while (c != 13)
				{
				strncat(mess,&code,sizeof(code));printf("%c", code);		
				res = read(fd,&c,1);code = c;	
				}

		printf("\n");
		strncpy(command,&mess[2],2);
		lg = strlen(mess); printf("longueur de la chaine reçue : %d\n",lg);

		//LENGHT TEST FOR DEBUG ACK
		int datagram=2;
		if (lg==16) {datagram=3;}	
		

		strncpy(command,&mess[datagram],2);
		//printf("COMMAND IS : %s\n",&command);

		wait_flag = TRUE;      // waiting for new data
		// STRING PARSER in order to respond


		// Now, we check the aswer of the order

		// if we give the IMSTART order, we need to receive that :



		if (strncmp(order,IMSTART,2)==0)
			{

			if 	(strncmp(command,ACKPC,2)==0) 	
				{printf("MM260 ACK \n");strcpy(retvalue,"Confirmation reçue");}

			else if (strncmp(command,ENDOF,2)==0)
				{
				printf("MM260 ACK PICKING \n");
				lg = strlen("0230X\n");printf("longueur du ACK envoyé : %d \n",lg);write(fd,"0230X\n",lg);
				STOP=TRUE;strcpy(retvalue,"Opération effectuée");							
				}

			else 	{printf("Error !\n");strcpy(retvalue,"Erreur : opération annulée");STOP=TRUE;return *retvalue; }

			}

		else if (strncmp(order,TXTSTR,2)==0)
			{

			if	(strncmp(command,ACKLIFT,2)==0) 	
				{printf("MM260 ACK TXT STRING \n");STOP=TRUE;strcpy(retvalue,"Demande d'affichage reçue");}

			else	{printf("Error in aswer for TXTSTR ! \n");STOP=TRUE;strcpy(retvalue,"Erreur : opération changement de texte annulée");}
			}

		else if (strncmp(order,STATUSQUERY,2)==0)
			{

			if	(strncmp(command,STATUS,2)==0)
				{printf("MM260 ASWER STATUS QUERY \n");STOP=TRUE;strcpy(retvalue,"Demande de statut effectuée");}

			else	{printf("Error in aswer for STATUSQUERY ! \n");STOP=TRUE;strcpy(retvalue,"Erreur dans la demande de statut");}

			}

		else if (strncmp(order,TERMINIT,2)==0)
			{

			printf("Terminit sended ! \n");STOP=TRUE;strcpy(retvalue,"Terminit OK");

			}
			
		else if (strncmp(order,CONTROL,2)==0)
			{
				if	(strncmp(command,ACKLIFT,2)==0) 	
				{printf("MM260 ACK CONTROL \n");STOP=TRUE;strcpy(retvalue,"Demande de parking reçue");}

			else	{printf("Error in aswer for CONTROL ! \n");STOP=TRUE;strcpy(retvalue,"Erreur : opération parking annulée");}
			
			}

		else {printf("Order Unknown\n");STOP=TRUE;strcpy(retvalue,"Erreur : ordre inconnu");STOP=TRUE;}


		bzero(mess,sizeof(mess));
			


		}
	
//printf("FLAG boucle : %d",STOP);

 	}

return *retvalue;
//bzero(param,sizeof(param));
exit(0);
}





/********************************************************************************
*										*
* Signal Handler. Put wait_flag on FALSE  to indicate receiving char in the loop*
* 										*
*********************************************************************************/

void signal_handler_IO (int status)
{
 	wait_flag = FALSE;
}

/********************************************************************************
*										*
* 			Other Signal Handler for main program			*
* 										*
*********************************************************************************/

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}


















///////////////////////////////////////////////////
//////////////////// START ////////////////////////
///////////////////////////////////////////////////


int main(void)
{
	

	int 	sockfd, new_fd;  	// listen on sock_fd, new connection on new_fd
	struct 	sockaddr_in my_addr;	// my address information
	struct 	sockaddr_in their_addr; // connector's address information
		socklen_t sin_size;
	struct 	sigaction sa;
	int 	yes=1;
	char 	buffer[256], COM[6];
	char	sline[256],parseline[256],param[128],val[128];
	int	line =128;
	char *  info;
	char  	pointeur;
   	char 	other[30];


///////////////////////////////////////////////
// CONF PARSER

FILE*conf;

conf=fopen(CONFILE,"r");

//if conf file doesn't exist, creat it with default data
if (conf==NULL)
{ 
	conf=fopen(CONFILE,"w");
	fputs("#DEFAULT CONF FILE, see readme for more details\n",conf);
	fputs("#serial port used\nSERIAL_PORT1=/dev/ttyS0\nSERIAL_PORT2=/dev/ttyS1\n",conf);
        fputs("#listening port\nport=1025\n",conf);
	fputs("#megalift number on port?\nMegalift01=serial_port1\nMegalift02=serial_port2\n",conf);
	fclose(conf);
}
// else we parse it !
else
{

	
	
	while(fgets(parseline,sizeof parseline,conf)!=NULL)
		{
			
			

			if (parseline[0]!='#')
				{
					
					info=strchr(parseline,'=');*info=0;info++;

					//which parameter is it ?
					if (strncmp(parseline,"port",4)==0) 		{strcpy(port,info);}
					if (strncmp(parseline,"SERIAL_PORT1",12)==0)	{strcpy(serial_port1,info);}
					if (strncmp(parseline,"SERIAL_PORT2",12)==0)	{strcpy(serial_port2,info);}
					if (strncmp(parseline,"Megalift02",10)==0) 	{strcpy(megalift02,info);}
					if (strncmp(parseline,"Megalift01",10)==0) 	{strcpy(megalift01,info);}
				}

	
		}

	fclose(conf);
}
					
					
printf("VARIABLE CONF : %s \n %s \n %s \n %s \n %s \n",port,serial_port1,serial_port2,megalift02,megalift01);

///////////////////////////////////////////////
// GESTION DU LOG DU DAEMON
FILE *log;


log=fopen(LOGFILE, "a+");

// initialisation de la variable temporelle
time_t t;	
t = time(NULL);

// when client connection
// avoid this line of compact flash disk can increase it's lifetime

//fprintf(log, "Connection from client at %s \n",ctime(& t));
	

////////////////////////////////////////////




if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) 		
	{
		perror("socket");
		fprintf(log, "Error creating socket at %s \n",ctime(& t));
		exit(1);
	}

if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) 
	{
		perror("setsockopt");
		fprintf(log, "Error setting socket at %s \n",ctime(& t));
		exit(1);
	}
	
	
my_addr.sin_family = AF_INET;		 // host byte order
my_addr.sin_port = htons(MYPORT);	 // short, network byte order
my_addr.sin_addr.s_addr = INADDR_ANY; 	// automatically fill with my IP
memset(&(my_addr.sin_zero), '\0', 8); 	// zero the rest of the struct

if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) 
	{
		perror("bind");
		fprintf(log, "Socket : Binding Error at %s \n",ctime(& t));
		exit(1);
	}

if (listen(sockfd, BACKLOG) == -1) 
	{
		perror("listen");
		fprintf(log, "Listening error at %s \n",ctime(& t));
		exit(1);
	}
	
	

sa.sa_handler = sigchld_handler; // reap all dead processes
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_RESTART;

if (sigaction(SIGCHLD, &sa, NULL) == -1) 
	{
		perror("sigaction");
		fprintf(log, "Sigaction at %s \n",ctime(& t));
		exit(1);
	}

while(1) {  // main accept() loop
	sin_size = sizeof(struct sockaddr_in);

	if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) 
		{
			perror("accept");
			fprintf(log, "Error accepting socket at %s \n",ctime(& t));
			continue;
		}
		
		printf("server: got connection from %s\n",inet_ntoa(their_addr.sin_addr));

		if (!fork()) { // this is the child process
		close(sockfd); // child doesn't need the listener
			
			
// STEP 1 : just after connection, sending OK to the client
		if (send(new_fd, "OK\n", 3, 0) == -1) 
			{
				perror("send");
				fprintf(log, "Error sending data at %s \n",ctime(& t));
			}
				bzero(buffer,256);

// STEP 2 : read the datagram				
		if (read(new_fd,buffer,sizeof(buffer)) == -1)
			{
				perror("read datagram");
				fprintf(log, "Error reading datagram at %s \n",ctime(& t));
			}
				
		printf("Received : %s\n",buffer);	
			
// OPTIONAL STEP : for testing purpose
// if dlift received an "alive" demand, responding "OK"
// and starting testmode 

strcpy(other,"alive");

		if (strcmp(other,buffer)==0)
			{
				send(new_fd, "OK\n", 3, 0);
				bzero(buffer,256);testmode=1;
			}
			
// MAINTENANCE STEP : for fixing purpose
strcpy(other,"repair");

		if (strcmp(other,buffer)==0)
			{
				//send(new_fd, "OK\n", 3, 0);
				bzero(buffer,256);testmode=2;
				printf("Starting repair mode\n");
			}
// REBOOT STEP 
strcpy(other,"reboot");

		if (strcmp(other,buffer)==0)
			{
				//send(new_fd, "OK\n",3,0);
				bzero(buffer,256);testmode=3;
				printf("Starting reboot\n");
			}

// STATS STEP
strcpy(other,"stats");

		if (strcmp(other,buffer)==0)
			{
				//send(new_fd, "OK\n",3,0);
                                bzero(buffer,256);testmode=4;
                                printf("Starting stats\n");
			}

// SYSTEM IS READY TO GO. SENDING DATA TO TOWERS !		
///////////////////////////////////////////////:
		if (testmode==1)  
			{
				// STEP : read the test datagram				
				if (read(new_fd,buffer,sizeof(buffer)) == -1)
					{
						perror("read datagram");
						fprintf(log, "Error reading datagram at %s \n",ctime(& t));
					}
				serial(buffer);bzero(buffer,256);
				testmode=0;
			}

		else if (testmode==2)	
			{
				system("sh /etc/init.d/dlift restart");printf("Restart daemon !\n");
				testmode=0;
			}			
		
		else if (testmode==3)
			{
				system("reboot");printf("System will reboot !\n");
				testmode=0;
			}

		else if (testmode==4)
			{
				system("w>/tmp/stats");
				FILE*stats;stats=fopen("/tmp/stats","r");
				fgets(sline,sizeof(sline),stats);
				printf("taille sline %d \n",strlen(sline));
				send(new_fd,sline,strlen(sline),0);
				fclose(stats);
				testmode=0;
			}

		else if (testmode==0)
			{
				serial(buffer);
			}
		
		else printf("ERROR");
			
//printf("Result of serial function : %s \n",serial(buffer));
		printf("fin de serial : %s \n",retvalue);
		testmode=0;
		int taille;

		taille = strlen(retvalue);




		if (send(new_fd, retvalue, taille, 0) == -1) 
			{
				perror("send retvalue");
				fprintf(log, "Error sending data at %s \n",ctime(& t));
			}
				printf("Envoyé : %s \n",retvalue);
				bzero(buffer,256);


		//bzero(buffer,sizeof(buffer));			
		exit(0);

			}
		close(new_fd);  // parent doesn't need this
	}
	
	fclose(log);
	return 0;
}
