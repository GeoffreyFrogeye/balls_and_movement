/*
Projet-ISN

Fichier: callback.h

Contenu: Prototypes des fonctions contenues dans callback.c

Actions: Permet � l'ordinateur de conna�tre toutes les fonctions pr�sentes dans le programme ainsi que leurs arguments.

Jean-Loup BEAUSSART & Dylan GUERVILLE
*/

#ifndef CALLBACK_H_INCLUDED
#define CALLBACK_H_INCLUDED	//Protection contre les inclusions infinies

/* Prototypes des fonctions */
void AfficherMenu(GtkWidget *pWidget, gpointer pData);
int Avancer();
void Connexion(GtkWidget *pWidget, gpointer pData);
void ConnexionMySql(GtkWidget *pWidget, gpointer pData);
void DemandeModeEditeur(GtkWidget *pWidget, gpointer pData);
void DemandeModeJeu(GtkWidget *pWidget, gpointer pData);
void EditionNiveau(GtkWidget *pWidget, gpointer pData);
void FenetreConfirmationQuitter(GtkWidget *pWidget, gpointer pData);
void FermerCredit(GtkWidget *pWidget, GdkEvent *event, gpointer pData);
void FermerFenetre(GtkWidget *pWidget, gpointer pData);
void LancementCredits(GtkWidget *pWidget, gpointer pData);
void LancementEditeur(GtkWidget *pWidget, gpointer pData);
void LancementOptions(GtkWidget *pWidget, gpointer pData);
void LancerEditeur(GtkWidget *pWidget, gpointer pData);
void LancerJeuModeCampagne(GtkWidget *pWidget, gpointer pData);
void LancerJeuModePerso(GtkWidget *pWidget, gpointer pData);
void MiseAJourSelection(GtkTreeSelection *pSelection, gpointer pData);
void ModeGuest(GtkWidget *pWidget, gpointer pData);
void ModifierOptionsListe(GtkComboBox *pComboBox, gpointer pData);
gboolean ModifierOptionsRange1(GtkRange *range, GtkScrollType scroll, double valeur, gpointer pData);
gboolean ModifierOptionsRange2(GtkRange *range, GtkScrollType scroll, double valeur, gpointer pData);
void ModifierOptionsToggleButton1(GtkToggleButton *pToggleButton, gpointer pData);
void ModifierOptionsToggleButton2(GtkToggleButton *pToggleButton, gpointer pData);
void ModifierOptionsToggleButton3(GtkToggleButton *pToggleButton, gpointer pData);
void Peindre(GtkWidget *pWidget, cairo_t *cr, gpointer pData);
void PeindreV1(GtkWidget *pWidget, cairo_t *cr, gpointer pData);
void PeindreV2(GtkWidget *pWidget, cairo_t *cr, gpointer pData);
int Redessiner(gpointer pData);
int QuitterEchapeCredits(GtkWidget *pWidget, GdkEventKey *pEvent, gpointer pData);
int QuitterEchapeMain(GtkWidget *pWidget, GdkEventKey *pEvent);
void SauverOptions(GtkWidget *pWidget, gpointer pData);
void SupprimerNiveau(GtkWidget *pWidget, gpointer pData);

#endif // CALLBACK_H_INCLUDED

//Fin du fichier callback.h
