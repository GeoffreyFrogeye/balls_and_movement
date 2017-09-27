/*
Projet-ISN

Fichier: jeuSDL.c

Contenu: Fonctions d'initialisation du jeu et de gestion des scores.

Actions: C'est ici que sont effectuées les tâches sauvegarde des scores sur la base MySql, de lancement de la SDL pour le jeu ou l'éditeur, ...

Bibliothèques utilisées: Bibliothèques standards, SDL, SDL_image, SDL_ttf, FMOD, GTK

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <winsock.h>
#include <mysql.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <fmod.h>
#include <gtk/gtk.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "IOoptions.h"
#include "IOmain.h"
#include "main.h"
#include "editeur.h"
#include "jeu.h"
#include "JeuSDL.h"

extern int TailleBloc, TailleBoule, TailleMissileH, TailleMissileW, BMusique, BSons;		//Lien vers les variables globales déclarées dans main.c
extern double Volume, Hauteur, Largeur;
extern InfoDeJeu infos;

int LancerJeu(gpointer *pData)
{
    /* Cette fonction va appeler les fonctions d'initialisations de la SDL et lancer le jeu ou l'éditeur */

    SDL_Renderer *pMoteurRendu = NULL;  //Pointeurs sur le moteur de rendu
    SDL_Window *pFenetre = NULL;        //Pointeur sur la fenêtre
    FMOD_CHANNEL *channelEnCours = NULL;    //Pour le contrôle des différents canaux audios

    Sprite images[50] = {{NULL}, {0,0}};   //Tableau des images (textures + positions)
    TTF_Font *polices[10] = {NULL};		//Tableau des polices

    Options *pOptions = NULL;	//Pointeur sur une structure Options

    FILE *pFichierErreur = fopen("ressources/ErreursLog.txt", "a");        //Pointeur sur le fichier d'erreurs

    SDL_Surface *surf = NULL;	//Pointeur sur une surface
    SDL_Texture *pEcranChargement = NULL;	//Pointeur sur une texture pour l'écran de chargement

    Animation anim[10];	//Tableau de structures Animation

    int erreur=0;	//Code d'erreur

    Joueur *pJoueur = (Joueur *)g_slist_nth_data((GSList*)pData, 6);	//On récupère le pointeur vers la structure Joueur dans la liste chaînée
    Sons *pSons = (Sons*)g_slist_nth_data((GSList*)pData, 4);		//De même avec celui vers la structure Sons
    FMOD_SYSTEM *pMoteurSon = (FMOD_SYSTEM *)g_slist_nth_data((GSList*)pData, 3);	//De même avec celui vers la structure FMOD_SYSTEM

    if(pFichierErreur == NULL)	//Vérification
    {
        exit(EXIT_FAILURE);
    }

    /* On lit les options et on remplit la structure */
    pOptions = DefinirOptions();

    Initialisation(&pMoteurRendu, pFichierErreur, &pFenetre, pOptions);     //Initialisation des principaux éléments (SDL, fenêtre, moteur de rendu)

    FMOD_System_GetChannel(pMoteurSon, M_MENU, &channelEnCours);	//On met en pause la musique du menu
    FMOD_Channel_SetPaused(channelEnCours, true);

    if(BMusique)	//S'il y a de la musique
    {
        FMOD_System_PlaySound(pMoteurSon, M_LOAD, pSons->music[M_LOAD], true, NULL);        // On lit la musique de chargement
        FMOD_System_GetChannel(pMoteurSon, M_LOAD, &channelEnCours);
        FMOD_Channel_SetVolume(channelEnCours, (float)(Volume/100.0));
        FMOD_Channel_SetPaused(channelEnCours, false);
    }

    /* On charge l'image de chargement et on vérifie */
    surf = IMG_Load("ressources/img/load.png");

    if (surf == NULL)
    {
        fprintf(pFichierErreur, "Erreur: impossible d'ouvrir le fichier ressources/img/load.png");
        exit(EXIT_FAILURE);
    }

    /* On transforme la surface en texture pour l'affichage et on libère la mémoire occupée par la surface */
    pEcranChargement = SDL_CreateTextureFromSurface(pMoteurRendu, surf);
    SDL_FreeSurface(surf);

    SDL_ShowCursor(false);	//On masque le curseur pendant le jeu (on affichera un curseur personnalisé dans l'éditeur)

    /* On efface l'écran et on colle l'image de chargement */
    SDL_SetRenderDrawColor(pMoteurRendu, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(pMoteurRendu);
    SDL_RenderCopy(pMoteurRendu, pEcranChargement, NULL, NULL);
    SDL_RenderPresent(pMoteurRendu);

    SDL_EventState(SDL_TEXTEDITING, SDL_DISABLE);   //Désactivation des événements dont on a pas besoin.
    SDL_EventState(SDL_TEXTINPUT, SDL_DISABLE);

    SDL_DisableScreenSaver();       //Désactivation de l'écran de veille.

    erreur = Chargements(images, pMoteurRendu, polices, anim);	//On charge tout !

    /* Traitement des éventuelles erreurs */
    if(erreur == 1)
    {
        fprintf(pFichierErreur, "Erreur lors du chargement des images. Veuillez vérifier ressources\\img\\... \n");
        exit(EXIT_FAILURE);
    }
    else if (erreur == 2)
    {
        fprintf(pFichierErreur, "Erreur lors du chargement des polices. Veuillez vérifier ressources\\fonts\\... \n");
        exit(EXIT_FAILURE);
    }
    else if (erreur == 3)
    {
        fprintf(pFichierErreur, "Erreur lors du chargement des animations. Veuillez vérifier ressources\\anim\\... \n");
        exit(EXIT_FAILURE);
    }

    if (BMusique)
    {
        FMOD_System_GetChannel(pMoteurSon, M_LOAD, &channelEnCours);	//On arrête la musique du chargement
        FMOD_Channel_SetPaused(channelEnCours, true);

        FMOD_Sound_SetLoopCount(pSons->music[M_JEU], -1);      // On active la lecture en boucle

        FMOD_System_PlaySound(pMoteurSon, M_JEU, pSons->music[M_JEU], true, NULL);        // On lit la musique du jeu
        FMOD_System_GetChannel(pMoteurSon, M_JEU, &channelEnCours);
        FMOD_Channel_SetVolume(channelEnCours, (float)(Volume/100.0));
        FMOD_Channel_SetPaused(channelEnCours, false);
    }

    /* On regarde si on a appelé la fonction en mode jeu ou en mode éditeur */
    if (pJoueur->mode == MODE_CAMPAGNE)
    {
        InitialiserInfos(pOptions, pJoueur);	//On définit les infos sur la partie en cours
        erreur = BouclePrincipale(pJoueur, images, anim, pMoteurRendu, pMoteurSon, pSons, polices);    //Boucle du jeu

        if(erreur == JEU_FIN_ERREUR_CHARGEMENT)
        {
            MessageInformations("Erreur lors du chargement d'un niveau, consultez le fichier erreurs.txt", polices, pMoteurRendu, NULL);
        }

        SauverMySql(pJoueur);	//On sauvegarde l'avancée du joueur
    }
    else if(pJoueur->mode == MODE_PERSO)
    {
        InitialiserInfos(pOptions, pJoueur);	//On définit les infos sur la partie en cours
        erreur = BouclePrincipale(pJoueur, images, anim, pMoteurRendu, pMoteurSon, pSons, polices);    //Boucle du jeu

        if(erreur == JEU_FIN_ERREUR_CHARGEMENT)
        {
            MessageInformations("Erreur lors du chargement d'un niveau, consultez le fichier erreurs.txt", polices, pMoteurRendu, NULL);
        }
    }
    else if (pJoueur->mode == MODE_EDITEUR)
    {
        erreur = Editeur(pMoteurRendu, images, pMoteurSon, pSons, polices, pJoueur);	//On lance la boucle de l'éditeur

        if(erreur == JEU_FIN_ERREUR_CHARGEMENT)
        {
            MessageInformations("Erreur lors du chargement d'un niveau, consultez le fichier erreurs.txt", polices, pMoteurRendu, NULL);
        }
    }

    /* Libération de la mémoire */
    LibererMemoire(pMoteurRendu, images, anim, polices, pFenetre, pOptions);

    fclose(pFichierErreur);	//On ferme le fichier d'erreur

    return 0;
}

