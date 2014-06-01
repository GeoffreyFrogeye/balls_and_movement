/*
Projet-ISN

Fichier: IOoptions.c

Contenu: Fonctions d'entr�es sorties (chargements, sauvegardes de fichiers, ...)

Actions: C'est ici que se trouve les fonctions qui lisent ou �crivent sur le disque et qui concernent les options.

Biblioth�ques utilis�es: Biblioth�ques standards, SDL, SDL_image, SDL_ttf, FMOD, GTK

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <fmod.h>
#include <gtk/gtk.h>
#include "IOoptions.h"
#include "IOmain.h"
#include "main.h"

char** LectureOptions()
{
    FILE *pFichierOptions = NULL;       //Pointeur sur le fichier d'options
    int i=0, j=1;	//Compteurs
    char caractere=0;
    char **options = (char**)malloc(50 * sizeof(char*));     //Variable pour retenir le caract�re actuel et tableau de cha�nes pour la lecture

    pFichierOptions = fopen("ressources/settings.ini", "r");       //Ouverture du fichier en mode lecture

    if(options != NULL)
    {
        for (i=0; i<50; i++)
        {
            options[i] = (char*)malloc(50 * sizeof(char));
            if(options[i] == NULL)
            {
                return NULL;
            }
        }
    }
    else
    {
        return NULL;
    }

    if(pFichierOptions == NULL) //On v�rifie si le fichier existe
    {
        return NULL;
    }

    i=0;
    do
    {
        do
        {
            caractere = (char)fgetc(pFichierOptions); //On lit un caract�re
            options[j][i] = caractere;  //On le met dans le tableau
            i++;    //Caract�re suivant dans le tableau
        }
        while (caractere != '\n' && caractere != EOF);   //Tant qu'il n'y a pas de retour � la ligne ou de fin de fichier

        options[j][i-1] = '\0';     //On place le caract�re de fin de cha�ne � la place du retour � la ligne
        j++;      //Ligne suivante dans le tableau
        i=0;	//On revient au d�but de la ligne dans le tableau
    }
    while (caractere != EOF);        //Tant qu'on est pas � la fin du fichier

    fclose(pFichierOptions);        //On ferme le fichier

    options[0][0]= (char)j-1;	//Sur le premier octet on met le nombre de lignes total

    return options;
}

int ValiderChangement(char **options)
{
    int i=0;	//Compteur
    FILE *pFichierOptions = NULL;	//Pointeur sur le fichier des options

    pFichierOptions = fopen("ressources/settings.ini", "w");      //On ouvre le fichier et on l'efface

    if(pFichierOptions == NULL)     //On v�rifie l'ouverture
    {
        return -1;
    }

    for(i=1; i<options[0][0] ; i++)
    {
        fputs(options[i], pFichierOptions);   //On �crit chaque ligne
        fputc('\n', pFichierOptions);       //On place un retour � la ligne
    }

    fclose(pFichierOptions);        //On ferme le fichier

    for (i=0; i<50; i++)
    {
        free(options[i]);
    }

    free(options);

    return 0;
}

Options* DecouperOptions(char** options)
{
    /* Cette fonction alloue une structure Options qu'elle remplit avec le tableau lu par la fonction pr�c�dente */

    int i=0;
    Options *pOptions = (Options*)malloc(sizeof(Options));	//On alloue une structure Options
    char *c=NULL;	//Pointeur sur un caract�re pour faire des recherches dans les cha�nes

    if(pOptions == NULL)	//On v�rifie l'allocation de la structure
    {
        return NULL;
    }

    c = strstr(options[1], "=");	//On cherche le '=', on place son adresse dans le pointeur
    pOptions->musique = strtol((c+1), NULL, 10);	//On convertit en nombre d�cimal le contenu de la cha�ne � partir du caract�re suivant le '=' d'o� (c+1)

    c = strstr(options[2], "=");
    pOptions->sons = strtol((c+1), NULL, 10);

    c = strstr(options[3], "=");
    pOptions->largeur = strtol((c+1), NULL, 10);

    c = strstr(options[3], "x");	//Ici c'est le 'x' entre les deux valeurs de la r�solution que l'on recherche
    pOptions->hauteur = strtol((c+1), NULL, 10);

    c = strstr(options[4], "=");
    pOptions->vies = (char)strtol((c+1), NULL, 10);

    c = strstr(options[5], "=");
    pOptions->volume = (float)strtod((c+1), NULL);

    c = strstr(options[6], "=");
    pOptions->fullScreen = strtol((c+1), NULL, 10);

    pOptions->nbLigne = options[0][0];	//On stocke le nombre de ligne dans la structure �galement

    for(i=0; i<50; i++)
    {
        free(options[i]);
    }
    free(options);

    return pOptions;	//On retourne l'adresse de la structure qu'on a allou�e
}

Options* DefinirOptions()
{
    char **tmp=LectureOptions();

    if(tmp != NULL)
    {
        return DecouperOptions(tmp);
    }

    return NULL;
}

//Fin du fichier IOoptions.c
