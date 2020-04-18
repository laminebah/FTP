/*
 * echoclient.c - An echo client
 */
#include "csapp.h"
#include <string.h>
#define MAX_NAME_LEN 256
#define N 3
#define MAX 500

/**-Cette fonction vérifie si buf est une commande
   -Donnees : -Une chaine de caractère (nom de la commande)
   -Resultat : 1 si buf est une commande autre que get sion 0
*****/
int isCmd (char *buf);
/**-Cette fonction découpe la chaine qu'on lui fournie en fonction de " "
   -Donnees : - Une chaine (autre que les commandes ) de caractère à découper
              - un tableau de chaine qui contiendra les différentes chaines obtenues
   -Resultat : le nombre de sous chaine obtenue
*****/
int decoupe (char *buf,char ** args);
/**-Cette fonction est le moteur du téléchargement du fichier de ce coté client 
        C'est par elle que le client réceptionne le résultat de sa requete.Et ce résultat 
        est envoyé par un serveur esclave
    -Données : - un descripteur pour la communication entre le client et l'esclave
               - un nom de fichier dans le quel sera récopié le résultat
               - une structure rio
    -Résultat : Cete fonction ne renvoie rien

*****/
void telechargeFile (int clientfd,char *cmd);


int main(int argc, char **argv)
{   
    int clientfd,port;
    char *host,bufPort [MAXLINE],buf[MAXLINE],*hostCliSlav,**args;
    rio_t rio;


    struct sockaddr_in client;
    socklen_t clientlen;
    if (argc != 4) {
        fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
        exit(0);
    }
    host = argv[1];
    hostCliSlav = argv [2];
    port = atoi (argv [3]);

    /*
     * Note that the 'host' can be a name or an IP address.
     * If necessary, Open_clientfd will perform the name resolution
     * to obtain the IP address.
     */
    //il se connecte au maitre étant client
    clientfd = Open_clientfd(host, 2121);
    clientlen = (socklen_t) sizeof(client);
    /*
     * At this stage, the connection is established between the client
     * and the server OS ... but it is possible that the server application
     * has not yet called "Accept" for this connection
     */
    getpeername (clientfd,(SA *)&client,&clientlen);
    printf("client connected to server OS and to port %d\n",ntohs(client.sin_port));
    Rio_readinitb(&rio, clientfd);
    sprintf (bufPort,"%d ",port);
    // le client envoie le port et le host pour qu'un esclave puisse lui envoyer le résultat de sa requete
    Rio_writen (clientfd,bufPort,strlen(bufPort));
    strcat (hostCliSlav," ");
    Rio_writen (clientfd,hostCliSlav,strlen(hostCliSlav));
    close (clientfd);
    
    /***************Connexion entre le client et le slave ici le client est le maitre du slave***************************/
    int listenfdS, connfdS;
    socklen_t clientlenS;
    struct sockaddr_in clientaddrS;
    char client_ip_stringS[INET_ADDRSTRLEN],slave_hostname[MAX_NAME_LEN];
    rio_t rio_sc;
    clientlenS = (socklen_t)sizeof(clientaddrS);

    listenfdS = Open_listenfd(port);
        
    connfdS = Accept(listenfdS, (SA *)&clientaddrS, &clientlenS);

        /* determine the name of the client */
    Getnameinfo((SA *) &clientaddrS, clientlenS,
    slave_hostname, MAX_NAME_LEN, 0, 0, 0);
            
            /* determine the textual representation of the client's IP address */
    Inet_ntop(AF_INET, &clientaddrS.sin_addr, client_ip_stringS,
                      INET_ADDRSTRLEN);
            
    printf("Communication établie entre le client et le serveur esclave %s (%s)\n", slave_hostname,
                   client_ip_stringS);
    Rio_readinitb(&rio_sc, connfdS);
    args = malloc (sizeof (char *) * N);
    for (int i = 0; i < N; i ++)
        args [i] = malloc (sizeof(char)*MAXLINE);
    while (1) {
      printf("ftp> ");
      Fgets (buf, MAXLINE, stdin);
      Rio_writen (connfdS,buf,strlen(buf));
      int nb = decoupe (buf,args);
      memset (buf,0,MAXLINE);
      if (nb == 2 && (!strcmp (args [0],"get") || !strcmp (args [0],"Get") || !strcmp (args [0],"GET"))) {
          Rio_readlineb (&rio_sc,buf,MAX);
          args [1] [strlen(args[1]) - 1] = '\0';
          if (!strcmp (buf,"ERROR")) {
             printf("Fichier %s manquant\n",args [1]);
          } else {
            telechargeFile (connfdS,args [1]); 
          }
         
      } else {
          if (nb < 2) {
            args [0][strlen(args [0]) - 1] = '\0';
          } else {
            args [1][strlen(args [1]) - 1] = '\0';
          }
          if (!strcmp (args [0],"bye")) {
                printf("A bientot\n");
                exit (0);
          } else if (!strcmp (args [0],"ls")) {
              char resultat [MAX];
              if (Rio_readlineb (&rio_sc,resultat,MAX)) {
                  resultat [strlen (resultat)] = '\0';
                  printf("%s", resultat);
              }
            printf("\n");
          } else if (!strcmp (args [0],"pwd")) {
              char resultat [MAXLINE] = "";
              if (Rio_readlineb(&rio_sc,resultat,MAXLINE)) {
                printf("%s",resultat);
              }
              printf("\n");
          }
      }
      memset (buf,0,MAXLINE);
    }
    free (args);
    close (connfdS);
    exit(0);
}
