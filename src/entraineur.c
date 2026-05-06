#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include "entraineur.h"

#define MAX_ENTRAINEURS_PAR_COURS 100
#define MAX_COURS 50

typedef struct {
    Cours *c; 
    Entraineur *liste_entraineurs[MAX_ENTRAINEURS_PAR_COURS];
    int nb_inscrits;
} inscription_cours;

inscription_cours inscriptions[MAX_COURS];


// Ajouter un entraîneur - VERSION CORRIGÉE SANS STATUT
int ajouter_entraineur(char *filename, Entraineur e){
    FILE *f = fopen(filename, "a");
    if(f != NULL){
        // NOUVEAU FORMAT: ID NomEtPrenom Specialite Experience Age Sexe
        fprintf(f, "%d %s %s %d %d %c\n", 
                e.id, e.nometprenom, e.specialite, e.experience, e.age, e.sexe);
        fclose(f);
        return 1;
    }
    return 0;
}

// Modifier un entraîneur - VERSION CORRIGÉE SANS STATUT
int modifier_entraineur(char *filename, int id, Entraineur nouv) {
    int tr = 0;
    
    if(id <= 0) {
        return 0;
    }
    
    FILE *f = fopen(filename, "r");
    if(f == NULL) {
        return 0;
    }
    
    FILE *f2 = fopen("temp.txt", "w");
    if(f2 == NULL) {
        fclose(f);
        return 0;
    }
    
    Entraineur e;
    char ligne[512];
    int line_count = 0;
    
    while(fgets(ligne, sizeof(ligne), f) != NULL && line_count < 10000) {
        line_count++;
        
        // NOUVEAU FORMAT de lecture SANS STATUT
        int scan_result = sscanf(ligne, "%d %59s %29s %d %d %c",
                                &e.id, e.nometprenom, e.specialite, 
                                &e.experience, &e.age, &e.sexe);
        
        if(scan_result >= 5) { // Au moins 5 champs requis
            if(e.id == id) {
                // Écrire l'entraîneur modifié
                fprintf(f2, "%d %s %s %d %d %c\n",
                       nouv.id, nouv.nometprenom, nouv.specialite, 
                       nouv.experience, nouv.age, nouv.sexe);
                tr = 1;
            } else {
                // Écrire l'entraîneur tel quel
                fprintf(f2, "%d %s %s %d %d %c\n",
                       e.id, e.nometprenom, e.specialite, 
                       e.experience, e.age, e.sexe);
            }
        } else {
            fputs(ligne, f2);
        }
    }
    
    fclose(f);
    fclose(f2);
    
    if(tr == 0) {
        remove("temp.txt");
        return 0;
    }
    
    if(remove(filename) != 0) {
        remove("temp.txt");
        return 0;
    }
    
    if(rename("temp.txt", filename) != 0) {
        return 0;
    }
    
    return 1;
}

// Supprimer un entraîneur - VERSION CORRIGÉE SANS STATUT
int supprimer_entraineur(char *filename, int id){
    int tr = 0;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("temp.txt", "w");
    Entraineur e;

    if(f != NULL && f2 != NULL){
        char ligne[512];
        while(fgets(ligne, sizeof(ligne), f) != NULL) {
            // NOUVEAU FORMAT de lecture SANS STATUT
            if(sscanf(ligne, "%d %59s %29s %d %d %c",
                     &e.id, e.nometprenom, e.specialite, 
                     &e.experience, &e.age, &e.sexe) >= 5) {
                if(e.id == id) {
                    tr = 1;
                } else {
                    fprintf(f2, "%d %s %s %d %d %c\n",
                           e.id, e.nometprenom, e.specialite, 
                           e.experience, e.age, e.sexe);
                }
            }
        }
        fclose(f);
        fclose(f2);
        remove(filename);
        rename("temp.txt", filename);
    }
    return tr;
}

// Chercher un entraîneur par ID - VERSION CORRIGÉE SANS STATUT
Entraineur chercher_entraineur(char *filename, int id){
    Entraineur e;
    e.id = -1; // Valeur par défaut si non trouvé
    
    FILE *f = fopen(filename, "r");
    if(f != NULL){
        char ligne[512];
        while(fgets(ligne, sizeof(ligne), f) != NULL) {
            // NOUVEAU FORMAT de lecture SANS STATUT
            if(sscanf(ligne, "%d %59s %29s %d %d %c",
                     &e.id, e.nometprenom, e.specialite, 
                     &e.experience, &e.age, &e.sexe) >= 5) {
                if(e.id == id) {
                    fclose(f);
                    return e;
                }
            }
        }
        fclose(f);
    }
    e.id = -1;
    return e;
}

