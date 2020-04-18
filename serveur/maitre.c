/*
 * echoserveri.c - An iterative echo server
 */

#include "csapp.h"
#include <string.h>

#define MAX_NAME_LEN 256
#define NPROC 2

/**fonction de traitement des zombies et du signal SIGINT**/
void handler (int sig);

int  k = 0;

/* 
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */
int main(int argc, char **argv) {
    int listenfd, connfd, port,tPort [30],slave;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN],buf [MAXLINE];
    
    Signal (SIGCHLD,handler);
    Signal (SIGINT,handler);

    
    if (argc < 2) {
        fprintf(stderr, "usage: (insuffisants pour les slaves) %s <port>\n", argv[0]);
        exit(0);
    }
   
    //port de communication entre le client et le serveur
    port = 2121;
    //Liste des esclaves disponibles vers les quels on redirige la connexion
    printf("***************Esclaves***************\n");
    for (int i = 0; i < argc - 1 ; i ++) {
        tPort [i] = atoi (argv [i + 1]);
        if (tPort [i] == port) {
            fprintf(stderr,"le port %d ne peut pas etre utiliser pour connecter le maitre aux slaves\n",tPort [i]);
            exit(0);
        }
        printf("\tEsclaves %d\n",tPort [i]);
    }
    clientlen = (socklen_t)sizeof(clientaddr);

    listenfd = Open_listenfd(port);
    for (int i = 0; i < NPROC ; i ++) {

        if (Fork () == 0) {
            while (1) {
                
                /***Communication client vers le serveur******/
                connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
                //Rio_writen (slave,"bonjour",strlen("bonjour"));
                getsockname (connfd,(SA *)&clientaddr,&clientlen);
                /* determine the name of the client */
                Getnameinfo((SA *) &clientaddr, clientlen,
                            client_hostname, MAX_NAME_LEN, 0, 0, 0);
                
                /* determine the textual representation of the client's IP address */
                Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                          INET_ADDRSTRLEN);
                
                printf("Connexion du nouveau client %s (%s) : %d\n", client_hostname,
                                   client_ip_string,ntohs (clientaddr.sin_port));
                size_t n;
                rio_t rio;
                Rio_readinitb(&rio, connfd);
                printf("Tentative de connextion du Nouveau Client vers l'esclave du port %d\n",tPort [k]);
                slave = Open_clientfd ("localhost",tPort [k]);
                //réception des données et la requete du client : N°Port ,Host Name client,requete du client
                while ((n = Rio_readlineb(&rio, buf, MAXLINE))) {
                    printf("Transfert de la requete du Nouveau client à l'esclave du port %d\n",tPort [k]);
                    n = rio_writen (slave,buf,n-1);
                }
                Close (slave);
                Close (connfd);

            }
        }
        k += 1;
    }
    //le père qui termine les processus fils
    for (int i = 0; i < NPROC ; i ++)
        wait (NULL);
    exit(0);
}

