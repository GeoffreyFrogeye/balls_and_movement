/*
Projet-ISN

Fichier: interfaceMain.c

Contenu: Fonctions appelées par main(), elles construisent l'interface.

Actions: C'est ici que l'interface du menu est construite et que les fonctions pour les animations du menu sont définies. Toutes ces fonctions sont uniquement appelées par main().

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

extern int TailleBloc, TailleBoule, TailleMissileH, TailleMissileW, BMusique, BSons;		//Lien vers les variables globales déclarées dans main.c
extern double Volume, Largeur, Hauteur;
extern InfoDeJeu infos;

void CreerBoutons (GtkWidget **pBoutonConnexion, GtkWidget **pBoutonJouer, GtkWidget **pBoutonCredits, GtkWidget **pBoutonOptions, GtkWidget **pBoutonEditeur, GtkWidget **pBoutonQuitter, GSList **pListeElements)
{
    /* Déclaration des widgets qui vont contenir le texte des buttons */
    GtkWidget *pBoutonConnexionLabel, *pBoutonJouerLabel, *pBoutonCreditsLabel, *pBoutonOptionsLabel, *pBoutonEditeurLabel, *pBoutonQuitterLabel;
    GdkRGBA couleurBoutons= {0.650, 0.850, 0.925, 1}, couleurBoutonsEnfonce= {0.550, 0.655, 0.700, 1};

    /* On crée le bouton CONNEXION */
    *pBoutonConnexion = gtk_button_new();
    pBoutonConnexionLabel = gtk_label_new(g_locale_to_utf8("<span size=\"25000\"><b>Connexion</b></span>", -1, NULL, NULL, NULL));
    gtk_label_set_use_markup(GTK_LABEL(pBoutonConnexionLabel), true);
    gtk_label_set_justify(GTK_LABEL(pBoutonConnexionLabel), GTK_JUSTIFY_CENTER);
    gtk_container_add(GTK_CONTAINER(*pBoutonConnexion), pBoutonConnexionLabel);	//On ajoute le label au bouton

    /* On définit les différentes couleurs */
    gtk_widget_override_background_color(*pBoutonConnexion, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
    gtk_widget_override_background_color(pBoutonConnexionLabel, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
    gtk_widget_override_background_color(*pBoutonConnexion, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
    gtk_widget_override_background_color(pBoutonConnexionLabel, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);

    g_signal_connect(G_OBJECT(*pBoutonConnexion), "clicked", G_CALLBACK(Connexion), *pListeElements); // On ouvre la fenêtre de connexion au clic

    /*						   */
    /* On crée le bouton JOUER */
    *pBoutonJouer = gtk_button_new();
    pBoutonJouerLabel = gtk_label_new(g_locale_to_utf8("<span size=\"25000\"><b>Jouer</b></span>", -1, NULL, NULL, NULL));
    gtk_label_set_use_markup(GTK_LABEL(pBoutonJouerLabel), true);
    gtk_label_set_justify(GTK_LABEL(pBoutonJouerLabel), GTK_JUSTIFY_CENTER);
    gtk_container_add(GTK_CONTAINER(*pBoutonJouer), pBoutonJouerLabel); //On ajoute le label au bouton

    /* On définit les différentes couleurs */
    gtk_widget_override_background_color(*pBoutonJouer, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
    gtk_widget_override_background_color(pBoutonJouerLabel, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
    gtk_widget_override_background_color(*pBoutonJouer, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
    gtk_widget_override_background_color(pBoutonJouerLabel, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
    gtk_widget_override_background_color(*pBoutonJouer, GTK_STATE_FLAG_INSENSITIVE, &couleurBoutons);
    gtk_widget_override_background_color(pBoutonJouerLabel, GTK_STATE_FLAG_INSENSITIVE, &couleurBoutons);

    gtk_widget_set_sensitive(*pBoutonJouer, false);	//On désactive le bouton au début

    /* On ouvre la fenêtre pour choisir le mode de jeu au clique */
    g_signal_connect(G_OBJECT(*pBoutonJouer), "clicked", G_CALLBACK(DemandeModeJeu), *pListeElements);

    *pListeElements = (GSList*)g_list_append((GList*)*pListeElements, GTK_BUTTON(*pBoutonJouer)); //1

    /*						   */
    /* On crée le bouton EDITEUR */
    *pBoutonEditeur = gtk_button_new();
    pBoutonEditeurLabel = gtk_label_new(g_locale_to_utf8("<span size=\"25000\"><b>Editeur</b></span>", -1, NULL, NULL, NULL));
    gtk_label_set_use_markup(GTK_LABEL(pBoutonEditeurLabel), true);
    gtk_label_set_justify(GTK_LABEL(pBoutonEditeurLabel), GTK_JUSTIFY_CENTER);
    gtk_container_add(GTK_CONTAINER(*pBoutonEditeur), pBoutonEditeurLabel);	//On ajoute le label au bouton

    /* On définit les différentes couleurs */
    gtk_widget_override_background_color(*pBoutonEditeur, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
    gtk_widget_override_background_color(pBoutonEditeurLabel, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
    gtk_widget_override_background_color(*pBoutonEditeur, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
    gtk_widget_override_background_color(pBoutonEditeurLabel, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
    gtk_widget_override_background_color(*pBoutonEditeur, GTK_STATE_FLAG_INSENSITIVE, &couleurBoutons);
    gtk_widget_override_background_color(pBoutonEditeurLabel, GTK_STATE_FLAG_INSENSITIVE, &couleurBoutons);

    gtk_widget_set_sensitive(*pBoutonEditeur, false);	//On désactive le bouton au début

    g_signal_connect(G_OBJECT(*pBoutonEditeur), "clicked", G_CALLBACK(DemandeModeEditeur), *pListeElements);// On ouvre la fenêtre SDL de l'éditeur au clic

    *pListeElements = (GSList*)g_list_append((GList*)*pListeElements, GTK_BUTTON(*pBoutonEditeur)); //2

    /*						   */
    /* On crée le bouton QUITTER */
    *pBoutonQuitter = gtk_button_new();
    pBoutonQuitterLabel = gtk_label_new(g_locale_to_utf8("<span size=\"25000\"><b>Quitter</b></span>", -1, NULL, NULL, NULL));
    gtk_label_set_use_markup(GTK_LABEL(pBoutonQuitterLabel), true);
    gtk_label_set_justify(GTK_LABEL(pBoutonQuitterLabel), GTK_JUSTIFY_CENTER);
    gtk_container_add(GTK_CONTAINER(*pBoutonQuitter), pBoutonQuitterLabel);	//On ajoute le label au bouton

    /* On définit les différentes couleurs */
    gtk_widget_override_background_color(*pBoutonQuitter, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
    gtk_widget_override_background_color(pBoutonQuitterLabel, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
    gtk_widget_override_background_color(*pBoutonQuitter, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
    gtk_widget_override_background_color(pBoutonQuitterLabel, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);

    g_signal_connect(G_OBJECT(*pBoutonQuitter), "clicked", G_CALLBACK(FenetreConfirmationQuitter), NULL);	//On ferme quand on clique dessus

    /*						   */
    /* On crée le bouton CREDITS */
    *pBoutonCredits = gtk_button_new();
    pBoutonCreditsLabel = gtk_label_new(g_locale_to_utf8("<span size=\"25000\"><b>Crédits</b></span>", -1, NULL, NULL, NULL));
    gtk_label_set_use_markup(GTK_LABEL(pBoutonCreditsLabel), true);
    gtk_label_set_justify(GTK_LABEL(pBoutonCreditsLabel), GTK_JUSTIFY_CENTER);
    gtk_container_add(GTK_CONTAINER(*pBoutonCredits), pBoutonCreditsLabel);	//On ajoute le label au bouton

    /* On définit les différentes couleurs */
    gtk_widget_override_background_color(*pBoutonCredits, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
    gtk_widget_override_background_color(pBoutonCreditsLabel, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
    gtk_widget_override_background_color(*pBoutonCredits, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
    gtk_widget_override_background_color(pBoutonCreditsLabel, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);

    g_signal_connect(G_OBJECT(*pBoutonCredits), "clicked", G_CALLBACK(LancementCredits), *pListeElements);//On affiche les crédits au clic

    /*						   */
    /*On crée le bouton OPTIONS */
    *pBoutonOptions = gtk_button_new();
    pBoutonOptionsLabel = gtk_label_new(g_locale_to_utf8("<span size=\"25000\"><b>Options</b></span>", -1, NULL, NULL, NULL));
    gtk_label_set_use_markup(GTK_LABEL(pBoutonOptionsLabel), true);
    gtk_label_set_justify(GTK_LABEL(pBoutonOptionsLabel), GTK_JUSTIFY_CENTER);
    gtk_container_add(GTK_CONTAINER(*pBoutonOptions), pBoutonOptionsLabel);	//On ajoute le label au bouton

    /* On définit les différentes couleurs */
    gtk_widget_override_background_color(*pBoutonOptions, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
    gtk_widget_override_background_color(pBoutonOptionsLabel, GTK_STATE_FLAG_NORMAL, &couleurBoutons);
    gtk_widget_override_background_color(*pBoutonOptions, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);
    gtk_widget_override_background_color(pBoutonOptionsLabel, GTK_STATE_FLAG_ACTIVE, &couleurBoutonsEnfonce);

    g_signal_connect(G_OBJECT(*pBoutonOptions), "clicked", G_CALLBACK(LancementOptions), *pListeElements);	//On affiche les options au clic
}

void CreerFenetre(GtkWidget **pWindow)
{
    GdkRGBA couleurFond= {0.610, 0.805, 0.920, 1};

    /* On crée la fenêtre */
    *pWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_widget_set_events(*pWindow, GDK_KEY_RELEASE_MASK);

    /* Connexion du signal "delete-event" avec la fonction DestructionFenetre qui affiche un message de confirmation pour quitter */
    g_signal_connect(G_OBJECT(*pWindow), "delete-event", G_CALLBACK(FenetreConfirmationQuitter), NULL);
    g_signal_connect(G_OBJECT(*pWindow), "key_release_event", G_CALLBACK(QuitterEchapeMain), NULL);

    gtk_window_set_position(GTK_WINDOW(*pWindow), GTK_WIN_POS_CENTER_ALWAYS);	//Position
    gtk_window_set_title(GTK_WINDOW(*pWindow), g_locale_to_utf8("Boules et Mouvement", -1, NULL, NULL, NULL));	//Titre
    gtk_widget_set_size_request(*pWindow, 800, 600);	//Taille
    gtk_window_set_resizable(GTK_WINDOW(*pWindow), false);
    gtk_window_set_icon_from_file(GTK_WINDOW(*pWindow), "ressources/img/z.png", NULL);	//Icône
    gtk_widget_override_background_color(*pWindow, GTK_STATE_FLAG_NORMAL, &couleurFond);	//Couleur
}

void CreerBoites(GtkWidget **pWindow, GtkWidget **pBoutonConnexion, GtkWidget **pBoutonJouer, GtkWidget **pBoutonOptions, GtkWidget **pBoutonEditeur, GtkWidget **pBoutonCredits, GtkWidget **pBoutonQuitter, GtkWidget **pZoneDessinV1, GtkWidget **pZoneDessinV2, GtkWidget **pTitre, GtkWidget **pZoneDessin)
{
    GtkWidget *pBoxVBoutons, *pBoxHMissilesBoutons, *pBoxVTitreBoulesMissilesButons;

    /* On crée les boites qui vont contenir les différents éléments de l'interface afin de la mettre en forme, on ajoute dans les boites les éléments de l'interface, puis on met les boites dans une grande boite, que l'on place dans la fenêtre */

    pBoxVBoutons = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(pBoxVBoutons), *pBoutonConnexion, false, false, 8);
    gtk_box_pack_start(GTK_BOX(pBoxVBoutons), *pBoutonJouer, false, false, 8);
    gtk_box_pack_start(GTK_BOX(pBoxVBoutons), *pBoutonOptions, false, false, 8);
    gtk_box_pack_start(GTK_BOX(pBoxVBoutons), *pBoutonEditeur, false, false, 8);
    gtk_box_pack_start(GTK_BOX(pBoxVBoutons), *pBoutonCredits, false, false, 8);
    gtk_box_pack_start(GTK_BOX(pBoxVBoutons), *pBoutonQuitter, false, false, 8);

    pBoxHMissilesBoutons = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(pBoxHMissilesBoutons), *pZoneDessinV1, false, false, 0);
    gtk_box_pack_start(GTK_BOX(pBoxHMissilesBoutons), pBoxVBoutons, false, false, 0);
    gtk_box_pack_start(GTK_BOX(pBoxHMissilesBoutons), *pZoneDessinV2, false, false, 0);

    pBoxVTitreBoulesMissilesButons = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(pBoxVTitreBoulesMissilesButons), *pTitre, false, false, 0);
    gtk_box_pack_start(GTK_BOX(pBoxVTitreBoulesMissilesButons), *pZoneDessin, false, false, 0);
    gtk_box_pack_start(GTK_BOX(pBoxVTitreBoulesMissilesButons), pBoxHMissilesBoutons, false, false, 0);

    gtk_container_add(GTK_CONTAINER(*pWindow), pBoxVTitreBoulesMissilesButons);
}

void CreerTitre(GtkWidget **pTitre)
{
    /* On crée le texwte du titre */
    *pTitre = gtk_label_new(g_locale_to_utf8("<span underline=\"single\" font-family=\"Snickles\" size=\"50000\">Balls and movement</span>", -1, NULL, NULL, NULL));
    gtk_label_set_use_markup(GTK_LABEL(*pTitre), true);
    gtk_label_set_justify(GTK_LABEL(*pTitre), GTK_JUSTIFY_CENTER);
    gtk_widget_set_size_request(*pTitre, 800, 40);
}

void CreerZonesDessin(GtkWidget **pZoneDessin, GtkWidget **pZoneDessinV1, GtkWidget **pZoneDessinV2, GdkImages *pix)
{
    /* On crée les différentes zone de dessin des animations */
    *pZoneDessin = gtk_drawing_area_new();
    gtk_widget_set_size_request(*pZoneDessin, 800, 160);
    g_signal_connect(G_OBJECT(*pZoneDessin), "draw", G_CALLBACK(Peindre), pix);

    *pZoneDessinV1 = gtk_drawing_area_new();
    gtk_widget_set_size_request(*pZoneDessinV1, 300, 400);
    g_signal_connect(G_OBJECT(*pZoneDessinV1), "draw", G_CALLBACK(PeindreV1), pix);

    *pZoneDessinV2 = gtk_drawing_area_new();
    gtk_widget_set_size_request(*pZoneDessinV2, 300, 400);
    g_signal_connect(G_OBJECT(*pZoneDessinV2), "draw", G_CALLBACK(PeindreV2), pix);
}

void LiberationMemoireMain(GSList *pListeElements, Options *pOptions, FILE *pFichierErreur, Sons *pSons, FMOD_SYSTEM *pMoteurSon, GdkImages *pPix)
{
    /* Cette fonction libère la mémoire utilisée par les fonctions d'interface de main et FMOD */

    int i=0;

    while(pSons->music[i] !=NULL)
    {
        FMOD_Sound_Release(pSons->music[i]);
        i++;
    }

    i=0;

    while(pSons->bruits[i] !=NULL)
    {
        FMOD_Sound_Release(pSons->bruits[i]);
        i++;
    }

    g_list_free((GList*)pListeElements);

    for(i=0; i<4; i++)
    {
        g_object_unref(pPix->img[i]);
    }

    FMOD_System_Release(pMoteurSon);

    free(pOptions);
    fclose(pFichierErreur);
}

void AffectationVariableGlobale(Options *pOptions)
{
    /* Affectation des variables globales */

    Hauteur = pOptions->hauteur;
    Largeur = pOptions->largeur;
    TailleBloc = Arrondir(Largeur/40.0);
    TailleBoule = Arrondir(Largeur/45.0);
    TailleMissileW = Arrondir(Largeur/30.0);
    TailleMissileH = Arrondir(Hauteur/5.5);
    BSons = pOptions->sons;
    BMusique = pOptions->musique;
    Volume = pOptions->volume;
}

void ChargementAnimMAIN(FILE *pFichierErreur, GdkImages *pPix)
{
    int i=0;

    /* On charge les images pour les animations du menu */
    pPix->img[0] = gdk_pixbuf_new_from_file("ressources/img/boule_bleue.png", NULL);
    pPix->img[1] = gdk_pixbuf_new_from_file("ressources/img/boule_verte.png", NULL);
    pPix->img[2] = gdk_pixbuf_new_from_file("ressources/img/boule_magenta.png", NULL);
    pPix->img[3] = gdk_pixbuf_new_from_file("ressources/img/missile.png", NULL);

    for(i=0; i<4; i++)
    {
        if(pPix->img[i] == NULL)
        {
            fprintf(pFichierErreur, "Impossible de charger les images pour les animations du menu.\n");	
            exit(EXIT_FAILURE);
        }
    }
}

void CreerInterfaceMAIN(GSList **pListeElements, GtkWidget **pWindow, GtkWidget **pBoutonConnexion, GtkWidget **pBoutonJouer, GtkWidget **pBoutonCredits, GtkWidget **pBoutonOptions, GtkWidget **pBoutonEditeur, GtkWidget **pBoutonQuitter, GtkWidget **pTitre, GtkWidget **pZoneDessinV1, GtkWidget **pZoneDessinV2, GtkWidget **pZoneDessin, GdkImages *pix)
{
    /* Création de la fenêtre du menu, des boutons, ... */
    CreerFenetre(pWindow);
    *pListeElements = (GSList*)g_list_append((GList*)*pListeElements, *pWindow); //0
    CreerTitre(pTitre);
    CreerBoutons(pBoutonConnexion, pBoutonJouer, pBoutonCredits, pBoutonOptions, pBoutonEditeur, pBoutonQuitter, pListeElements);
    CreerZonesDessin(pZoneDessin, pZoneDessinV1, pZoneDessinV2, pix);
    CreerBoites(pWindow, pBoutonConnexion, pBoutonJouer, pBoutonOptions, pBoutonEditeur, pBoutonCredits, pBoutonQuitter, pZoneDessinV1, pZoneDessinV2, pTitre, pZoneDessin);
}

void AjoutTimeOut(GtkWidget *pZoneDessin, GtkWidget *pZoneDessinV1, GtkWidget *pZoneDessinV2)
{
    /* On ajoute des timers pour les fonctions de mise à jour des animations */
    g_timeout_add(20, (GSourceFunc)Avancer, NULL);
    g_timeout_add_full(G_PRIORITY_HIGH, 30, (GSourceFunc)Redessiner, pZoneDessin, NULL);
    g_timeout_add_full(G_PRIORITY_HIGH, 30, (GSourceFunc)Redessiner, pZoneDessinV1, NULL);
    g_timeout_add_full(G_PRIORITY_HIGH, 30, (GSourceFunc)Redessiner, pZoneDessinV2, NULL);
}

void AjoutElementsListe(GSList **pListeElements, FMOD_SYSTEM *pMoteurSon, Sons *pSons, Options *pOptions, Joueur *pJoueur)
{
    /* Ajout des éléments dont ont aura besoin après à la liste chainée */
    *pListeElements = g_slist_append(*pListeElements, pMoteurSon);//3
    *pListeElements = g_slist_append(*pListeElements, pSons);	//4
    *pListeElements = g_slist_append(*pListeElements, pOptions); //5
    *pListeElements = g_slist_append(*pListeElements, pJoueur); //6
}

//Fin du fichier interfaceMain.c