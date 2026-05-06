#ifndef COURS_H_INCLUDED
#define COURS_H_INCLUDED

#include <stdio.h>
#include "common.h"

// Structure pour les inscriptions
typedef struct {
    int id_inscription;
    int id_membre;
    int id_cours;
    char date_inscription[30];
    char statut[20];
} Inscription;

// Fonctions de gestion des cours
int ajouter_cours(char *filename, Cours c);
int modifier_cours(char *filename, int id, Cours nouv);
int supprimer_cours(char *filename, int id);
Cours chercher_cours(char *filename, int id);

// Fonctions statistiques
int nombre_cours(char *filename);
int cours_par_niveau(char *filename, char *niveau);
int cours_par_coach(char *filename, char *coach);
void statistiques_completes_cours(char *filename);

// Fonctions d'inscription
int sinscrire_cours(char *filename, int id_membre, int id_cours, char *date_inscription);
int verifier_inscription(char *filename, int id_membre, int id_cours);
void afficher_inscriptions_membre(char *filename_inscriptions, int id_membre, char *filename_cours);

#endif
