#include "echo.h"

int isCmd (char *buf) {
    return !strcmp (buf,"ls") || !strcmp (buf,"pwd") 
        || !strcmp (buf,"cd") || !strcmp (buf,"mkdir");
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

int tailF (int fd) {
    int tailleF = 0;
    struct stat st;
    fstat (fd,&st);
    tailleF = (int)st.st_size;
    return tailleF;
}

void telechargeFile (int clientfd,char *cmd) {
    rio_t rio;
    int tailleF, nbPaquet,fd,tailCurrent=0, tailRest,n,nbPaquetRest;
    char message[MAX], tailleFF[MAX];
    struct timeval debut, fin;
    struct timezone tz;
    float taux;
    long long diff;
    Rio_readinitb(&rio, clientfd);
    
    printf("%d\n",clientfd);
    //cmd [strlen(cmd) - 1] = '\0';
    
    fd = open(cmd, O_CREAT | O_APPEND | O_RDWR , 0666);     
    if(fd < 0){
            printf("Erreur de créaton du fichier \n");
            exit(0);
    }

        tailCurrent = tailF(fd);
        //recuperer la taille du fichier envoyer par le serveur
        if (Rio_readlineb(&rio, tailleFF, MAX) > 0) {
            //la taille du fichier
            tailleF = atoi(tailleFF);             
        }
        printf("--%d\n",tailleF); 
        
        //la taille actuelle
        tailCurrent = tailF(fd);     

        if(tailleF == tailCurrent){
            printf("Le fichier existe déjà\n");
            Close(fd);
            exit(0);
        } else { 
            printf("Debut de telechargement d'un fichier de taille: %d\n", tailleF);
            if (tailCurrent !=0) {
                //se placer dans le fichier au bon endroit pour continuer le téléchargemnt ou il faut
                lseek(fd, tailCurrent-1, SEEK_SET);    
            }

            //on commence le téléchargement
            gettimeofday (&debut, &tz); 
            //la taille restante
            tailRest = tailleF - tailCurrent;
            //on calcul le nombre de paquet
            nbPaquet = tailRest/MAX;
            nbPaquetRest = tailRest - (nbPaquet*MAX);
            memset(message, 0, MAX);            

            //on télécharge les paquets
            for (int i = 0; i<nbPaquet; i++){         
                if((n = Rio_readn(clientfd, message, MAX))>0){
                    write(fd , message, n);          
                    tailCurrent = tailF(fd);
                    printf("téléchargement de %d paquets ...(%d / %d)\n", nbPaquet,tailCurrent, tailleF );
                }
            }
            //on récupère le nombre de paquet restant
            if(nbPaquetRest != 0){     
                memset(message, 0, MAX);
                if(( n =Rio_readn(clientfd, message, nbPaquetRest))>0){
                    write(fd , message, n);
                    tailCurrent = tailF(fd);
                    printf("téléchargement des %d bytes restants (%d / %d)\n", n ,tailCurrent, tailleF );
                }
            }    
                close(fd);
        }

        gettimeofday (&fin, &tz);
        diff = (fin.tv_sec - debut.tv_sec) * 1000000L + ( fin.tv_usec - debut.tv_usec); 
        n = tailRest;
        printf(" tranfer successfully complete\n");
        taux = n*0.001/diff*0.000001;
        printf("%d bytes received in %llu seconds (%f kbytes/s)\n", tailCurrent , diff, taux);
}
