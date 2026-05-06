#include "equipement.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// FONCTIONS DE BASE (CORRECTED)
// ============================================================================

int ajouter_equipement(char *filename, Equipement e)
{
    // Vérifier si la référence existe déjà
    Equipement existant = chercher_equipement(filename, e.reference);
    if (strcmp(existant.reference, "") != 0)
    {
        printf("✗ Erreur : la référence '%s' existe déjà.\n", e.reference);
        return 0;
    }

    FILE *f = fopen(filename, "a");
    if(f != NULL)
    {
        fprintf(f, "%s %s %s %d %c %c\n", 
                e.reference, e.type, e.centre, e.capacite, e.disponibilite, e.etat);
        fclose(f);
        printf("✓ Équipement ajouté avec succès\n");
        return 1;
    }
    else 
    {
        printf("✗ Erreur : impossible d'ouvrir le fichier %s\n", filename);
        return 0;
    }
}

int modifier_equipement(char *filename, char *reference, Equipement nouv)
{
    int tr = 0;
    Equipement e;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("temp.txt", "w");
    
    if(f == NULL || f2 == NULL)
    {
        printf("✗ Erreur : impossible d'ouvrir les fichiers\n");
        if(f != NULL) fclose(f);
        if(f2 != NULL) fclose(f2);
        return 0;
    }

    while(fscanf(f, "%s %s %s %d %c %c", 
                e.reference, e.type, e.centre, &e.capacite, &e.disponibilite, &e.etat) != EOF)
    {
        if(strcmp(e.reference, reference) == 0)
        {
            fprintf(f2, "%s %s %s %d %c %c\n", 
                    nouv.reference, nouv.type, nouv.centre, nouv.capacite, nouv.disponibilite, nouv.etat);
            tr = 1;
        }
        else
        {
            fprintf(f2, "%s %s %s %d %c %c\n", 
                    e.reference, e.type, e.centre, e.capacite, e.disponibilite, e.etat);
        }
    }
    
    fclose(f);
    fclose(f2);
    
    remove(filename);
    rename("temp.txt", filename);
    
    if(tr) {
        printf("✓ Équipement '%s' modifié avec succès\n", reference);
    } else {
        printf("✗ Équipement '%s' non trouvé\n", reference);
    }
    
    return tr;
}

int supprimer_equipement(char *filename, const char *reference)
{
    int tr = 0;
    Equipement e;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("temp.txt", "w");
    
    if(f == NULL || f2 == NULL)
    {
        printf("✗ Erreur : impossible d'ouvrir les fichiers\n");
        if(f != NULL) fclose(f);
        if(f2 != NULL) fclose(f2);
        return 0;
    }

    while(fscanf(f, "%s %s %s %d %c %c", 
                e.reference, e.type, e.centre, &e.capacite, &e.disponibilite, &e.etat) != EOF)
    {
        if(strcmp(e.reference, reference) == 0)
            tr = 1;
        else
            fprintf(f2, "%s %s %s %d %c %c\n", 
                    e.reference, e.type, e.centre, e.capacite, e.disponibilite, e.etat);
    }
    
    fclose(f);
    fclose(f2);
    
    remove(filename);
    rename("temp.txt", filename);
    
    if(tr) {
        printf("✓ Équipement '%s' supprimé avec succès\n", reference);
    } else {
        printf("✗ Équipement '%s' non trouvé\n", reference);
    }
    
    return tr;
}

Equipement chercher_equipement(char *filename, const char *reference)
{
    Equipement e;
    int tr = 0;
    FILE *f = fopen(filename, "r");
    
    // Initialiser l'équipement avec des valeurs par défaut
    strcpy(e.reference, "");
    strcpy(e.type, "");
    strcpy(e.centre, "");
    e.capacite = 0;
    e.disponibilite = ' ';
    e.etat = ' ';
    
    if(f != NULL)
    {
        while(tr == 0 && fscanf(f, "%s %s %s %d %c %c", 
                               e.reference, e.type, e.centre, &e.capacite, &e.disponibilite, &e.etat) != EOF)
        {
            if(strcmp(e.reference, reference) == 0)
                tr = 1;
        }
        fclose(f);
    }
    
    if(tr == 0)
        strcpy(e.reference, "");
    
    return e;
}

// ============================================================================
// FONCTIONS DE COMPTAGE
// ============================================================================

int compter_total_equipements(char *filename)
{
    Equipement e;
    int count = 0;
    FILE *f = fopen(filename, "r");
    if(f != NULL)
    {
        while(fscanf(f, "%s %s %s %d %c %c", 
                    e.reference, e.type, e.centre, &e.capacite, &e.disponibilite, &e.etat) != EOF)
        {
            count++;
        }
        fclose(f);
    }
    return count;
}

