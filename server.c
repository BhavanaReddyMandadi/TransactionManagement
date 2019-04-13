#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/file.h>
#include <fcntl.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX 1024
#define BUFMAX 2048

void broadcastPortNumber(int portNumber);
char *readInfo(char response[], int accountNo);
char *updateInfo(char *response, int accountNo, float value); 
float ntohf(float value);


struct record
{
    int acctnum;
    char name[20];
    float value;
    int age;
};

int main()
{
    struct sockaddr_in local, remote;
    int sk, nsk, rlen = sizeof(remote), len = sizeof(local);
    char buf[MAX];
    int pid, n;
    sk = socket(AF_INET, SOCK_STREAM, 0);
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = 0;
    if (bind(sk, (struct sockaddr *)&local, sizeof(local)) < 0)
    {
        printf("bind fails!\n");
        exit(1);
    }
    getsockname(sk, (struct sockaddr *)&local, &len);
    broadcastPortNumber(ntohs(local.sin_port));
    listen(sk, 5);
    while (1)
    {
        nsk = accept(sk, (struct sockaddr *)&remote, &rlen);
        pid = fork();
        if (pid < 0)
        {
            printf("Cannot fork!\n");
            exit(1);
        }
        else if (pid == 0)
        {
            close(sk);
            int action, accountNo;
            n = read(nsk, &action, sizeof(action));
            action = ntohl(action);
            n = read(nsk, &accountNo, sizeof(accountNo));
            accountNo = ntohl(accountNo);
            char ipaddress[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &remote.sin_addr.s_addr, ipaddress, INET_ADDRSTRLEN);
            printf("Service Requested from %s\n", ipaddress);
            char response[256];
            if (action == 1000)
            {
                readInfo(response, accountNo);
                send(nsk, response, strlen(response) + 1, 0);
            }
            else if (action == 1001)
            {
                float amount;
                n = read(nsk, &amount, sizeof(amount));
                float value = ntohf(amount);
                updateInfo(response, accountNo, value );
                send(nsk, response, strlen(response) + 1, 0);
            }
            close(nsk);
            exit(0);
        }
        close(nsk);
    }
}

void broadcastPortNumber(int portNumber)
{
   
    char PUTMSG[50];
    char buf[BUFMAX];
    int sk;
    sprintf(PUTMSG, "PUT CISBANK %d", portNumber);
    struct sockaddr_in remote;
    struct sockaddr_in remaddr;
    socklen_t addrlen = sizeof(remaddr);
    int recvlen;
    struct hostent *hp;
    sk = socket(AF_INET, SOCK_DGRAM, 0);
    remote.sin_family = AF_INET;
    remote.sin_addr.s_addr = inet_addr("255.255.255.255");
    remote.sin_port = ntohs(atoi("25446"));
    setsockopt(sk, SOL_SOCKET, SO_BROADCAST, (struct sockaddr *)&remote, sizeof(remote));
    sendto(sk, PUTMSG, strlen(PUTMSG) + 1, 0, (struct sockaddr *)&remote, sizeof(remote));
    recvlen = recvfrom(sk, buf, BUFMAX, 0, (struct sockaddr *)&remaddr, &addrlen);
    char ipaddress[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &remaddr.sin_addr.s_addr, ipaddress, INET_ADDRSTRLEN);
    printf("Registration %s from %s\n", buf, ipaddress);
    close(sk);
}
char *readInfo(char response[], int accountNo)
{
    FILE *infile;
    struct record input;
    infile = fopen("db18", "r");
    if (infile == NULL)
    {
        printf("Error opening file\n");
        exit(1);
    }

    int found = 0;
    while (fread(&input, sizeof(struct record), 1, infile))
    {
        if (input.acctnum == accountNo)
        {
            found = 1;
            break;
        }
    }
    if (found == 1)
    {
        sprintf(response, "%s %d %.1f", input.name, input.acctnum, input.value);
    }
    else
    {
        sprintf(response, "Invalid account number");
    }
    fclose(infile);
    return response;
}

char *updateInfo(char *response, int accountNo, float value)
{
    FILE *infile;
    struct record input;
    infile = fopen("db18", "rb+");
    if (infile == NULL)
    {
        fprintf(stderr, "\nError opening file\n");
        exit(1);
    }
    int found = 0;
    int index = -1;
    while (fread(&input, sizeof(struct record), 1, infile))
    {
        index = index + 1;
        if (input.acctnum == accountNo)
        {
            found = 1;
            if (lockf(fileno(infile), F_LOCK, sizeof(struct record)) == -1)
            {
                exit(1);
            }
            int pos = ftell(infile);
            fseek(infile, pos - (sizeof(struct record)), SEEK_SET);
            fread(&input, sizeof(struct record), 1, infile);
            input.value = input.value + value;
            fseek(infile, pos - (sizeof(struct record)), SEEK_SET);
            fwrite(&input, sizeof(struct record), 1, infile);
            if (lockf(fileno(infile), F_ULOCK, sizeof(struct record)) == -1)
            {
                exit(1);
            }
            break;
        }
    }
    if (found == 1)
    {
        sprintf(response, "%s %d %.1f", input.name, input.acctnum, input.value);
    }
    else
    {
        sprintf(response, "Invalid account number");
    }
    fclose(infile);
    return response;
}

float ntohf(float value)
{
    int temp = htonl(*(unsigned int *)&value);
    return *(float *)&temp;
}