/* fpont 1/00 */
/* pont.net    */
/* tcpServer.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> /* close */

#define SUCCESS 0
#define ERROR 1

#define END_LINE 0x0
#define SERVER_PORT1 15121
#define SERVER_PORT2 25121
#define MAX_MSG 100

/* function readline */
char *read_client(int newSd);

int main(int argc, char *argv[])
{

    int sd, newSd, cliLen, rc;

    struct sockaddr_in cliAddr, servAddr;
    char line[MAX_MSG];

    char recvBuff[1024];
    char sendBuff[1024];

    /* create socket */
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd < 0)
    {
        perror("cannot open socket ");
        return ERROR;
    }

    /* bind server port */
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(SERVER_PORT1);

    // Enable Socket Reuse
    int opt = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        return ERROR;
    }

    if (bind(sd, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("cannot bind port ");
        return ERROR;
    }

    listen(sd, 5);

    while (1)
    {
        printf("%s: waiting for data on port TCP %u\n", argv[0], SERVER_PORT1);
        fflush(stdout); // Flush the output buffer

        cliLen = sizeof(cliAddr);
        newSd = accept(sd, (struct sockaddr *)&cliAddr, &cliLen);
        if (newSd < 0)
        {
            perror("cannot accept connection ");
            return ERROR;
        }

        // read from client //
        strcpy(recvBuff, read_client(newSd)); // call read_client() which call read()
        printf("%s: received from %s:TCP%d : %s\n", argv[0],
               inet_ntoa(cliAddr.sin_addr),
               ntohs(cliAddr.sin_port), recvBuff);

        // keep name and birthdate in tokens
        char *token = strtok(recvBuff, " ");
        char *tokens[10];
        int token_count = 0;
        while (token != NULL)
        {
            tokens[token_count] = token;
            token_count++;

            token = strtok(NULL, " ");
        }

        // print name and surname
        printf("name: %s %s\n", tokens[0], tokens[1]);

        // get birth date and convert to A.D.
        char birthdate[9];
        strcpy(birthdate, tokens[2]);
        birthdate[8] = '\0';
        printf("birthdate: %s\n", birthdate);

        char year_char[5];
        strcpy(year_char, birthdate + 4);

        int year_int = atoi(year_char);
        year_int = year_int - 543; // convert to A.D.
        printf("year: %i\n", year_int);
        // end read from client //

        // send response to client //
        memset(sendBuff, 0, sizeof(sendBuff));

        // send "Server recieved"
        strcpy(sendBuff, "Server recieved");
        rc = send(newSd, sendBuff, sizeof(sendBuff), 0);
        printf("'%s' is sent\n", sendBuff);

        memset(sendBuff, 0, sizeof(sendBuff));

        // send converted year
        sprintf(sendBuff, "%i", year_int);
        rc = send(newSd, sendBuff, sizeof(sendBuff), 0);
        printf("'%s' is sent\n", sendBuff);

        close(newSd);
        // end send response to client //

    } /* while (1) */
}

char *read_client(int newSd)
{
    char recvBuff[1024];
    memset(recvBuff, 0, sizeof(recvBuff));

    int rc = read(newSd, recvBuff, sizeof(recvBuff));
    if (rc < 0)
    {
        perror("Error reading from socket");
        return NULL;
    }
    else if (rc == 0)
    {
        printf("Connection closed by client\n");
        return NULL;
    }
    else
    {
        // Allocate memory for the received string (including null terminator)
        char *received_data = malloc(rc + 1);
        if (received_data == NULL)
        {
            perror("Failed to allocate memory for received data");
            return NULL;
        }

        // Copy the received data into the dynamically allocated memory
        memcpy(received_data, recvBuff, rc);
        received_data[rc] = '\0'; // Null-terminate the string

        return received_data;
    }
}