int compter_equipements_par_type(char *filename, char *type)
{
    Equipement e;
    int count = 0;
    FILE *f = fopen(filename, "r");
    if(f != NULL)
    {
        while(fscanf(f, "%s %s %s %d %c %c", 
                    e.reference, e.type, e.centre, &e.capacite, &e.disponibilite, &e.etat) != EOF)
        {
            if(strcmp(e.type, type) == 0)
                count++;
        }
        fclose(f);
    }
    return count;
}

// ============================================================================
// FONCTIONS D'AFFICHAGE
// ============================================================================

void afficher_equipements_disponibles(char *filename)
{
    Equipement e;
    FILE *f = fopen(filename, "r");
    if(f != NULL)
    {   int count = 0;
        while(fscanf(f, "%s %s %s %d %c %c", 
                    e.reference, e.type, e.centre, &e.capacite, &e.disponibilite, &e.etat) != EOF)
        {
            if(e.disponibilite == 'D' && e.etat == 'A')
            {
                char etat_str[10];
                if(e.etat == 'A') strcpy(etat_str, "Actif");
                else strcpy(etat_str, "Non actif");
                
                printf("%s\t%s\t%s\t%d\t%s\n", 
                       e.reference, e.type, e.centre, e.capacite, etat_str);
                count++;
            }
        }
        fclose(f);
        
    }
    else
    {
        printf("✗ Erreur : impossible d'ouvrir le fichier %s\n", filename);
    }
}

void filtrer_equipements_par_capacite(char *filename, int capacite_min)
{
    Equipement e;
    FILE *f = fopen(filename, "r");
    if(f != NULL)
    {
        printf("\n=== EQUIPEMENTS AVEC CAPACITE >= %d ===\n", capacite_min);
        printf("Reference\tType\t\tCentre\t\tCapacite\tDisponibilite\tEtat\n");
        printf("--------------------------------------------------------------------------------------------\n");
        
        int count = 0;
        while(fscanf(f, "%s %s %s %d %c %c", 
                    e.reference, e.type, e.centre, &e.capacite, &e.disponibilite, &e.etat) != EOF)
        {
            if(e.capacite >= capacite_min)
            {
                char disp_str[15], etat_str[10];
                if(e.disponibilite == 'D') strcpy(disp_str, "Disponible");
                else strcpy(disp_str, "Indisponible");
                if(e.etat == 'A') strcpy(etat_str, "Actif");
                else strcpy(etat_str, "Non actif");
                
                printf("%s\t%s\t%s\t%d\t%s\t%s\n", 
                       e.reference, e.type, e.centre, e.capacite, disp_str, etat_str);
                count++;
            }
        }
        fclose(f);
        printf("--------------------------------------------------------------------------------------------\n");
        printf("Total: %d équipements avec capacité >= %d\n", count, capacite_min);
    }
    else
    {
        printf("✗ Erreur : impossible d'ouvrir le fichier %s\n", filename);
    }
}

// ============================================================================
// FONCTIONS DE STATISTIQUES
// ============================================================================

void statistiques_equipements(char *filename)
{
    int total = compter_total_equipements(filename);
    printf("\n=== STATISTIQUES DES EQUIPEMENTS ===\n");
    printf("Nombre total d'equipements: %d\n", total);

    if(total == 0) {
        printf("Aucun équipement enregistré.\n");
        printf("==========================================\n");
        return;
    }

    FILE *f = fopen(filename, "r");
    if(f != NULL)
    {
        Equipement e;
        char types[10][30];
        int counts[10] = {0};
        int nb_types = 0;
        int actifs = 0, disponibles = 0;

        while(fscanf(f, "%s %s %s %d %c %c", 
                    e.reference, e.type, e.centre, &e.capacite, &e.disponibilite, &e.etat) != EOF)
        {
            // Compter par type
            int found = 0;
            for(int i = 0; i < nb_types; i++)
            {
                if(strcmp(types[i], e.type) == 0)
                {
                    counts[i]++;
                    found = 1;
                    break;
                }
            }
            if(!found && nb_types < 10)
            {
                strcpy(types[nb_types], e.type);
                counts[nb_types] = 1;
                nb_types++;
            }
            
            // Compter états
            if(e.etat == 'A') actifs++;
            if(e.disponibilite == 'D') disponibles++;
        }
        fclose(f);

        printf("\nRépartition par type:\n");
        for(int i = 0; i < nb_types; i++)
        {
            printf("  %s: %d equipements (%.1f%%)\n", 
                   types[i], counts[i], (counts[i] * 100.0) / total);
        }

        printf("\nEtat:\n");
        printf("  Actifs: %d (%.1f%%)\n", actifs, (actifs * 100.0) / total);
        printf("  Non actifs: %d (%.1f%%)\n", total - actifs, ((total - actifs) * 100.0) / total);
        
        printf("\nDisponibilite:\n");
        printf("  Disponibles: %d (%.1f%%)\n", disponibles, (disponibles * 100.0) / total);
        printf("  Indisponibles: %d (%.1f%%)\n", total - disponibles, ((total - disponibles) * 100.0) / total);
    }
    printf("==========================================\n");
}

