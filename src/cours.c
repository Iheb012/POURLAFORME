#include "cours.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// FONCTIONS DE BASE POUR COURS
// ============================================================================

int ajouter_cours(char *filename, Cours c)
{
    // Vérifier si l'ID existe déjà
    Cours existant = chercher_cours(filename, c.id);
    if (existant.id != -1)
    {
        printf("✗ Erreur : l'ID %d existe déjà.\n", c.id);
        return 0;
    }

    FILE *f = fopen(filename, "a");
    if (f != NULL)
    {
        fprintf(f, "%d %s %s %s %s %s\n",
                c.id, c.cours, c.coach, c.date_heure, c.salle, c.niveau);
        fclose(f);
        printf("✓ Cours ajouté avec succès (ID: %d)\n", c.id);
        return 1;
    }
    else
    {
        printf("✗ Erreur : impossible d'ouvrir le fichier %s\n", filename);
        return 0;
    }
}

Cours chercher_cours(char *filename, int id) {
    Cours c;
    c.id = -1;
    
    if(id <= 0) {
        return c;
    }
    
    FILE *f = fopen(filename, "r");
    if(f == NULL) {
        return c;
    }
    
    Cours temp;
    char ligne[512];
    int line_count = 0;
    int max_lines = 10000;
    
    while(fgets(ligne, sizeof(ligne), f) != NULL && line_count < max_lines) {
        line_count++;
        
        // Format: id cours coach date_heure salle niveau
        int scan_result = sscanf(ligne, "%d %s %s %s %s %s",
                                &temp.id, temp.cours, temp.coach, 
                                temp.date_heure, temp.salle, temp.niveau);
        
        if(scan_result == 6 && temp.id == id) {
            c = temp;
            fclose(f);
            return c;
        }
    }
    
    fclose(f);
    c.id = -1;
    return c;
}

// Modifier un cours - VERSION ADAPTÉE
int modifier_cours(char *filename, int id, Cours nouv) {
    if(id <= 0) {
        return 0;
    }
    
    FILE *f = fopen(filename, "r");
    if(f == NULL) {
        return 0;
    }
    
    FILE *f2 = fopen("temp_cours.txt", "w");
    if(f2 == NULL) {
        fclose(f);
        return 0;
    }
    
    Cours c;
    int found = 0;
    char ligne[512];
    int line_count = 0;
    int max_lines = 10000;
    
    while(fgets(ligne, sizeof(ligne), f) != NULL && line_count < max_lines) {
        line_count++;
        
        int scan_result = sscanf(ligne, "%d %s %s %s %s %s",
                                &c.id, c.cours, c.coach, 
                                c.date_heure, c.salle, c.niveau);
        
        if(scan_result == 6) {
            if(c.id == id) {
                fprintf(f2, "%d %s %s %s %s %s\n",
                       nouv.id, nouv.cours, nouv.coach, 
                       nouv.date_heure, nouv.salle, nouv.niveau);
                found = 1;
            } else {
                fprintf(f2, "%d %s %s %s %s %s\n",
                       c.id, c.cours, c.coach, 
                       c.date_heure, c.salle, c.niveau);
            }
        } else {
            fputs(ligne, f2);
        }
    }
    
    fclose(f);
    fclose(f2);
    
    if(!found) {
        remove("temp_cours.txt");
        return 0;
    }
    
    if(remove(filename) != 0) {
        remove("temp_cours.txt");
        return 0;
    }
    
    if(rename("temp_cours.txt", filename) != 0) {
        return 0;
    }
    
    return 1;
}

