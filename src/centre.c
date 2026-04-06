#include "centre.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// ============================================================================
// FONCTIONS DE BASE CORRIGÉES
// ============================================================================

int ajouter_centre(const char *filename, Centre c)
{
    FILE *f = fopen(filename, "a");
    if(f != NULL)
    {
        fprintf(f, "%d;%s;%s;%s;%d;%c\n", 
                c.id, c.nom, c.adresse, c.ville, c.capacite, c.etat);
        fclose(f);
        printf("✓ Centre ajouté avec succès: ID=%d, Nom=%s\n", c.id, c.nom);
        return 1;
    }
    else 
    {
        printf("✗ Erreur : impossible d'ouvrir le fichier %s\n", filename);
        return 0;
    }
}

int modifier_centre(const char *filename, int id, Centre nouv)
{
    int tr = 0;
    Centre c;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("temp_centre.txt", "w");
    
    if(f && f2)
    {
        while(fscanf(f, "%d;%99[^;];%199[^;];%49[^;];%d;%c\n", 
                    &c.id, c.nom, c.adresse, c.ville, &c.capacite, &c.etat) != EOF)
        {
            if(c.id == id)
            {
                fprintf(f2, "%d;%s;%s;%s;%d;%c\n", 
                        nouv.id, nouv.nom, nouv.adresse, nouv.ville, nouv.capacite, nouv.etat);
                tr = 1;
            }
            else
            {
                fprintf(f2, "%d;%s;%s;%s;%d;%c\n", 
                        c.id, c.nom, c.adresse, c.ville, c.capacite, c.etat);
            }
        }
        fclose(f);
        fclose(f2);
        
        remove(filename);
        rename("temp_centre.txt", filename);
    }
    
    if(tr) {
        printf("✓ Centre '%d' modifié avec succès\n", id);
    } else {
        printf("✗ Centre '%d' non trouvé\n", id);
    }
    
    return tr;
}

int supprimer_centre(const char *filename, int id)
{
    int tr = 0;
    Centre c;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("temp_centre.txt", "w");
    
    if(f && f2)
    {
        while(fscanf(f, "%d;%99[^;];%199[^;];%49[^;];%d;%c\n", 
                    &c.id, c.nom, c.adresse, c.ville, &c.capacite, &c.etat) != EOF)
        {
            if(c.id == id) {
                tr = 1;
            } else {
                fprintf(f2, "%d;%s;%s;%s;%d;%c\n", 
                        c.id, c.nom, c.adresse, c.ville, c.capacite, c.etat);
            }
        }
        fclose(f);
        fclose(f2);
        
        if(tr) {
            remove(filename);
            rename("temp_centre.txt", filename);
        } else {
            remove("temp_centre.txt");
        }
    }
    
    if(tr) {
        printf("✓ Centre '%d' supprimé avec succès\n", id);
    } else {
        printf("✗ Centre '%d' non trouvé\n", id);
    }
    
    return tr;
}

Centre chercher_centre(const char *filename, int id)
{
    Centre c; 
    int tr = 0;
    FILE *f = fopen(filename, "r");
    
    // Initialiser la structure
    c.id = -1;
    strcpy(c.nom, "");
    strcpy(c.adresse, "");
    strcpy(c.ville, "");
    c.capacite = 0;
    c.etat = 'I';
    
    if(f != NULL)
    {
        while(!tr && fscanf(f, "%d;%99[^;];%199[^;];%49[^;];%d;%c\n", 
                           &c.id, c.nom, c.adresse, c.ville, &c.capacite, &c.etat) != EOF)
        {
            if(c.id == id) {
                tr = 1;
                break;
            }
        }
        fclose(f);
    }
    
    if(!tr) {
        c.id = -1;
    }
    
    return c;
}


// ============================================================================
// FONCTIONS DE COMPTAGE CORRIGÉES
// ============================================================================

int nombre_centres(const char *filename)
{
    Centre c;
    int count = 0;
    FILE *f = fopen(filename, "r");
    if(f != NULL)
    {
        while(fscanf(f, "%d;%99[^;];%199[^;];%49[^;];%d;%c\n", 
                    &c.id, c.nom, c.adresse, c.ville, &c.capacite, &c.etat) != EOF)
        {
            count++;
        }
        fclose(f);
    }
    return count;
}

// ============================================================================
// FONCTIONS D'AFFICHAGE CORRIGÉES
// ============================================================================

