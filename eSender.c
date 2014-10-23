#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>      // for struct hostent
#include <fcntl.h>
#define MAX_REQUEST 1024
#define MAX_REPLY 1024

int main(int argc, char **argv)
{
    int sockfd, sockcli, retval;
    struct sockaddr_in servaddr;
    char request[MAX_REQUEST+1];
    char reply[MAX_REPLY+1];
    unsigned int server_port = 25;
    struct hostent *hostptr;
    struct in_addr *ptr;
    unsigned short	port_number;
    char userinput[801]; //MAX_IN is initialized to 800
    char msg[1024];
    char buf[1024],addr[17];
    int code;

    // Read hostname and port from the command-line
    if (argc > 2)
        port_number = atoi(argv[2]);
    else
      	port_number = 25;

    if ( (hostptr = (struct hostent *) gethostbyname(argv[1])) == NULL) {
        perror("gethostbyname error for host");
	      return(1);
    }

    ptr = (struct in_addr *) *(hostptr->h_addr_list);
    //printf ("DEBUG: server address: %u %s\n",ptr->s_addr,inet_ntoa(*ptr));
    strcpy(addr,inet_ntoa(*ptr));
    sockcli = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  	printf("Making Socket Successfull\n");
  	bzero(&servaddr, sizeof(servaddr)); //sama seperti memset(a,0,sizeof(a));
  	servaddr.sin_family = AF_INET;
  	servaddr.sin_port = htons(port_number);
  	inet_aton(inet_ntoa(*ptr), &servaddr.sin_addr);

    retval = connect(sockcli, (struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("Connect Successfull\n");
    retval=read(sockcli, buf, sizeof(buf)-1);
    buf[retval] = '\0';
    printf("%s", buf);

  	/*sprintf(msg,"EHLO %s\r\n",addr);
  	write(sockcli, msg, strlen(msg));
  	retval=read(sockcli, buf, sizeof(buf)-1);
  	buf[retval] = '\0';
  	printf("%s", buf);*/
    
    sprintf(msg,"AUTH LOGIN\r\n");
    write(sockcli, msg, strlen(msg));
    retval=read(sockcli, buf, sizeof(buf)-1);
    buf[retval] = '\0';
    printf("%s", buf);

    sprintf(msg,"dGVzQGl0cy1zYnkuZWR1\r\n");
    write(sockcli, msg, strlen(msg));
    retval=read(sockcli, buf, sizeof(buf)-1);
    buf[retval] = '\0';
    printf("%s", buf);

    sprintf(msg,"dGVzcHJvZ2phcg==\r\n");
    write(sockcli, msg, strlen(msg));
    retval=read(sockcli, buf, sizeof(buf)-1);
    buf[retval] = '\0';
    printf("%s", buf);

    sprintf(msg,"MAIL FROM:tes@its-sby.edu\r\n");
    write(sockcli, msg, strlen(msg));
    retval=read(sockcli, buf, sizeof(buf)-1);
    buf[retval] = '\0';
    printf("%s", buf);
    char rcpt[1024];
    do {
        printf ("Enter Recipient ");
        scanf("%s",rcpt);
        sprintf(msg,"RCPT TO:%s\r\n",rcpt);
        printf("%s",msg );
        write(sockcli, msg, strlen(msg));
        retval=read(sockcli, buf, sizeof(buf)-1);
        buf[retval] = '\0';
        printf("%s", buf);
        sscanf(buf,"%d %*s",&code);
    } while(code != 250);

    sprintf(msg,"DATA\r\n");
    write(sockcli, msg, strlen(msg));
    retval=read(sockcli, buf, sizeof(buf)-1);
    buf[retval] = '\0';
    printf("%s", buf);

    char sbj[1024],body[1024];
    printf ("Subject : ");
    scanf ("%s",sbj);
    getchar();
    printf ("Enter text body\n");
    fgets(body,sizeof(body),stdin);
        
    sprintf(msg,"SUBJECT:%s\n\n%s\r\n.\r\n",sbj,body);
    //printf("%s\n",msg );
    write(sockcli, msg, strlen(msg));
    retval=read(sockcli, buf, sizeof(buf)-1);
    buf[retval] = '\0';
    printf("%s", buf);   

    return 0;

  }
