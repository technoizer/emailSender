#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>      // for struct hostent
#include <fcntl.h>

char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
void decodeblock(unsigned char in[], char *clrstr);
void b64_decode(char *b64src, char *clrdst);
void encodeblock( unsigned char in[], char b64str[], int len );
void b64_encode(char *clrstr, char *b64dst);

int main(int argc, char **argv)
{
    int sockfd, sockcli, retval;
    struct sockaddr_in servaddr;
    unsigned int server_port = 25;
    struct hostent *hostptr;
    struct in_addr *ptr;
    unsigned short  port_number;
    char userinput[801]; //MAX_IN is initialized to 800
    char msg[1024];
    char buf[1024],addr[17];
    char temp[1024];
    char usr[64],pwd[64];
    int code,i;

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
    //printf ("server address: %u %s\n",ptr->s_addr,inet_ntoa(*ptr));
    strcpy(addr,inet_ntoa(*ptr));

    //Create Socket
    sockcli = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    printf("Making Socket Successfull\n");
    bzero(&servaddr, sizeof(servaddr)); //sama seperti memset(a,0,sizeof(a));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port_number);
    inet_aton(addr, &servaddr.sin_addr);
    
    //Connect to SMTP Server    
    retval = connect(sockcli, (struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("Connect Successfull\n");
    retval=read(sockcli, buf, sizeof(buf)-1);
    buf[retval] = '\0';
    printf("%s", buf);

    //Read-Write Process
    for (i=1; i <= 7; i++){
        memset(temp,0,sizeof(temp));
        if (i==0){
            sprintf(msg,"EHLO %s\r\n",addr);
        }
        if (i==1){
            sprintf(msg,"AUTH LOGIN\r\n");
        }
        else if (i==2){
            printf ("Username : ");
            scanf("%s",usr);
            b64_encode(usr,temp);
            //printf("%s\n",temp);
            sprintf(msg,"%s\r\n",temp);
        }
        else if (i==3){
            printf ("Password : ");
            scanf("%s",pwd);
            b64_encode(pwd,temp);
            //printf("%s\n",temp);
            sprintf(msg,"%s\r\n",temp);
        }
        else if (i==4){
            printf ("Enter Email Sender : ");
            scanf("%s",temp);
            sprintf(msg,"MAIL FROM:%s\r\n",temp);
            printf("%s",msg );
        }
        else if (i==5){
            printf ("Enter Email Recipient : ");
            scanf("%s",temp);
            sprintf(msg,"RCPT TO:%s\r\n",temp);
            printf("%s",msg );
        }
        else if (i==6){
            sprintf(msg,"DATA\r\n");
        }
        else if (i==7){
            char sbj[1024],body[1024];
            printf ("Subject : ");
            scanf ("%s",sbj);
            getchar();
            printf ("Enter text body\n");
            fgets(body,sizeof(body),stdin);    
            sprintf(msg,"SUBJECT:%s\n\n%s\r\n.\r\n",sbj,body);
        }      
        write(sockcli, msg, strlen(msg));
        retval=read(sockcli, buf, sizeof(buf)-1);
        buf[retval] = '\0';
        printf("%s", buf);
        sscanf(buf,"%d %*s",&code);
        if (i == 3 && code != 235)
            i = 0;

        if (i == 4 && code != 250)
            i = 3;

        if (i == 5 && code != 250)
            i = 4;
    }
    return 0;
  }

/* decodeblock - decode 4 '6-bit' characters into 3 8-bit binary bytes */
void decodeblock(unsigned char in[], char *clrstr) {
  unsigned char out[4];
  out[0] = in[0] << 2 | in[1] >> 4;
  out[1] = in[1] << 4 | in[2] >> 2;
  out[2] = in[2] << 6 | in[3] >> 0;
  out[3] = '\0';
  strncat(clrstr, out, sizeof(out));
}

void b64_decode(char *b64src, char *clrdst) {
  int c, phase, i;
  unsigned char in[4];
  char *p;

  clrdst[0] = '\0';
  phase = 0; i=0;
  while(b64src[i]) {
    c = (int) b64src[i];
    if(c == '=') {
      decodeblock(in, clrdst); 
      break;
    }
    p = strchr(b64, c);
    if(p) {
      in[phase] = p - b64;
      phase = (phase + 1) % 4;
      if(phase == 0) {
        decodeblock(in, clrdst);
        in[0]=in[1]=in[2]=in[3]=0;
      }
    }
    i++;
  }
}

/* encodeblock - encode 3 8-bit binary bytes as 4 '6-bit' characters */
void encodeblock( unsigned char in[], char b64str[], int len ) {
    unsigned char out[5];
    out[0] = b64[ in[0] >> 2 ];
    out[1] = b64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (unsigned char) (len > 1 ? b64[ ((in[1] & 0x0f) << 2) |
             ((in[2] & 0xc0) >> 6) ] : '=');
    out[3] = (unsigned char) (len > 2 ? b64[ in[2] & 0x3f ] : '=');
    out[4] = '\0';
    strncat(b64str, out, sizeof(out));
}

/* encode - base64 encode a stream, adding padding if needed */
void b64_encode(char *clrstr, char *b64dst) {
  unsigned char in[3];
  int i, len = 0;
  int j = 0;

  b64dst[0] = '\0';
  while(clrstr[j]) {
    len = 0;
    for(i=0; i<3; i++) {
     in[i] = (unsigned char) clrstr[j];
     if(clrstr[j]) {
        len++; j++;
      }
      else in[i] = 0;
    }
    if( len ) {
      encodeblock( in, b64dst, len );
    }
  }
}