void LibererMemoire(SDL_Renderer *pMoteurRendu, Sprite images[], Animation anim[], TTF_Font *polices[], SDL_Window *pFenetre, Options *pOptions)
{
    int i=0, j=0;	//Compteurs

    /* On détruit le moteur de rendu et la fenêtre */
    SDL_DestroyRenderer(pMoteurRendu);
    SDL_DestroyWindow(pFenetre);

    /* On detruit les textures des animations une par une, animation par animation */
    for(j=0, i=0; i<10; i++)
    {
        while(anim[i].img[j] !=NULL)
        {
            SDL_DestroyTexture(anim[i].img[j]);
            j++;
        }

        j=0;
    }

    /* On détruit les textures des Sprites une par une, Sprite par Sprite */
    for(j=0, i=0; i<50; i++)
    {
        while(images[i].pTextures[j] != NULL)
        {
            SDL_DestroyTexture(images[i].pTextures[j]);
            j++;
        }

        j=0;
    }

    /* On ferme les polices */
    for(i=0; i<=POLICE_SNICKY_GRAND; i++)
    {
        TTF_CloseFont(polices[i]);
    }

    free(pOptions);	//On libère la structure Options

    /* On quitte la SDL et ses extensions */
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

int SauverMySql(Joueur *pJoueur)
{
    /* On va sauvegarder l'avancement du joueur niveau maximum atteint, score, niveaux et scores des niveaux précédents */

    MYSQL *mysql = mysql_init(NULL);	//On initialise le gestionnaire de connexion
    char requete[256] = "";		//Chaîne pour les requêtes MySql

    mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP, "default");	//On charge les options de connexion par défaut

    /* Si on a réussi à se connecter à la base */
    if(mysql_real_connect(mysql, "mysql1.alwaysdata.com", "89504_beaussart", "beaussart62", "ballsandmovement_players", 3306, NULL, 0))
    {
        /* On crée la requête */
        sprintf(requete, "UPDATE projetz SET niveau_max = %d, autre = '%s' WHERE pseudo = '%s'", infos.niveau, pJoueur->autre, pJoueur->pseudo);

        mysql_query(mysql, requete);	//On lance la requête

        mysql_close(mysql);	//On ferme la connexion
    }

    return 0;
}

void InitialiserInfos(Options *pOptions, Joueur *pJoueur)
{
    /* On définit les valeurs pour la partie qui va commencer à partir des options et des informations issues de la base MySql */
    if(pJoueur->mode == MODE_CAMPAGNE)
    {
        infos.niveau = pJoueur->niveau_max;
    }
    else	//Mode perso
    {
        infos.niveau = 1;
    }

    infos.score = 1000;
    infos.vies = pOptions->vies;
    infos.viesInitiales = pOptions->vies;
    infos.compteurTemps = 0;
    infos.bonus &= AUCUN_BONUS;
}

//Fin du fichier JeuSDL.c