void afficher_centres_actifs_seulement(const char *filename)
{
    Centre c;
    FILE *f = fopen(filename, "r");
    if(f != NULL)
    {
        printf("\n=== CENTRES ACTIFS ===\n");
        printf("ID\tNom\t\tAdresse\t\tVille\t\tCapacite\n");
        printf("----------------------------------------------------------------\n");
        
        int count = 0;
        while(fscanf(f, "%d;%99[^;];%199[^;];%49[^;];%d;%c\n", 
                    &c.id, c.nom, c.adresse, c.ville, &c.capacite, &c.etat) != EOF)
        {
            if(c.etat == 'A')
            {
                printf("%d\t%s\t%s\t%s\t%d\n", 
                       c.id, c.nom, c.adresse, c.ville, c.capacite);
                count++;
            }
        }
        fclose(f);
        printf("----------------------------------------------------------------\n");
        printf("Total actifs: %d centres\n", count);
    }
    else
    {
        printf("✗ Erreur : impossible d'ouvrir le fichier %s\n", filename);
    }
}

void afficher_tous_centres_tableau(const char *filename)
{
    Centre c;
    FILE *f = fopen(filename, "r");
    
    if(f != NULL)
    {
        printf("\n+----+----------------------+--------------------+----------------+----------+--------+\n");
        printf("| ID |         Nom          |      Adresse       |     Ville      | Capacité |  État  |\n");
        printf("+----+----------------------+--------------------+----------------+----------+--------+\n");
        
        while(fscanf(f, "%d;%99[^;];%199[^;];%49[^;];%d;%c\n", 
                    &c.id, c.nom, c.adresse, c.ville, &c.capacite, &c.etat) != EOF)
        {
            char etat_str[10];
            if(c.etat == 'A') strcpy(etat_str, "Actif");
            else strcpy(etat_str, "Inactif");
            
            printf("| %-2d | %-20s | %-18s | %-14s | %-8d | %-6s |\n",
                   c.id, c.nom, c.adresse, c.ville, c.capacite, etat_str);
        }
        printf("+----+----------------------+--------------------+----------------+----------+--------+\n");
        fclose(f);
    }
    else
    {
        printf("✗ Erreur : impossible d'ouvrir le fichier %s\n", filename);
    }
}

// ============================================================================
// FONCTIONS DE STATISTIQUES CORRIGÉES
// ============================================================================

void statistiques_completes_centre(const char *filename)
{
    int total = nombre_centres(filename);
    int actifs = 0;
    
    printf("\n=== STATISTIQUES DES CENTRES ===\n");
    printf("Nombre total de centres: %d\n", total);
    
    if(total == 0) {
        printf("Aucun centre enregistré.\n");
        printf("==========================================\n");
        return;
    }

    // Compter les centres actifs
    Centre c;
    FILE *f = fopen(filename, "r");
    if(f != NULL)
    {
        while(fscanf(f, "%d;%99[^;];%199[^;];%49[^;];%d;%c\n", 
                    &c.id, c.nom, c.adresse, c.ville, &c.capacite, &c.etat) != EOF)
        {
            if(c.etat == 'A')
                actifs++;
        }
        fclose(f);
    }

    printf("Centres actifs: %d (%.1f%%)\n", actifs, (actifs * 100.0) / total);
    printf("Centres inactifs: %d (%.1f%%)\n", total - actifs, ((total - actifs) * 100.0) / total);
    
    // Statistiques par ville
    f = fopen(filename, "r");
    if(f != NULL)
    {
        char villes[10][30];
        int counts[10] = {0};
        int nb_villes = 0;

        while(fscanf(f, "%d;%99[^;];%199[^;];%49[^;];%d;%c\n", 
                    &c.id, c.nom, c.adresse, c.ville, &c.capacite, &c.etat) != EOF)
        {
            int found = 0;
            for(int i = 0; i < nb_villes; i++)
            {
                if(strcmp(villes[i], c.ville) == 0)
                {
                    counts[i]++;
                    found = 1;
                    break;
                }
            }
            if(!found && nb_villes < 10)
            {
                strcpy(villes[nb_villes], c.ville);
                counts[nb_villes] = 1;
                nb_villes++;
            }
        }
        fclose(f);

        printf("\nRepartition par ville:\n");
        for(int i = 0; i < nb_villes; i++)
        {
            printf("  %s: %d centres (%.1f%%)\n", 
                   villes[i], counts[i], (counts[i] * 100.0) / total);
        }
    }
    printf("==========================================\n");
}

void afficher_statistiques_centres(const char *filename)
{
    statistiques_completes_centre(filename);
}

// ============================================================================
// FONCTIONS D'INSCRIPTION CORRIGÉES
// ============================================================================