// Supprimer un cours - VERSION ADAPTÉE
int supprimer_cours(char *filename, int id) {
    if(id <= 0) {
        return 0;
    }
    
    FILE *f = fopen(filename, "r");
    if(f == NULL) {
        return 0;
    }
    
    FILE *f2 = fopen("temp_cours.txt", "w");
    if(f2 == NULL) {
        fclose(f);
        return 0;
    }
    
    Cours c;
    int found = 0;
    char ligne[512];
    int line_count = 0;
    int max_lines = 10000;
    
    while(fgets(ligne, sizeof(ligne), f) != NULL && line_count < max_lines) {
        line_count++;
        
        int scan_result = sscanf(ligne, "%d %s %s %s %s %s",
                                &c.id, c.cours, c.coach, 
                                c.date_heure, c.salle, c.niveau);
        
        if(scan_result == 6) {
            if(c.id != id) {
                fprintf(f2, "%d %s %s %s %s %s\n",
                       c.id, c.cours, c.coach, 
                       c.date_heure, c.salle, c.niveau);
            } else {
                found = 1;
            }
        } else {
            fputs(ligne, f2);
        }
    }
    
    fclose(f);
    fclose(f2);
    
    if(!found) {
        remove("temp_cours.txt");
        return 0;
    }
    
    if(remove(filename) != 0) {
        remove("temp_cours.txt");
        return 0;
    }
    
    if(rename("temp_cours.txt", filename) != 0) {
        return 0;
    }
    
    return 1;
}
// ============================================================================
// FONCTIONS DE COMPTAGE
// ============================================================================

int nombre_cours(char *filename)
{
    int count = 0;
    Cours c;
    FILE *f = fopen(filename, "r");
    if (f != NULL)
    {
        while (fscanf(f, "%d %s %s %s %s %s",
                      &c.id, c.cours, c.coach, c.date_heure, c.salle, c.niveau) != EOF)
            count++;
        fclose(f);
    }
    return count;
}

int cours_par_niveau(char *filename, char *niveau)
{
    int count = 0;
    Cours c;
    FILE *f = fopen(filename, "r");
    if (f != NULL)
    {
        while (fscanf(f, "%d %s %s %s %s %s",
                      &c.id, c.cours, c.coach, c.date_heure, c.salle, c.niveau) != EOF)
        {
            if (strcmp(c.niveau, niveau) == 0)
                count++;
        }
        fclose(f);
    }
    return count;
}

int cours_par_coach(char *filename, char *coach)
{
    int count = 0;
    Cours c;
    FILE *f = fopen(filename, "r");
    if (f != NULL)
    {
        while (fscanf(f, "%d %s %s %s %s %s",
                      &c.id, c.cours, c.coach, c.date_heure, c.salle, c.niveau) != EOF)
        {
            if (strcmp(c.coach, coach) == 0)
                count++;
        }
        fclose(f);
    }
    return count;
}

// ============================================================================
// FONCTIONS D'AFFICHAGE
// ============================================================================

void afficher_tous_cours(char *filename)
{
    Cours c;
    FILE *f = fopen(filename, "r");
    
    if(f != NULL)
    {
        printf("\n=== TOUS LES COURS ===\n");
        printf("ID\tCours\t\tCoach\t\tSalle\tDate/Heure\t\tNiveau\n");
        printf("--------------------------------------------------------------------------------\n");
        
        while(fscanf(f, "%d %s %s %s %s %s", 
                    &c.id, c.cours, c.coach, c.date_heure, c.salle, c.niveau) != EOF)
        {
            printf("%d\t%s\t%s\t%s\t%s\t%s\n",
                   c.id, c.cours, c.coach, c.salle, c.date_heure, c.niveau);
        }
        printf("--------------------------------------------------------------------------------\n");
        printf("Total: %d cours\n", nombre_cours(filename));
        fclose(f);
    }
    else
    {
        printf("✗ Erreur : impossible d'ouvrir le fichier %s\n", filename);
    }
}

void afficher_cours_haut_niveau(char *filename)
{
    Cours c;
    FILE *f = fopen(filename, "r");
    
    if(f != NULL)
    {
        printf("\n=== COURS HAUT NIVEAU ===\n");
        printf("ID\tCours\t\tCoach\t\tSalle\tDate/Heure\n");
        printf("----------------------------------------------------------------\n");
        
        int count = 0;
        while(fscanf(f, "%d %s %s %s %s %s", 
                    &c.id, c.cours, c.coach, c.date_heure, c.salle, c.niveau) != EOF)
        {
            if(strstr(c.niveau, "Avancé") != NULL || strstr(c.niveau, "Expert") != NULL || 
               strstr(c.niveau, "Pro") != NULL || strstr(c.niveau, "Haut") != NULL)
            {
                printf("%d\t%s\t%s\t%s\t%s\n", 
                       c.id, c.cours, c.coach, c.salle, c.date_heure);
                count++;
            }
        }
        printf("----------------------------------------------------------------\n");
        printf("Total cours haut niveau: %d\n", count);
        fclose(f);
    }
}

