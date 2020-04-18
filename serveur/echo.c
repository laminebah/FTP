
/*
 * echo - read and echo text lines until client closes connection
 */
#include "csapp.h"
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

#define MAX 500
#define N 3

void handler (int sig) {
    switch (sig) {
        case SIGINT:
            kill (0,SIGINT);
            exit (0);
            break;
        case SIGCHLD:
            while(waitpid(-1,NULL, WNOHANG) > 0);
            break;
    }
}

int tailF (int fd) {
    int tailleF = 0;
    struct stat st;
    fstat (fd,&st);
    tailleF = (int)st.st_size;
    return tailleF;
}


int isCmd (char *buf) {
    return !strcmp (buf,"ls") || !strcmp (buf,"pwd")  || !strcmp (buf,"cd") 
        || !strcmp (buf,"mkdir")  || !strcmp (buf,"rm") || !strcmp (buf,"bye");
}

int decoupe (char *buf,char ** args)  {
    int i = 0;
     char *mot;
    if (!isCmd(buf)){   
        mot = strtok(buf, " ");          
    
        while ( mot != NULL && i<N ) {
            strcpy(args[i], mot);
            mot = strtok(NULL, " ");
            i++;
        }
     }
    return i;
}

void echo (int slaveToClient,char **args,int fd) {
    int  tailleF,i=0, nbPaquet,size_rcp = 0, tailRest =0,n = 0,nbPaquetRest;
    char message[MAX];
    rio_t rio;
    Rio_readinitb(&rio, slaveToClient);
    if (!strcmp (args [0],"get") ||!strcmp (args [0],"Get") || !strcmp (args [0],"GET")) {
            //on calcule la taille du fichier
            tailleF = tailF (fd);                 
            //on envoie la taille du fichier au client
            memset(message,0,MAX);
            sprintf(message, " %d", tailleF);        
            n = rio_writen(slaveToClient,message,strlen(message));
            printf("%d:%s\n",n,message);  
            if(size_rcp){
                //on se réplace au bon endroit dans le fichier pour pouvoir continuer l'envoie après une panne du serveur
                printf("Envoie du fichier à partir du %d eme octet\n", size_rcp );
                lseek(fd, size_rcp-1, SEEK_SET);         
            }                                            

            tailRest = tailleF -  size_rcp ;
                
            //Transfert du fichier par parquet
            nbPaquet=  tailRest/MAX;                 
            nbPaquetRest  = tailRest - (nbPaquet*MAX);
            memset (message,0,MAX);
            //le premier paquet est envoyé
            Rio_writen(slaveToClient, message , MAX);    

            for (i = 0; i < nbPaquet; i++){
                //Transfert des autres paquets
                sleep(1);        
                memset(message, 0, MAX);
                n = Rio_readn(fd, message, MAX);         
                Rio_writen(slaveToClient, message, n);            
            }
            //envoie des paquets restants après une panne
            if(nbPaquetRest != 0){
                memset(message , 0, MAX);
                if((n = Rio_readn (fd, message , nbPaquetRest)) > 0)
                    Rio_writen(slaveToClient, message ,n);
            }
    }
}

void ls (int slaveToClient) {
    struct dirent *dir;
    DIR *d = opendir (".");
    char resultat [MAXLINE],chaine [MAX] = "",resultats [MAXLINE * 5] = "";
    if (d) {
        while ((dir = readdir (d)) != NULL) {
            sprintf (resultat,"%s",dir -> d_name);
            strcpy (chaine,resultat);
            if (strcmp (chaine,"..") && strcmp (chaine,".")) {
                strcat (resultats,chaine);
                strcat (resultats," ");
            }
        }
        //printf("%s ", resultats);
        closedir (d);
    }
    Rio_writen (slaveToClient,resultats,MAXLINE*2);
    printf("\n");
}

void mKdir (char *cmd) {
    char path [MAX] = "./";
    strcat (path,cmd);
    if (mkdir (path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) {
        printf("Erreur : création impossible du dossier %s\n",cmd);
    }
}

void pwD (int slaveToClient) {
    char path[MAXLINE];
    char *ptr;
    
    memset(path, 0, MAXLINE);
    if ((ptr = getcwd(path, MAXLINE)) < 0){
        fprintf(stderr, "%s\n", strerror(errno));
    }
    else{
        Rio_writen (slaveToClient,path,MAXLINE);
    }
}

void cD (char *cmd) {
    if (chdir (cmd) < 0){
        fprintf(stderr, "Impossible de se déplacer dans le dossier %s\n", strerror(errno));
    }
}

void rm (char **cmd,int n) {
    struct stat info;
    int status;

    if (n == 2) {
        //suppression d'un fichier
        status = stat(cmd[1], &info);
        (void) status; 
        if (S_ISREG(info.st_mode) ){
            if (remove(cmd [1]) < 0){
            printf("ERREUR : suppression impossible. \n");
            }
        }
        else {
            printf("ATTENTION : utiliser rm -r pour un dossier ! \n");
        }
    } else if (n == 3 && !strcmp (cmd [1],"-r")) {
        printf("%s\n", cmd [2]);
    } else {
        printf("rm : opérande manquant\n");
    }
}