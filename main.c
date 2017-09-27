/*
Projet-ISN

Fichier: main.c

Contenu: Fonction main() et autres fonctions générales ( ex: Arrondir() )

Actions: C'est ici que le programme commence et ... se termine.

Bibliothèques utilisées: Bibliothèques standards, SDL, SDL_image, SDL_ttf, FMOD, GTK

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <math.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <fmod.h>
#include "SDL2/SDL2_gfxPrimitives.h"
#include "IOoptions.h"
#include "IOmain.h"
#include "main.h"
#include "interfaceMain.h"
#include "callback.h"
#include "jeu.h"
#include "JeuSDL.h"
#include "IOcredits.h"

/* Variables globales accessibles depuis toutes les fonctions de tous les fichiers (si elles sont liées au début des fichiers en question) */
int TailleBloc, TailleBoule, TailleMissileH, TailleMissileW, BMusique, BSons;
double Volume, Hauteur, Largeur;
InfoDeJeu infos;

/* Fonction principale */
int main(int argc, char *argv[])
{
    /* Déclaration des widgets */
    GtkWidget *pWindow, *pTitre, *pBoutonConnexion, *pBoutonJouer, *pBoutonOptions, *pBoutonEditeur, *pBoutonCredits, *pBoutonQuitter, *pZoneDessin, *pZoneDessinV1, *pZoneDessinV2;
    GSList *pListeElements=NULL;	//Liste chaînée où l'on va mettre les éléments dont ont aura besoin plus tard
    GdkImages pix;	//Pointeur sur une structure de 4 'GdkPixBuf*'

    Options *pOptions = NULL;	//Pointeur sur une structure 'Options'
    Joueur joueur;	//Structure de type 'Joueur'

    FMOD_SYSTEM *pMoteurSon=NULL;     //Pointeur sur le moteur sonore
    FMOD_CHANNEL *channelEnCours=NULL;    //Pointeur pour le contrôle des différents canaux audios
    Sons sons = {{NULL}, {NULL}};     //Structure des sons

    FILE *pFichierErreur = fopen("ressources/ErreursLog.txt", "a");       //Création du fichier d'erreur.
    if (pFichierErreur == NULL)
    {
        exit(EXIT_FAILURE);	//On quitte si on a pas réussi à l'ouvrir ou à le créer
    }

    /* On lit les options */
    pOptions = DefinirOptions();
    if(pOptions == NULL)
    {
        fprintf(pFichierErreur, "Erreur de lecture des options.\n");	//On quitte et on écrit l'erreur
        exit(EXIT_FAILURE);
    }

    AffectationVariableGlobale(pOptions);

    InitialisationSon(&pMoteurSon, pFichierErreur, &sons);	//On charge les sons et on initialise le moteur FMOD

    /* On lit la musique si l'option est activée */
    if(pOptions->musique)
    {
        FMOD_System_PlaySound(pMoteurSon, M_MENU, sons.music[M_MENU], true, NULL);
        FMOD_System_GetChannel(pMoteurSon, M_MENU, &channelEnCours);
        FMOD_Channel_SetVolume(channelEnCours, (float)(pOptions->volume/120.0));
        FMOD_Channel_SetPaused(channelEnCours, false);
    }

    gtk_init(&argc, &argv);		//Initialisation de GTK

    ChargementAnimMAIN(pFichierErreur, &pix);

    CreerInterfaceMAIN(&pListeElements, &pWindow, &pBoutonConnexion, &pBoutonJouer, &pBoutonCredits, &pBoutonOptions, &pBoutonEditeur, &pBoutonQuitter, &pTitre, &pZoneDessinV1, &pZoneDessinV2, &pZoneDessin, &pix);

    AjoutTimeOut(pZoneDessin, pZoneDessinV1, pZoneDessinV2);

    AjoutElementsListe(&pListeElements, pMoteurSon, &sons, pOptions, &joueur);

    /*Affichage de la fenêtre*/
    gtk_widget_show_all(pWindow);

    /* Démarrage de la boucle évènementielle */
    gtk_main();

    /* La boucle évènementielle est terminée on nettoie avant de quitter */
    LiberationMemoireMain(pListeElements, pOptions, pFichierErreur, &sons, pMoteurSon, &pix);

    return EXIT_SUCCESS;	//Fin du programme
}

int Arrondir(double nombre)
{
    /* Cette fonction arrondit (pour les pixels), elle ajoute 0.5 aux nombres positifs (-0.5 aux négatifs), puis tronque la partie décimale */
    if (nombre >= 0)
    {
        return (int)(nombre+0.5);
    }
    else
    {
        return (int)(nombre-0.5);
    }
}

void InitialiserJoueur(Joueur *pJoueur)
{
    /* On met à zéro les informations sur le joueur */
    pJoueur->connexion = 0;
    pJoueur->niveau_max = 1;
    pJoueur->niveauEditeur = -1;
    sprintf(pJoueur->pseudo, "default_pseudo");
    sprintf(pJoueur->mdp, "default_mdp");
    sprintf(pJoueur->autre, "default_other");
}

//Fin du fichier main.c