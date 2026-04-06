#ifndef COMMON_H
#define COMMON_H

#include <gtk/gtk.h>  // AJOUTEZ CETTE LIGNE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// ============================================================================
// STRUCTURES UNIFIQUES
// ============================================================================

typedef struct {
    int id;
    char nom[100];  // Increased size to hold both nom and prenom
    char email[100];
    int age;
    char sexe[20];
    char Typeabonnement[20];
    float Tarif;
    
} Membre;
typedef struct {
    int id;
    char nometprenom[60];
    char specialite[30];
    int experience;
    int age;           
    char sexe;        
} Entraineur;
typedef struct {
    int id;
    char cours[30];
    char coach[30];
    char date_heure[30];
    char salle[20];
    char niveau[20];
} Cours;

typedef struct {
    char reference[20];
    char type[30];
    char etat;
    char centre[50];
    int capacite;
    char disponibilite;
} Equipement;

typedef struct {
    int id;
    char nom[100];
    char adresse[200];
    char ville[50];
    int capacite;
    char etat;
} Centre;

// ============================================================================
// STRUCTURE AUTHENTIFICATION
// ============================================================================

typedef struct {
    char username[50];
    char password[50];
    char type[20];
    int user_id;
} Utilisateur;

// ============================================================================
// FONCTIONS AUTHENTIFICATION
// ============================================================================

int verifier_login(const char *username, const char *password, char *user_type, int *user_id);
int ajouter_utilisateur(const char *username, const char *password, const char *type, int user_id);
int utilisateur_existe(const char *username);
int supprimer_utilisateur(const char *username);

// ============================================================================
// FONCTIONS GESTION FICHIERS
// ============================================================================

int fichier_existe(const char *filename);
void creer_fichier_si_inexistant(const char *filename);
void initialiser_fichiers_donnees();
int compter_lignes_fichier(const char *filename);
int est_fichier_vide(const char *filename);

// ============================================================================
// FONCTIONS GÉNÉRATION ID
// ============================================================================

int get_prochain_id_membre();
int get_prochain_id_entraineur();
int get_prochain_id_cours();
int get_prochain_id_centre();
char* generer_reference_equipement(const char *type);

// ============================================================================
// FONCTIONS VALIDATION
// ============================================================================

int est_email_valide(const char *email);
int est_telephone_valide(const char *telephone);
int est_date_valide(const char *date);
int est_nom_valide(const char *nom);
int est_age_valide(int age);
int est_prix_valide(float prix);

// ============================================================================
// FONCTIONS DATE ET HEURE
// ============================================================================

void get_date_actuelle(char *date);
void get_heure_actuelle(char *heure);
void get_datetime_actuelle(char *datetime);
int comparer_dates(const char *date1, const char *date2);
int ajouter_jours_date(const char *date_source, int jours, char *date_resultat);

// ============================================================================
// FONCTIONS CHAÎNES DE CARACTÈRES
// ============================================================================

void trim_string(char *str);
void to_uppercase(char *str);
void to_lowercase(char *str);
int contient_seulement_lettres(const char *str);
int contient_seulement_chiffres(const char *str);
void supprimer_caracteres_speciaux(char *str);

// ============================================================================
// FONCTIONS CONVERSION
// ============================================================================

char* statut_equipement_char_to_str(char statut);
char statut_equipement_str_to_char(const char *statut);
char* type_abonnement_to_str(int type);
char* sexe_to_str(char sexe);
char sexe_str_to_char(const char *sexe);

// ============================================================================
// FONCTIONS STATISTIQUES
// ============================================================================

int get_nombre_membres_actifs();
int get_nombre_entraineurs_actifs();
int get_nombre_cours_actifs();
int get_nombre_equipements_disponibles();
float get_taux_remplissage_centre(int centre_id);

// ============================================================================
// FONCTIONS RAPPORTS
// ============================================================================

void generer_rapport_membres(const char *filename);
void generer_rapport_cours(const char *filename);
void generer_rapport_financier(const char *filename);

#endif
