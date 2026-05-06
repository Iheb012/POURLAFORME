#include "membre.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

// ============================================================================
// FONCTIONS DE GESTION DES MEMBRES - FORMAT AVEC ESPACES (INCHANGÉ)
// ============================================================================

// Ajouter un membre - FORMAT ORIGINAL
int ajouter_membre(char *filename, Membre m) {
    FILE *f = fopen(filename, "a");
    if(f == NULL) return 0;
    
    fprintf(f, "%d %s %s %d %s %s %.2f\n", 
           m.id, m.nom, m.email, m.age, m.sexe, m.Typeabonnement, m.Tarif);
    fclose(f);
    return 1;
}


// Chercher un membre par ID
Membre chercher_membre(char *filename, int id) {
    Membre m;
    m.id = -1;
    
    FILE *f = fopen(filename, "r");
    if(f == NULL) return m;
    
    char ligne[512];
    Membre temp;
    
    while(fgets(ligne, sizeof(ligne), f) != NULL) {
        if(parse_membre_ligne(ligne, &temp)) {
            if(temp.id == id) {
                m = temp;
                fclose(f);
                return m;
            }
        }
    }
    
    fclose(f);
    m.id = -1;
    return m;
}

// Fonction helper pour parser une ligne - VERSION CORRIGÉE SANS DEBUG
int parse_membre_ligne(char *ligne, Membre *m) {
    // Réinitialiser la structure
    memset(m, 0, sizeof(Membre));
    m->id = -1;
    
    // Faire une copie pour ne pas modifier l'original
    char ligne_copy[512];
    strncpy(ligne_copy, ligne, sizeof(ligne_copy) - 1);
    ligne_copy[sizeof(ligne_copy) - 1] = '\0';
    
    // Supprimer le saut de ligne
    ligne_copy[strcspn(ligne_copy, "\n")] = 0;
    
    // APPROCHE ROBUSTE : compter les champs manuellement
    char *tokens[10];
    int token_count = 0;
    char *token = strtok(ligne_copy, " \t");
    
    while (token != NULL && token_count < 10) {
        tokens[token_count++] = token;
        token = strtok(NULL, " \t");
    }
    
    // Vérifier qu'on a assez de tokens
    if (token_count < 7) {
        return 0;
    }
    
    // Parser les tokens
    m->id = atoi(tokens[0]);
    strncpy(m->nom, tokens[1], sizeof(m->nom) - 1);
    m->nom[sizeof(m->nom) - 1] = '\0';
    
    strncpy(m->email, tokens[2], sizeof(m->email) - 1);
    m->email[sizeof(m->email) - 1] = '\0';
    
    m->age = atoi(tokens[3]);
    
    // GESTION SPÉCIALE DU SEXE : "Non spécifié" peut être 2 tokens
    if (token_count >= 7) {
        // Vérifier si on a "Non" suivi de "spécifié"
        if (strcmp(tokens[4], "Non") == 0 && token_count >= 8 && strcmp(tokens[5], "spécifié") == 0) {
            // Cas "Non spécifié"
            strncpy(m->sexe, "Non spécifié", sizeof(m->sexe) - 1);
            m->sexe[sizeof(m->sexe) - 1] = '\0';
            
            strncpy(m->Typeabonnement, tokens[6], sizeof(m->Typeabonnement) - 1);
            m->Typeabonnement[sizeof(m->Typeabonnement) - 1] = '\0';
            
            m->Tarif = atof(tokens[7]);
        } else {
            // Cas normal - sexe en un seul mot
            strncpy(m->sexe, tokens[4], sizeof(m->sexe) - 1);
            m->sexe[sizeof(m->sexe) - 1] = '\0';
            
            strncpy(m->Typeabonnement, tokens[5], sizeof(m->Typeabonnement) - 1);
            m->Typeabonnement[sizeof(m->Typeabonnement) - 1] = '\0';
            
            m->Tarif = atof(tokens[6]);
        }
    }
    
    return 1;
}

// Chercher un membre par nom
Membre chercher_membre_par_nom(char *filename, const char *nom) {
    Membre m;
    m.id = -1;
    
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        return m;
    }
    
    Membre temp;
    char ligne[512];
    int trouve = 0;
    
    while(fgets(ligne, sizeof(ligne), f) != NULL) {
        if(parse_membre_ligne(ligne, &temp)) {
            // Recherche insensible à la casse
            char nom_lower[100], search_lower[100];
            
            int i;
            for (i = 0; temp.nom[i] && i < 99; i++) {
                nom_lower[i] = tolower(temp.nom[i]);
            }
            nom_lower[i] = '\0';
            
            for (i = 0; nom[i] && i < 99; i++) {
                search_lower[i] = tolower(nom[i]);
            }
            search_lower[i] = '\0';
            
            if (strstr(nom_lower, search_lower) != NULL) {
                m = temp;
                trouve = 1;
                break;
            }
        }
    }
    
    fclose(f);
    
    if (!trouve) {
        m.id = -1;
    }
    
    return m;
}