// ============================================================================
// FONCTIONS DE STATISTIQUES
// ============================================================================

void statistiques_completes_cours(char *filename)
{
    int total = nombre_cours(filename);
    printf("\n=== STATISTIQUES DES COURS SPORTIFS ===\n");
    printf("Nombre total de cours: %d\n", total);

    if (total == 0) {
        printf("Aucun cours enregistré.\n");
        printf("=======================================\n");
        return;
    }

    FILE *f = fopen(filename, "r");
    if (f != NULL)
    {
        Cours c;
        char niveaux[10][20];
        char coaches[10][30];
        char salles[10][20];
        int counts_niveaux[10] = {0};
        int counts_coaches[10] = {0};
        int counts_salles[10] = {0};
        int nb_niveaux = 0, nb_coaches = 0, nb_salles = 0;

        while (fscanf(f, "%d %s %s %s %s %s",
                      &c.id, c.cours, c.coach, c.date_heure, c.salle, c.niveau) != EOF)
        {
            // Compter par niveau
            int found = 0;
            for (int i = 0; i < nb_niveaux; i++)
            {
                if (strcmp(niveaux[i], c.niveau) == 0)
                {
                    counts_niveaux[i]++;
                    found = 1;
                    break;
                }
            }
            if (!found && nb_niveaux < 10)
            {
                strcpy(niveaux[nb_niveaux], c.niveau);
                counts_niveaux[nb_niveaux] = 1;
                nb_niveaux++;
            }

            // Compter par coach
            found = 0;
            for (int i = 0; i < nb_coaches; i++)
            {
                if (strcmp(coaches[i], c.coach) == 0)
                {
                    counts_coaches[i]++;
                    found = 1;
                    break;
                }
            }
            if (!found && nb_coaches < 10)
            {
                strcpy(coaches[nb_coaches], c.coach);
                counts_coaches[nb_coaches] = 1;
                nb_coaches++;
            }

            // Compter par salle
            found = 0;
            for (int i = 0; i < nb_salles; i++)
            {
                if (strcmp(salles[i], c.salle) == 0)
                {
                    counts_salles[i]++;
                    found = 1;
                    break;
                }
            }
            if (!found && nb_salles < 10)
            {
                strcpy(salles[nb_salles], c.salle);
                counts_salles[nb_salles] = 1;
                nb_salles++;
            }
        }
        fclose(f);

        printf("\nRépartition par niveau:\n");
        for (int i = 0; i < nb_niveaux; i++)
        {
            printf("  %s: %d cours (%.1f%%)\n", 
                   niveaux[i], counts_niveaux[i], (counts_niveaux[i] * 100.0) / total);
        }

        printf("\nRépartition par coach:\n");
        for (int i = 0; i < nb_coaches; i++)
        {
            printf("  %s: %d cours (%.1f%%)\n", 
                   coaches[i], counts_coaches[i], (counts_coaches[i] * 100.0) / total);
        }

        printf("\nRépartition par salle:\n");
        for (int i = 0; i < nb_salles; i++)
        {
            printf("  %s: %d cours (%.1f%%)\n", 
                   salles[i], counts_salles[i], (counts_salles[i] * 100.0) / total);
        }
    }
    printf("=======================================\n");
}

// ============================================================================
// FONCTIONS D'INSCRIPTION (MODIFIEES AVEC LES BONS FICHIERS)
// ============================================================================