int inscrire_entraineur_centre(const char *filename_inscriptions, int id_entraineur, int id_centre, char *date_inscription)
{
    // Vérifier si l'entraineur existe dans entraineurs.txt
    FILE *f_ent = fopen("entraineurs.txt", "r");
    int entraineur_trouve = 0;
    char nom_entraineur[30], prenom_entraineur[30];
    if (f_ent != NULL)
    {
        int id;
        char nom[30], prenom[30], specialite[30], email[50], telephone[15], statut[20];
        int experience, age;
        char sexe;
        
        // Format de lecture cohérent avec points-virgules
        while (fscanf(f_ent, "%d;%29[^;];%29[^;];%29[^;];%d;%d;%c;%49[^;];%14[^;];%19[^\n]\n", 
                      &id, prenom, nom, specialite, &experience, &age, &sexe, email, telephone, statut) != EOF)
        {
            if (id == id_entraineur && strcmp(statut, "Actif") == 0)
            {
                entraineur_trouve = 1;
                strcpy(nom_entraineur, nom);
                strcpy(prenom_entraineur, prenom);
                break;
            }
        }
        fclose(f_ent);
    }
    
    if (!entraineur_trouve)
    {
        printf("✗ Erreur : entraineur avec ID %d non trouvé ou inactif dans entraineurs.txt.\n", id_entraineur);
        return 0;
    }
    
    // Vérifier si le centre existe dans centres.txt
    Centre c = chercher_centre("centres.txt", id_centre);
    if (c.id == -1)
    {
        printf("✗ Erreur : centre avec ID %d non trouvé dans centres.txt.\n", id_centre);
        return 0;
    }
    
    // Vérifier si l'entraineur est déjà inscrit à ce centre dans inscri_entraineur_centre.txt
    FILE *f_check = fopen("inscri_entraineur_centre.txt", "r");
    if (f_check != NULL)
    {
        int id_e, id_c;
        char date_inscr[30], statut[20];
        
        // Format de lecture cohérent
        while (fscanf(f_check, "%d;%d;%29[^;];%19[^\n]\n", &id_e, &id_c, date_inscr, statut) != EOF)
        {
            if (id_e == id_entraineur && id_c == id_centre && strcmp(statut, "Actif") == 0)
            {
                printf("✗ Erreur : vous êtes déjà inscrit à ce centre.\n");
                fclose(f_check);
                return 0;
            }
        }
        fclose(f_check);
    }
    
    // Créer l'inscription dans inscri_entraineur_centre.txt
    FILE *f_ins = fopen("inscri_entraineur_centre.txt", "a");
    if (f_ins != NULL)
    {
        // Format d'écriture cohérent
        fprintf(f_ins, "%d;%d;%s;Actif\n", id_entraineur, id_centre, date_inscription);
        fclose(f_ins);
        
        printf("✓ Inscription réussie !\n");
        printf("  Entraineur: %s %s (ID: %d)\n", prenom_entraineur, nom_entraineur, id_entraineur);
        printf("  Centre: %s\n", c.nom);
        printf("  Adresse: %s, %s\n", c.adresse, c.ville);
        printf("  Capacité: %d\n", c.capacite);
        printf("  Date d'inscription: %s\n", date_inscription);
        
        return 1;
    }
    
    printf("✗ Erreur lors de la création de l'inscription dans inscri_entraineur_centre.txt\n");
    return 0;
}

void mes_inscriptions_centres(const char *filename_inscriptions, int id_entraineur, const char *filename_centres)
{
    int id_centre, id_e;
    char date_inscription[30], statut[20];
    FILE *f = fopen("inscri_entraineur_centre.txt", "r");
    int count = 0;
    
    if (f != NULL)
    {
        printf("\n=== MES INSCRIPTIONS AUX CENTRES (ID: %d) ===\n", id_entraineur);
        printf("Centre\t\tVille\t\tDate Inscription\tStatut\n");
        printf("------------------------------------------------------------\n");
        
        // Format de lecture cohérent
        while (fscanf(f, "%d;%d;%29[^;];%19[^\n]\n", &id_e, &id_centre, date_inscription, statut) != EOF)
        {
            if (id_e == id_entraineur)
            {
                // Récupérer les détails du centre
                Centre c = chercher_centre("centres.txt", id_centre);
                if (c.id != -1)
                {
                    printf("%s\t%s\t%s\t%s\n", c.nom, c.ville, date_inscription, statut);
                    count++;
                }
            }
        }
        fclose(f);
        
        printf("------------------------------------------------------------\n");
        printf("Total: %d inscription(s)\n", count);
    }
    else
    {
        printf("✗ Aucune inscription trouvée pour cet entraineur dans inscri_entraineur_centre.txt\n");
    }
}