// Modifier un membre
int modifier_membre(char *filename, int id, Membre nouv) {
    FILE *f = fopen(filename, "r");
    if(f == NULL) return 0;
    
    FILE *f2 = fopen("temp.txt", "w");
    if(f2 == NULL) {
        fclose(f);
        return 0;
    }
    
    Membre m;
    int found = 0;
    char ligne[512];
    
    while(fgets(ligne, sizeof(ligne), f) != NULL) {
        if(parse_membre_ligne(ligne, &m)) {
            if(m.id == id) {
                // Écrire le membre modifié
                fprintf(f2, "%d %s %s %d %s %s %.2f\n", 
                       nouv.id, nouv.nom, nouv.email, nouv.age, 
                       nouv.sexe, nouv.Typeabonnement, nouv.Tarif);
                found = 1;
            } else {
                // Écrire le membre tel quel
                fprintf(f2, "%d %s %s %d %s %s %.2f\n", 
                       m.id, m.nom, m.email, m.age, m.sexe, m.Typeabonnement, m.Tarif);
            }
        } else {
            // Ligne mal formatée, la copier telle quelle
            fputs(ligne, f2);
        }
    }
    
    fclose(f);
    fclose(f2);
    
    if(!found) {
        remove("temp.txt");
        return 0;
    }
    
    remove(filename);
    rename("temp.txt", filename);
    return 1;
}

