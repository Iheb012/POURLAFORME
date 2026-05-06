#ifndef CENTRE_H
#define CENTRE_H

#include "common.h"  // Inclure common.h pour avoir la structure Centre

// Fonctions de gestion des centres
int ajouter_centre(const char *filename, Centre c);
int modifier_centre(const char *filename, int id, Centre nouv);
int supprimer_centre(const char *filename, int id);
Centre chercher_centre(const char *filename, int id);


// Fonctions de comptage
int nombre_centres(const char *filename);

// Fonctions d'affichage
void afficher_centres_actifs_seulement(const char *filename);
void afficher_tous_centres_tableau(const char *filename);

// Fonctions de statistiques
void statistiques_completes_centre(const char *filename);
void afficher_statistiques_centres(const char *filename);

// Fonctions d'inscription
int inscrire_entraineur_centre(const char *filename_inscriptions, int id_entraineur, int id_centre, char *date_inscription);
void mes_inscriptions_centres(const char *filename_inscriptions, int id_entraineur, const char *filename_centres);

// Fonctions utilitaires
void tester_fonctions_centre();
void lister_tous_centres(const char *filename);
int centre_est_actif(const char *filename, int id_centre);
void centres_par_ville(const char *filename, const char *ville);

#endif
