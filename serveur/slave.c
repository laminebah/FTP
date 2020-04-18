#include "csapp.h"

#define MAX_NAME_LEN 256
#define N 4
#define MAX 500

/**-Cette fonction est le coeur du transfert du fichier vers le client
   -Donnees : -Elle prend le descripteur du client pour établir la communivation entre le client et le slave
              -un liste d'arguments contenant le port du client,host du client ,puis la commande  à exécuter :
                -Get : pour le transfert d'un fichier
                -ls,cd,rm,des commandes
   -Resultat : elle ne renvoie aucune valeur
*****/
void echo(int SlaveToClient,char **args,int fd);
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
int decoupe (char *buf,char **args);

void ls (int slaveToClient);

void mKdir (char *cmd);

void pwD (int slaveToClient);

void cD (char *cmd);

void rm (char **cmd,int n);


int main (int argc,char **argv) {

    /********Communication entre le maitre et le slave*******/
	int listenfd, connfd, port,portClient;
    socklen_t master;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN], **args,client_hostname[MAX_NAME_LEN],*hostC;
    
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    args = malloc (sizeof (char *) * N);
    for (int i = 0; i < N; i ++)
        args [i] = malloc (sizeof(char)*MAXLINE);

    port = atoi(argv[1]);
    master = (socklen_t)sizeof (clientaddr);
    listenfd = Open_listenfd(port);
    while (1) {


        connfd = Accept(listenfd, (SA *)&clientaddr, &master);

                /* determine the name of the client */
        Getnameinfo((SA *) &clientaddr, master,
                            client_hostname, MAX_NAME_LEN, 0, 0, 0);
                
                /* determine the textual representation of the client's IP address */
        Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                          INET_ADDRSTRLEN);
                
        printf("Slave connected to %s (%s)\n", client_hostname,
                       client_ip_string);
        
        size_t n;
        char buf[MAXLINE];
        rio_t rio;
        Rio_readinitb(&rio, connfd);
        //récupère la requete du client y compris des informations concernant le client
        n = Rio_readlineb(&rio, buf, MAXLINE);
        (void) n;
        //buff contient : N° port client , le Host du client puis la requete du client
        n = decoupe (buf,args);
        (void) n;
        portClient = atoi (args [0]);//Numéro de port du client
        hostC = args [1];//le hostname du client
        Close (connfd);


        /**************Le serveur communique avec le client pour le transfert du fichier************/
        printf("Tentative de communication entre le slave et le client du port %d\n",portClient);
        int slaveToClient = Open_clientfd (hostC,portClient);
        rio_t rioC;
        int fd;
        Rio_readinitb(&rioC,slaveToClient);
        memset (buf,0,MAXLINE);
        while (Rio_readlineb (&rioC, buf, MAXLINE)) {
            n = decoupe (buf,args);
            args [1] [strlen(args[1]) - 1] = '\0';
            if (!strcmp (args [0],"GET")||!strcmp (args [0],"get") || !strcmp (args [0],"Get")) {
                fd = open(args[1], O_RDONLY, 0);
                if (fd < 0) {
                    printf("Erreur ouverture du fichier %s:%ld\n",args [1],strlen(args[1]));
                    char err [MAXLINE] = "ERROR";
                    Rio_writen (slaveToClient,err,MAXLINE);
                }  else {
                    char err [MAXLINE] = "SUCCESS";
                    Rio_writen (slaveToClient,err,MAXLINE);
                    echo (slaveToClient,args,fd);
                }
                memset (buf,0,MAXLINE);
                Close (fd);
            } else {
                if (n < 2) {
                    args [0][strlen(args [0]) - 1] = '\0';
                } else {
                    //args [1][strlen(args [1]) - 1] = '\0';
                }
                if (!strcmp (args [0],"bye")) {
                    printf("Deconnextion du client\n");
                } else if (!strcmp (args [0],"ls")) {
                    ls (slaveToClient);
                }  else if (!strcmp (args [0],"rm")) {
                    //printf("A complémenter %s\n",args [0]);
                    rm (args,n);
                } else if (!strcmp (args [0],"mkdir")) {
                    if (n < 2) {
                        strcpy (args[1],"Nouveau dossier");
                    }
                    mKdir (args [1]);
                } else if (!strcmp (args [0],"cd")) {
                     cD (args [1]);
                } else if (!strcmp (args [0],"pwd")) {
                     pwD (slaveToClient);
                }
            }
        }
        Close (slaveToClient);
    }

    free (args);
    exit(0);
}