int sinscrire_cours(char *filename, int id_membre, int id_cours, char *date_inscription)
{
    // Vérifier si le membre existe dans membres.txt
    FILE *f_mem = fopen("membres.txt", "r");
    int membre_trouve = 0;
    char nom_membre[30], prenom_membre[30];
    if (f_mem != NULL)
    {
        int id, age;
        char nom[30], prenom[30], sexe[10], email[50], tel[15], date_inscr[20], statut[20];
        while (fscanf(f_mem, "%d:%[^:]:%[^:]:%d:%[^:]:%[^:]:%[^:]:%[^:]:%[^\n]", 
                      &id, nom, prenom, &age, sexe, email, tel, date_inscr, statut) != EOF)
        {
            if (id == id_membre && strcmp(statut, "Actif") == 0)
            {
                membre_trouve = 1;
                strcpy(nom_membre, nom);
                strcpy(prenom_membre, prenom);
                break;
            }
        }
        fclose(f_mem);
    }
    
    if (!membre_trouve)
    {
        printf("✗ Erreur : membre avec ID %d non trouvé ou inactif dans membres.txt.\n", id_membre);
        return 0;
    }

    // Vérifier si le cours existe dans cours.txt
    Cours c = chercher_cours("cours.txt", id_cours);
    if (c.id == -1)
    {
        printf("✗ Erreur : cours avec ID %d non trouvé dans cours.txt.\n", id_cours);
        return 0;
    }

    // Vérifier si le membre est déjà inscrit à ce cours dans inscri_membre_cours.txt
    FILE *f_check = fopen("inscri_membre_cours.txt", "r");
    if (f_check != NULL)
    {
        int id_m, id_c;
        char date_inscr[30], statut[20];
        
        while (fscanf(f_check, "%d:%d:%[^:]:%[^\n]", &id_m, &id_c, date_inscr, statut) != EOF)
        {
            if (id_m == id_membre && id_c == id_cours && strcmp(statut, "Actif") == 0)
            {
                printf("✗ Erreur : Le membre est déjà inscrit à ce cours.\n");
                fclose(f_check);
                return 0;
            }
        }
        fclose(f_check);
    }

    // Créer l'inscription dans inscri_membre_cours.txt
    FILE *f = fopen("inscri_membre_cours.txt", "a");
    if (f != NULL) {
        fprintf(f, "%d:%d:%s:Actif\n", id_membre, id_cours, date_inscription);
        fclose(f);
        printf("✓ Inscription réussie !\n");
        printf("  Membre: %s %s (ID: %d)\n", prenom_membre, nom_membre, id_membre);
        printf("  Cours: %s (ID: %d)\n", c.cours, id_cours);
        printf("  Coach: %s\n", c.coach);
        printf("  Date/Heure: %s\n", c.date_heure);
        printf("  Date d'inscription: %s\n", date_inscription);
        return 1;
    }
    printf("✗ Erreur : impossible d'ouvrir le fichier inscri_membre_cours.txt\n");
    return 0;
}

int verifier_inscription(char *filename, int id_membre, int id_cours)
{
    int id_m, id_c;
    char date_inscription[30], statut[20];
    
    FILE *f = fopen("inscri_membre_cours.txt", "r");
    if (f != NULL) {
        while (fscanf(f, "%d:%d:%[^:]:%[^\n]", &id_m, &id_c, date_inscription, statut) != EOF) {
            if (id_m == id_membre && id_c == id_cours && strcmp(statut, "Actif") == 0) {
                fclose(f);
                return 1;
            }
        }
        fclose(f);
    }
    return 0;
}

void afficher_inscriptions_membre(char *filename_inscriptions, int id_membre, char *filename_cours)
{
    int id_m, id_c;
    char date_inscription[30], statut[20];
    Cours c;
    int trouve = 0;
    
    FILE *f = fopen("inscri_membre_cours.txt", "r");
    if (f != NULL) {
        printf("\n=== COURS INSCRITS DU MEMBRE ID %d ===\n", id_membre);
        while (fscanf(f, "%d:%d:%[^:]:%[^\n]", &id_m, &id_c, date_inscription, statut) != EOF) {
            if (id_m == id_membre && strcmp(statut, "Actif") == 0) {
                c = chercher_cours("cours.txt", id_c);
                if (c.id != -1) {
                    printf("• %s (Coach: %s)\n", c.cours, c.coach);
                    printf("  Salle: %s, Date/Heure: %s\n", c.salle, c.date_heure);
                    printf("  Niveau: %s\n", c.niveau);
                    printf("  Inscrit le: %s\n", date_inscription);
                    printf("  --------------------\n");
                    trouve = 1;
                }
            }
        }
        fclose(f);
        
        if (!trouve) {
            printf("Aucune inscription trouvée pour ce membre\n");
        }
    } else {
        printf("✗ Erreur : impossible d'ouvrir inscri_membre_cours.txt\n");
    }
}
