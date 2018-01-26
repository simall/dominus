#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "GPIO.h"
#include "serveur.h"

SOCKET sock;
/* the index for the array */
int actual = 0;
/* an array for all clients */
SOCKET clients[MAX_CLIENTS];

static void signals_handler(int signal_number)
{
   printf("Signal catched.\n");
   clear_clients(clients, actual);
   end_connection(sock);
   exit(EXIT_SUCCESS);
}

static void interpret(const char *buffer)
{
   int pin;

   if(strlen(buffer) != 3)
   {
      perror("interpret()");
   }
   else
   {
      pin = ((int)(buffer[0])-48)*10+(int)(buffer[1])-48;

      if(buffer[2] == '0')
      {
         /* switch off the LED */
         GPIOWrite(pin, 0);
         GPIOUnexport(pin);
         printf("GPIO %d off...\n", pin);
      }
      else if(buffer[2] == '1')
      {
         /* switch on the LED */
         GPIOExport(pin);
         GPIODirection(pin, OUT);

         GPIOWrite(pin, 30);
         printf("GPIO %d on...\n", pin);
      }
   }
}

static void start(void)
{
   sock = init_connection();
   char buffer[BUF_SIZE];
   int max = sock;
   

   fd_set rdfs;

   // signals handler
   struct sigaction action;
   action.sa_handler = signals_handler;
   sigemptyset(& (action.sa_mask));
   action.sa_flags = 0;
   sigaction(SIGINT, & action, NULL);

   while(1)
   {
      int i = 0;

      /* add the connection socket */
      FD_SET(sock, &rdfs);

      /* add socket of each client */
      for(i = 0; i < actual; i++)
      {
         FD_SET(clients[i], &rdfs);
      }

      select(max + 1, &rdfs, NULL, NULL, NULL) == -1;

      if(FD_ISSET(sock, &rdfs))
      {
         /* new client */
         SOCKADDR_IN csin = { 0 };
         size_t sinsize = sizeof csin;
         int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
         if(csock == SOCKET_ERROR)
         {
            perror("accept()");
            continue;
         }

         /* what is the new maximum fd ? */
         max = csock > max ? csock : max;

         FD_SET(csock, &rdfs);

         SOCKET c = csock;
         clients[actual] = c;
         actual++;
      }
      else
      {
         int i = 0;
         for(i = 0; i < actual; i++)
         {
            /* a client is sending a command */
            if(FD_ISSET(clients[i], &rdfs))
            {
               SOCKET client = clients[i];
               int c = read_client(clients[i], buffer);
               /* client disconnected */
               if(c == 0)
               {
                  closesocket(clients[i]);
                  remove_client(clients, i, &actual);
               }
               else
               {
                  /* read the command and remove the client */
                  interpret(buffer);
               }
               break;
            }
         }
      }
   }
}

static void clear_clients(SOCKET *clients, int actual)
{
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      closesocket(clients[i]);
   }
}

static void remove_client(SOCKET *clients, int to_remove, int *actual)
{
   /* we remove the client in the array */
   memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(SOCKET));
   /* number client - 1 */
   (*actual)--;
}

static int init_connection(void)
{
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = { 0 };

   if(sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
   {
      perror("bind()");
      exit(errno);
   }

   if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
   {
      perror("listen()");
      exit(errno);
   }

   return sock;
}

static void end_connection(int sock)
{
   closesocket(sock);
}

static int read_client(SOCKET sock, char *buffer)
{
   int n = 0;

   if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
   {
      perror("recv()");
      /* if recv error we disonnect the client */
      n = 0;
   }

   buffer[n] = 0;

   return n;
}

static void write_client(SOCKET sock, const char *buffer)
{
   if(send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}

int main(int argc, char **argv)
{

   start();

   return EXIT_SUCCESS;
}