// Supprimer un membre
int supprimer_membre(char *filename, int id) {
    FILE *f = fopen(filename, "r");
    if(f == NULL) return 0;
    
    FILE *f2 = fopen("temp.txt", "w");
    if(f2 == NULL) {
        fclose(f);
        return 0;
    }
    
    Membre m;
    int found = 0;
    char ligne[512];
    
    while(fgets(ligne, sizeof(ligne), f) != NULL) {
        if(parse_membre_ligne(ligne, &m)) {
            if(m.id != id) {
                fprintf(f2, "%d %s %s %d %s %s %.2f\n", 
                       m.id, m.nom, m.email, m.age, m.sexe, m.Typeabonnement, m.Tarif);
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
        remove("temp.txt");
        return 0;
    }
    
    remove(filename);
    rename("temp.txt", filename);
    return 1;
}

// Nombre de membres
int nombre_membres(char *filename) {
    int count = 0;
    Membre m;
    FILE *f = fopen(filename, "r");
    
    if(f != NULL) {
        char ligne[512];
        while(fgets(ligne, sizeof(ligne), f) != NULL) {
            if(parse_membre_ligne(ligne, &m)) {
                count++;
            }
        }
        fclose(f);
    }
    return count;
}

// Membres par type d'abonnement
int membres_par_Typeabonnement(char *filename, char *Typeabonnement) {
    int count = 0;
    Membre m;
    FILE *f = fopen(filename, "r");
    
    if(f != NULL) {
        char ligne[512];
        while(fgets(ligne, sizeof(ligne), f) != NULL) {
            if(parse_membre_ligne(ligne, &m)) {
                if(strcmp(m.Typeabonnement, Typeabonnement) == 0) {
                    count++;
                }
            }
        }
        fclose(f);
    }
    return count;
}

// Membres par sexe
int membres_par_sexe(char *filename, char *sexe) {
    int count = 0;
    Membre m;
    FILE *f = fopen(filename, "r");
    
    if(f != NULL) {
        char ligne[512];
        while(fgets(ligne, sizeof(ligne), f) != NULL) {
            if(parse_membre_ligne(ligne, &m)) {
                if(strcmp(m.sexe, sexe) == 0) {
                    count++;
                }
            }
        }
        fclose(f);
    }
    return count;
}

// Statistiques complètes
void statistiques_completes_membre(char *filename) {
    int total = nombre_membres(filename);
    printf("\n=== STATISTIQUES DES MEMBRES ===\n");
    printf("Nombre total de membres: %d\n", total);

    if (total == 0) {
        printf("Aucun membre enregistré.\n");
        printf("=================================\n");
        return;
    }

    FILE *f = fopen(filename, "r");
    if(f != NULL) {
        Membre m;
        char ligne[512];
        char types[10][20];
        int counts_types[10] = {0};
        int nb_types = 0;
        
        int hommes = 0, femmes = 0;
        int somme_ages = 0;
        float somme_tarifs = 0;
        int age_min = 999, age_max = 0;
        
        while(fgets(ligne, sizeof(ligne), f) != NULL) {
            if(parse_membre_ligne(ligne, &m)) {
                // Statistiques par type d'abonnement
                int found = 0;
                for(int i = 0; i < nb_types; i++) {
                    if(strcmp(types[i], m.Typeabonnement) == 0) {
                        counts_types[i]++;
                        found = 1;
                        break;
                    }
                }
                if(!found && nb_types < 10) {
                    strcpy(types[nb_types], m.Typeabonnement);
                    counts_types[nb_types] = 1;
                    nb_types++;
                }
                
                // Statistiques par sexe
                if(strcmp(m.sexe, "M") == 0 || strcmp(m.sexe, "Homme") == 0 || strcmp(m.sexe, "Masculin") == 0) hommes++;
                else if(strcmp(m.sexe, "F") == 0 || strcmp(m.sexe, "Femme") == 0 || strcmp(m.sexe, "Féminin") == 0) femmes++;
                
                // Statistiques d'âge
                somme_ages += m.age;
                if(m.age < age_min) age_min = m.age;
                if(m.age > age_max) age_max = m.age;
                
                // Statistiques de tarif
                somme_tarifs += m.Tarif;
            }
        }
        fclose(f);
        
        // Affichage
        printf("\nRépartition par abonnement:\n");
        for(int i = 0; i < nb_types; i++) {
            printf("  %s: %d membres (%.1f%%)\n", 
                   types[i], counts_types[i], (counts_types[i] * 100.0) / total);
        }
        
        printf("\nRépartition par sexe:\n");
        printf("  Hommes: %d (%.1f%%)\n", hommes, (hommes * 100.0) / total);
        printf("  Femmes: %d (%.1f%%)\n", femmes, (femmes * 100.0) / total);
        printf("  Non spécifié: %d (%.1f%%)\n", total - hommes - femmes, ((total - hommes - femmes) * 100.0) / total);
        
        printf("\nStatistiques d'âge:\n");
        printf("  Age moyen: %.1f ans\n", (float)somme_ages / total);
        printf("  Age minimum: %d ans\n", age_min);
        printf("  Age maximum: %d ans\n", age_max);
        
        printf("\nStatistiques financières:\n");
        printf("  Tarif moyen: %.1f DT\n", somme_tarifs / total);
        printf("  Revenu total: %.2f DT\n", somme_tarifs);
    }
    printf("=================================\n");
}

// ============================================================================
// FONCTIONS POUR RÉSERVATION D'ENTRAINEUR - CORRIGÉES (SANS STATUT)
// ============================================================================

Entraineur chercher_entraineur_disponible(char *filename) {
    Entraineur e;
    e.id = -1;

    FILE *f = fopen(filename, "r");
    if(f != NULL) {
        int id, experience, age;
        char nometprenom[60], specialite[30], sexe;
        
        // FORMAT SANS STATUT: ID NomEtPrenom Specialite Experience Age Sexe
        while(fscanf(f, "%d %59s %29s %d %d %c", 
                      &id, nometprenom, specialite, &experience, &age, &sexe) != EOF) {
            // Tous les entraineurs sont considérés comme disponibles
            e.id = id;
            strcpy(e.nometprenom, nometprenom);
            strcpy(e.specialite, specialite);
            e.experience = experience;
            e.age = age;
            e.sexe = sexe;
            break;
        }
        fclose(f);
    }
    return e;
}

void afficher_reservations_membre(int id_membre) {
    int id_m, id_e;
    char date[20], creneau[20];
    FILE *f = fopen("reserve_membre_coach.txt", "r");
    int count = 0;
    
    if (f != NULL) {
        printf("\n=== MES RÉSERVATIONS D'ENTRAINEUR (ID: %d) ===\n", id_membre);
        printf("Entraineur\t\tDate\t\tCreneau\n");
        printf("------------------------------------------------------------\n");
        
        // FORMAT SANS STATUT: id_membre:id_entraineur:date:creneau
        while (fscanf(f, "%d:%d:%[^:]:%[^\n]", &id_m, &id_e, date, creneau) != EOF) {
            if (id_m == id_membre) {
                FILE *f_ent = fopen("entraineurs.txt", "r");
                if (f_ent != NULL) {
                    int id, experience, age;
                    char nometprenom[60], specialite[30], sexe;
                    
                    // FORMAT SANS STATUT
                    while (fscanf(f_ent, "%d %59s %29s %d %d %c", 
                                  &id, nometprenom, specialite, &experience, &age, &sexe) != EOF) {
                        if (id == id_e) {
                            printf("%s\t%s\t%s\n", nometprenom, date, creneau);
                            count++;
                            break;
                        }
                    }
                    fclose(f_ent);
                }
            }
        }
        fclose(f);
        
        printf("------------------------------------------------------------\n");
        printf("Total: %d réservation(s)\n", count);
    } else {
        printf("Aucune réservation trouvée.\n");
    }
}
