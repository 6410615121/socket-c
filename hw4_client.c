#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> /* close */

#define PORT1 15121
#define PORT2 25121
#define MAX_MSG 100

char *read_server(int newSd);

int main(int argc, char *argv[])
{

    int sd1, sd2, rc, i;
    struct sockaddr_in localAddr, servAddr1, servAddr2;
    struct hostent *h;
    char recvBuff[1024];
    char sendBuff[1024];

    if (argc != 2)
    {
        printf("usage: %s <server>\n", argv[0]);
        exit(1);
    }

    h = gethostbyname(argv[1]);
    if (h == NULL)
    {
        printf("%s: unknown host '%s'\n", argv[0], argv[1]);
        exit(1);
    }

    /* -------------------------------- connetion ------------------------------- */
    servAddr1.sin_family = h->h_addrtype;
    memcpy((char *)&servAddr1.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    servAddr1.sin_port = htons(PORT1);
    
    servAddr2.sin_family = h->h_addrtype;
    memcpy((char *)&servAddr2.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    servAddr2.sin_port = htons(PORT2);


    /* create socket */
    sd1 = socket(AF_INET, SOCK_STREAM, 0);
    if (sd1 < 0)
    {
        perror("cannot open socket ");
        exit(1);
    }

    sd2 = socket(AF_INET, SOCK_STREAM, 0);
    if (sd2 < 0)
    {
        perror("cannot open socket ");
        exit(1);
    }

    /* connect to server */
    rc = connect(sd1, (struct sockaddr *)&servAddr1, sizeof(servAddr1));
    if (rc < 0)
    {
        perror("cannot connect ");
        exit(1);
    }

    rc = connect(sd2, (struct sockaddr *)&servAddr2, sizeof(servAddr2));
    if (rc < 0)
    {
        perror("cannot connect ");
        exit(1);
    }
    /* -------------------------------------------------------------------------- */

    /* --------------------------- send args to server -------------------------- */
    /* input */
    char name[15];
    char surname[15];
    char birthday[9];

    printf("Input name: ");
    scanf("%s", &name);

    printf("Input surname: ");
    scanf("%s", &surname);

    printf("Input birthday: ");
    scanf("%s", &birthday);
    /* end input */

    // prepare sendBuffer
    memset(sendBuff, 0, sizeof(sendBuff));
    sprintf(sendBuff, "%s %s %s", name, surname, birthday);

    // send sendBuffer to server //
    rc = send(sd1, sendBuff, strlen(sendBuff), 0);
    // printf("data sent: %s\n", sendBuff);
    /* -------------------------------------------------------------------------- */

    /* ---------------------- received messages from server ---------------------- */
    // Receive responses from port1
    strcpy(recvBuff, read_server(sd1));
    if (recvBuff == NULL)
    {
        printf("Error receiving response from server\n");
        exit(1);
    }
    printf("Message from server: '%s'\n", recvBuff);

    // read year from port2
    strcpy(recvBuff, read_server(sd2));
    if (recvBuff == NULL)
    {
        printf("Error receiving response from server\n");
        exit(1);
    }
    printf("Year from server: '%s'\n", recvBuff);
    /* -------------------------------------------------------------------------- */

    /* ------------------------------- send "bye" ------------------------------- */
    memset(sendBuff, 0, sizeof(sendBuff));

    // send sendBuffer to server port2 //
    strcat(sendBuff, "Bye");
    rc = send(sd2, sendBuff, strlen(sendBuff), 0);
    // printf("data sent: %s\n", sendBuff);
    /* -------------------------------------------------------------------------- */

    close(sd1);
    close(sd2);

    return 0;
}

/* -------------------------------- functions ------------------------------- */
char *read_server(int newSd)
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
/* -------------------------------------------------------------------------- */