// S'inscrire à un cours sportif en tant qu'entraîneur
void inscrire_entraineur_a_cours(Cours *c, Entraineur *e) {
    for (int i = 0; i < MAX_COURS; i++) {
        if (inscriptions[i].c == NULL) {
            inscriptions[i].c = c;
        }
        if (inscriptions[i].c == c) {
            if (inscriptions[i].nb_inscrits < MAX_ENTRAINEURS_PAR_COURS) {
                inscriptions[i].liste_entraineurs[inscriptions[i].nb_inscrits] = e;
                inscriptions[i].nb_inscrits++;
                printf("Entraineur %s inscrit au cours %s\n", e->nometprenom, c->cours);
            } else {
                printf("Le cours %s est complet.\n", c->cours);
            }
            return;
        }
    }
}

// Afficher entraîneurs du cours
void afficher_entraineurs_du_cours(Cours *c) {
    for (int i = 0; i < MAX_COURS; i++) {
        if (inscriptions[i].c == c) {
            printf("Entraineurs inscrits au cours %s:\n", c->cours);
            for (int j = 0; j < inscriptions[i].nb_inscrits; j++) {
                printf(" - %s (%s)\n", inscriptions[i].liste_entraineurs[j]->nometprenom,
                                        inscriptions[i].liste_entraineurs[j]->specialite);
            }
        }
    }
}

// Afficher tous les entraîneurs - VERSION CORRIGÉE SANS STATUT
void afficher_entraineurs(char *filename){
    FILE *f = fopen(filename, "r");
    Entraineur e;
    if(f != NULL){
        printf("\n%-5s %-20s %-15s %-8s %-5s %-10s\n", 
               "ID", "Nom et Prenom", "Specialite", "Exp", "Age", "Sexe");
        
        char ligne[512];
        while(fgets(ligne, sizeof(ligne), f) != NULL) {
            if(sscanf(ligne, "%d %59s %29s %d %d %c",
                     &e.id, e.nometprenom, e.specialite, 
                     &e.experience, &e.age, &e.sexe) >= 5) {
                printf("%-5d %-20s %-15s %-8d %-5d %-5c\n", 
                       e.id, e.nometprenom, e.specialite, 
                       e.experience, e.age, e.sexe);
            }
        }
        fclose(f);
    } else {
        printf("Fichier introuvable.\n");
    }
}

// Nombre total - VERSION CORRIGÉE SANS STATUT
int nombre_entraineurs(char *filename){
    int count = 0;
    FILE *f = fopen(filename, "r");
    Entraineur e;
    if(f != NULL){
        char ligne[512];
        while(fgets(ligne, sizeof(ligne), f) != NULL) {
            if(sscanf(ligne, "%d %59s %29s %d %d %c",
                     &e.id, e.nometprenom, e.specialite, 
                     &e.experience, &e.age, &e.sexe) >= 5) {
                count++;
            }
        }
        fclose(f);
    }
    return count;
}

// Expérience moyenne - VERSION CORRIGÉE SANS STATUT
float experience_moyenne(char *filename){
    int total_exp = 0, nb = 0;
    FILE *f = fopen(filename, "r");
    Entraineur e;
    if(f != NULL){
        char ligne[512];
        while(fgets(ligne, sizeof(ligne), f) != NULL) {
            if(sscanf(ligne, "%d %59s %29s %d %d %c",
                     &e.id, e.nometprenom, e.specialite, 
                     &e.experience, &e.age, &e.sexe) >= 5) {
                total_exp += e.experience;
                nb++;
            }
        }
        fclose(f);
    }
    if(nb > 0) return (float)total_exp/nb;
    return 0;
}

// Nombre d'entraîneurs par spécialité - VERSION CORRIGÉE SANS STATUT
int entraineurs_par_specialite(char *filename, char *specialite){
    int count = 0;
    FILE *f = fopen(filename, "r");
    Entraineur e;
    if(f != NULL){
        char ligne[512];
        while(fgets(ligne, sizeof(ligne), f) != NULL) {
            if(sscanf(ligne, "%d %59s %29s %d %d %c",
                     &e.id, e.nometprenom, e.specialite, 
                     &e.experience, &e.age, &e.sexe) >= 5) {
                if(strcmp(e.specialite, specialite) == 0) count++;
            }
        }
        fclose(f);
    }
    return count;
}

