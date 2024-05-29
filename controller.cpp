#include "iostream"
#include "algorithm"
#include "controller.h"

using namespace std;

namespace splendor_duel {
    Controleur* Controleur::instanceUnique = nullptr;

    bool fichierMemoireExiste(const filesystem::path& name) {
        if (FILE* file = fopen(name.string().c_str(), "r")) {
            fclose(file);
            return true;
        }
        return false;
    }

    void Controleur::partieContinuee() {
        partie = partie::Partie::chargerEtat();
    }

    Controleur* Controleur::donneInstance() {
        if (instanceUnique == nullptr)
            instanceUnique = new Controleur;
        return instanceUnique;
    }

    void Controleur::libereInstance() {
        delete instanceUnique;
        instanceUnique = nullptr;
    }

    void Controleur::start() {
        cout << "Splendor Duel\n1: Nouvelle partie\n";
        if (const filesystem::path memoryPath = partie::getBasePath() / "save" / "partie.xml"; fichierMemoireExiste(memoryPath))
            cout << "2: Continuer une partie\n";
        cout << "0: Quitter\n";
        int choix;
        cin >> choix;
        if (choix == 1) {
            nouvellePartie();
        } else if (choix == 2) {
            partieContinuee();
        } else return;
        partie->jouer();
        if (partie->estFinie())
            remove("../save/partie.xml");
        else
            partie->sauvegarderEtat();
    }

    void Controleur::nouvellePartie() {
        //demander joueurs
        cout << "\n1: 1 joueur (contre ordinateur)\n2: 2 joueurs\n3: Ordinateur contre ordinateur\n0: Annuler\n"; {
            int choix;
            const vector choixValide{1, 2, 3, 0};
            while (true) {
                if (cin >> choix && find(choixValide.cbegin(), choixValide.cend(), choix) != choixValide.cend())
                    break;
                cout << "Choix invalide, reessayez :\n";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            if (choix == 0) {
                start();
                return;
            }
            int choixIA = 0;
            if (choix != 2) {
                cout << "\nNiveau de l'IA : \n1: niveau 1 (aleatoire)\n0: Annuler\n";
                const vector choixIAValide{1};
                while (true) {
                    if (cin >> choixIA && find(choixValide.begin(), choixValide.end(), choixIA) != choixValide.end())
                        break;
                    cout << "Choix invalide, reessayez :\n";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
                if (choixIA == 0) {
                    nouvellePartie();
                    return;
                }
            }
            string j1 = "Bot Alpha";
            string j2 = "Bot Beta";
            joueur::Joueur joueur1(j1, true, choixIA);
            joueur::Joueur joueur2(j2, true, choixIA);
            if (choix != 3) {
                cout << "Nom du Joueur 1 (sans espaces) :";
                cin >> j1;
                joueur1 = joueur::Joueur(j1, false);
            }
            if (choix == 2) {
                cout << "Nom du Joueur 2 (sans espaces) :";
                cin >> j2;
                joueur2 = joueur::Joueur(j2, false);
            }
            partie = new partie::Partie(joueur1, joueur2, true);
        } // entre accolades pour permettre de free toutes les variables locales
        // vu que la fonction se termine seulement lorsqu'on ferme le jeu
    }
}