// ============================================================================
// FONCTION DE TEST ET DIAGNOSTIC
// ============================================================================

void tester_fonctions_centre() {
    printf("=== TEST FONCTIONS CENTRE ===\n");
    
    // Créer un centre de test
    Centre test;
    test.id = 999;
    strcpy(test.nom, "Centre Test");
    strcpy(test.adresse, "123 Rue Test");
    strcpy(test.ville, "Tunis");
    test.capacite = 100;
    test.etat = 'A';
    
    printf("Test avec centre: ID=%d, Nom=%s\n", test.id, test.nom);
    
    // Test ajout
    printf("1. Test ajout... ");
    if (ajouter_centre("centres.txt", test)) {
        printf("✓ Réussi\n");
    } else {
        printf("✗ Échec\n");
    }
    
    // Test recherche
    printf("2. Test recherche... ");
    Centre trouve = chercher_centre("centres.txt", 999);
    if (trouve.id != -1) {
        printf("✓ Réussi: %s\n", trouve.nom);
    } else {
        printf("✗ Échec\n");
    }
    
    // Test modification
    printf("3. Test modification... ");
    strcpy(test.nom, "Centre Test Modifié");
    if (modifier_centre("centres.txt", 999, test)) {
        printf("✓ Réussi\n");
    } else {
        printf("✗ Échec\n");
    }
    
    // Vérification modification
    printf("4. Vérification modification... ");
    trouve = chercher_centre("centres.txt", 999);
    if (trouve.id != -1 && strcmp(trouve.nom, "Centre Test Modifié") == 0) {
        printf("✓ Réussi: %s\n", trouve.nom);
    } else {
        printf("✗ Échec\n");
    }
    
    // Test suppression
    printf("5. Test suppression... ");
    if (supprimer_centre("centres.txt", 999)) {
        printf("✓ Réussi\n");
    } else {
        printf("✗ Échec\n");
    }
    
    // Vérification suppression
    printf("6. Vérification suppression... ");
    trouve = chercher_centre("centres.txt", 999);
    if (trouve.id == -1) {
        printf("✓ Réussi (centre supprimé)\n");
    } else {
        printf("✗ Échec (centre toujours présent)\n");
    }
    
    printf("=== FIN TEST ===\n");
}

// ============================================================================
// FONCTION POUR LISTER TOUS LES CENTRES
// ============================================================================

void lister_tous_centres(const char *filename) {
    Centre c;
    FILE *f = fopen(filename, "r");
    
    if(f != NULL) {
        printf("\n=== LISTE DE TOUS LES CENTRES ===\n");
        int count = 0;
        
        while(fscanf(f, "%d;%99[^;];%199[^;];%49[^;];%d;%c\n", 
                    &c.id, c.nom, c.adresse, c.ville, &c.capacite, &c.etat) != EOF) {
            printf("%d. %s - %s, %s (Capacité: %d, État: %s)\n", 
                   c.id, c.nom, c.adresse, c.ville, c.capacite, 
                   (c.etat == 'A') ? "Actif" : "Inactif");
            count++;
        }
        fclose(f);
        printf("Total: %d centres\n", count);
    } else {
        printf("✗ Erreur : impossible d'ouvrir le fichier %s\n", filename);
    }
}

// ============================================================================
// FONCTION POUR VÉRIFIER LA DISPONIBILITÉ D'UN CENTRE
// ============================================================================

int centre_est_actif(const char *filename, int id_centre) {
    Centre c = chercher_centre(filename, id_centre);
    return (c.id != -1 && c.etat == 'A');
}

// ============================================================================
// FONCTION POUR OBTENIR LES CENTRES PAR VILLE
// ============================================================================

void centres_par_ville(const char *filename, const char *ville) {
    Centre c;
    FILE *f = fopen(filename, "r");
    int count = 0;
    
    if(f != NULL) {
        printf("\n=== CENTRES À %s ===\n", ville);
        
        while(fscanf(f, "%d;%99[^;];%199[^;];%49[^;];%d;%c\n", 
                    &c.id, c.nom, c.adresse, c.ville, &c.capacite, &c.etat) != EOF) {
            if(strcmp(c.ville, ville) == 0 && c.etat == 'A') {
                printf("%d. %s - %s (Capacité: %d)\n", 
                       c.id, c.nom, c.adresse, c.capacite);
                count++;
            }
        }
        fclose(f);
        printf("Total: %d centres actifs à %s\n", count, ville);
    } else {
        printf("✗ Erreur : impossible d'ouvrir le fichier %s\n", filename);
    }
}