// Plus expérimenté - VERSION CORRIGÉE SANS STATUT
Entraineur plus_experimente(char *filename){
    Entraineur e, max_e;
    int max_exp = -1;
    FILE *f = fopen(filename, "r");
    max_e.id = -1;
    if(f != NULL){
        char ligne[512];
        while(fgets(ligne, sizeof(ligne), f) != NULL) {
            if(sscanf(ligne, "%d %59s %29s %d %d %c",
                     &e.id, e.nometprenom, e.specialite, 
                     &e.experience, &e.age, &e.sexe) >= 5) {
                if(e.experience > max_exp){
                    max_exp = e.experience;
                    max_e = e;
                }
            }
        }
        fclose(f);
    }
    return max_e;
}

// Moins expérimenté - VERSION CORRIGÉE SANS STATUT
Entraineur moins_experimente(char *filename){
    Entraineur e, min_e;
    int min_exp = INT_MAX;
    FILE *f = fopen(filename, "r");
    min_e.id = -1;
    if(f != NULL){
        char ligne[512];
        while(fgets(ligne, sizeof(ligne), f) != NULL) {
            if(sscanf(ligne, "%d %59s %29s %d %d %c",
                     &e.id, e.nometprenom, e.specialite, 
                     &e.experience, &e.age, &e.sexe) >= 5) {
                if(e.experience < min_exp){
                    min_exp = e.experience;
                    min_e = e;
                }
            }
        }
        fclose(f);
    }
    return min_e;
}

// Statistiques globales et par spécialité - VERSION CORRIGÉE SANS STATUT
void statistiques(){
    FILE *f = fopen("entraineurs.txt","r");
    if(!f){ printf("Fichier introuvable.\n"); return; }

    Entraineur e;
    Entraineur tab[100]; int nb_total=0;
    char specialites[20][20]; int nb_spec=0;

    char ligne[512];
    while(fgets(ligne, sizeof(ligne), f) != NULL){
        if(sscanf(ligne, "%d %59s %29s %d %d %c",
                 &e.id, e.nometprenom, e.specialite, 
                 &e.experience, &e.age, &e.sexe) >= 5) {
            tab[nb_total++] = e;
            int exist=0;
            for(int i=0;i<nb_spec;i++) if(strcmp(specialites[i],e.specialite)==0) exist=1;
            if(!exist && nb_spec<20) strcpy(specialites[nb_spec++], e.specialite);
        }
    }
    fclose(f);

    if(nb_total==0){ printf("Aucun entraîneur.\n"); return; }

    // Statistiques globales
    int total_exp=0,total_age=0;
    for(int i=0;i<nb_total;i++){ total_exp+=tab[i].experience; total_age+=tab[i].age; }
    printf("\n=== Statistiques globales ===\n");
    printf("Nombre total : %d\n", nb_total);
    printf("Experience moyenne : %.2f\n", (float)total_exp/nb_total);
    printf("Age moyen : %.2f\n", (float)total_age/nb_total);

    Entraineur plus=tab[0], moins=tab[0];
    for(int i=1;i<nb_total;i++){
        if(tab[i].experience>plus.experience) plus=tab[i];
        if(tab[i].experience<moins.experience) moins=tab[i];
    }
    printf("Le plus experimente : %s (%s) - %d ans\n", plus.nometprenom, plus.specialite, plus.experience);
    printf("Le moins experimente : %s (%s) - %d ans\n", moins.nometprenom, moins.specialite, moins.experience);

    // Statistiques par spécialité
    printf("\n=== Statistiques par specialite ===\n");
    for(int i=0;i<nb_spec;i++){
        int count=0,sum_exp=0,sum_age=0;
        Entraineur plus_s, moins_s; int first=1;
        for(int j=0;j<nb_total;j++){
            if(strcmp(tab[j].specialite, specialites[i])==0){
                count++;
                sum_exp+=tab[j].experience; sum_age+=tab[j].age;
                if(first){ plus_s=moins_s=tab[j]; first=0; }
                if(tab[j].experience>plus_s.experience) plus_s=tab[j];
                if(tab[j].experience<moins_s.experience) moins_s=tab[j];
            }
        }
        printf("\nSpecialite : %s\n", specialites[i]);
        printf("Nombre : %d\n", count);
        printf("Experience moyenne : %.2f\n", (float)sum_exp/count);
        printf("Age moyen : %.2f\n", (float)sum_age/count);
        printf("Le plus experimente : %s - %d ans\n", plus_s.nometprenom, plus_s.experience);
        printf("Le moins experimente : %s - %d ans\n", moins_s.nometprenom, moins_s.experience);
    }
    printf("\n===============================\n");
}