// ============================================================================
// FONCTIONS DE RESERVATION
// ============================================================================

int reserver_equipement(char *filename_equipements, char *reference, 
                       char *date, int id_entraineur, char *nom_entraineur)
{
    // Vérifier si l'équipement existe et est disponible
    Equipement e = chercher_equipement(filename_equipements, reference);
    if (strcmp(e.reference, "") == 0)
    {
        printf("✗ Erreur : équipement '%s' non trouvé.\n", reference);
        return 0;
    }
    
    if (e.disponibilite != 'D')
    {
        printf("✗ Erreur : équipement '%s' n'est pas disponible.\n", reference);
        return 0;
    }
    
    // Créer la réservation dans reserve_equipement.txt (avec : séparateur)
    FILE *f_res = fopen("reserve_equipement.txt", "a");
    if (f_res != NULL)
    {
        fprintf(f_res, "%d:%s:%s:entraineur:Actif\n", id_entraineur, reference, date);
        fclose(f_res);
        
        // Marquer l'équipement comme réservé
        e.disponibilite = 'R';
        modifier_equipement(filename_equipements, (char*)reference, e);
        
        printf("✓ Réservation réussie !\n");
        return 1;
    }
    
    printf("✗ Erreur lors de la création de la réservation.\n");
    return 0;
}

void mes_reservations_equipements(int id_entraineur)
{
    char reference[20], date[20], type_user[20], statut[20];
    int user_id;
    FILE *f = fopen("reserve_equipement.txt", "r");
    int count = 0;
    
    if (f != NULL)
    {
        printf("\n=== MES RÉSERVATIONS D'ÉQUIPEMENTS (ID: %d) ===\n", id_entraineur);
        while (fscanf(f, "%d:%[^:]:%[^:]:%[^:]:%[^\n]", &user_id, reference, date, type_user, statut) != EOF)
        {
            if (user_id == id_entraineur && strcmp(type_user, "entraineur") == 0)
            {
                printf("Équipement: %s, Date: %s, Statut: %s\n", reference, date, statut);
                count++;
            }
        }
        fclose(f);
        printf("Total: %d réservation(s)\n", count);
    }
    else
    {
        printf("✗ Aucune réservation trouvée.\n");
    }
}


void afficher_statistiques_equipements_labels(GtkLabel *label_total, GtkLabel *label_mobilite, 
                                     GtkLabel *label_musculation, GtkLabel *label_cardio, 
                                     const char *filename) {
    int total = 0;
    int mobilite = 0, musculation = 0, cardio = 0;
    Equipement e;
    
    FILE *f = fopen(filename, "r");
    if (f != NULL) {
        while (fscanf(f, "%s %s %s %d %c %c", 
                    e.reference, e.type, e.centre, &e.capacite, &e.disponibilite, &e.etat) != EOF) {
            total++;
            
            // Catégoriser par type
            if (strstr(e.type, "Tapis") != NULL || strstr(e.type, "treadmill") != NULL || 
                strstr(e.type, "Bike") != NULL || strstr(e.type, "Vélo") != NULL ||
                strstr(e.type, "Rower") != NULL || strstr(e.type, "rameur") != NULL) {
                cardio++;
            }
            else if (strstr(e.type, "Dumbbell") != NULL || strstr(e.type, "Haltère") != NULL ||
                     strstr(e.type, "Bench") != NULL || strstr(e.type, " Banc") != NULL ||
                     strstr(e.type, "Kettlebell") != NULL) {
                musculation++;
            }
            else if (strstr(e.type, "Ball") != NULL || strstr(e.type, "Balle") != NULL ||
                     strstr(e.type, "Mat") != NULL || strstr(e.type, "Tap") != NULL) {
                mobilite++;
            }
        }
        fclose(f);
    }
    
    // Mettre à jour les labels
    char buffer[50];
    
    sprintf(buffer, "%d", total);
    gtk_label_set_text(label_total, buffer);
    
    sprintf(buffer, "%d", mobilite);
    gtk_label_set_text(label_mobilite, buffer);
    
    sprintf(buffer, "%d", musculation);
    gtk_label_set_text(label_musculation, buffer);
    
    sprintf(buffer, "%d", cardio);
    gtk_label_set_text(label_cardio, buffer);
}


