#include <fstream>
#include <sstream>
#include <algorithm>
#include "random"
#include "game.h"
#include "token.h"

using namespace jeton;
using namespace carte;

namespace partie {
    filesystem::path getBasePath() {
        const auto assetsPath1 = filesystem::absolute("assets");
        const auto assetsPath2 = filesystem::absolute("../assets");
        const auto savePath1 = filesystem::absolute("save");
        const auto savePath2 = filesystem::absolute("../save");
        if (const vector path1 = {assetsPath1, savePath1}; all_of(
                path1.cbegin(),
                path1.cend(),
                [](const filesystem::path& path) { return exists(path); })
        )
            return assetsPath1.parent_path();
        if (const vector path2 = {assetsPath2, savePath2}; all_of(
                path2.cbegin(),
                path2.cend(),
                [](const filesystem::path& path) { return exists(path); })
        )
            return assetsPath2.parent_path();
        throw SplendorException("Couldn't find the path for the game's assets or the save path");
    }

    void printSpacing(const size_t base, const bool pyramide) {
        if (!pyramide)
            return;
        if (base == 3)
            cout << "                   ";
        else if (base == 4)
            cout << "         ";
    }

    void afficherLigneCartes(const vector<CarteJoaillerie *>& listeCartes, const size_t base, const bool pyramide) {
        const string bottomLine = "|________________|";
        printSpacing(base, pyramide);
        for (int i = 0; i < min(base, size_t(6)); i++) {
            cout << " ________________  ";
        }
        cout << "\n";
        printSpacing(base, pyramide);
        for (int i = 0; i < min(base, size_t(6)); i++) {
            if (listeCartes[i] != nullptr) {
                cout << "| P:" << listeCartes[i]->getPrestige() << " " << "C:" << listeCartes[i]->getCouronnes()
                        << "    ";
                if (listeCartes[i]->getNbReduc() != 0) {
                    cout << listeCartes[i]->getNbReduc() << "x";
                    if (listeCartes[i]->getCouleurReduc() != Couleur::perle)
                        cout << listeCartes[i]->getCouleurReduc();
                    else
                        cout << "?";
                } else cout << "   ";
                cout << " | ";
            } else cout << "|                | ";
        }
        cout << "\n";
        printSpacing(base, pyramide);
        for (size_t line = 0; line < 5; line++) {
            for (int i = 0; i < min(base, size_t(6)); i++) {
                string capacite;
                if (listeCartes[i] != nullptr)
                    toStringShort(capacite, listeCartes[i]->getCapacite(), line);
                else toStringShort(capacite, Capacite::pas_de_carte, line);
                cout << capacite << " ";
            }
            cout << "\n";
            printSpacing(base, pyramide);
        }
        for (int i = 0; i < min(base, size_t(6)); i++) {
            if (listeCartes[i] != nullptr && listeCartes[i]->getPrix() != nullptr) {
                cout << "| ";
                size_t countPrice = 0;
                for (const auto [couleur, nb]: *listeCartes[i]-> getPrix()) {
                    cout << nb << couleur << " ";
                    countPrice++;
                }
                for (auto j = countPrice; j < 4; j++)
                    cout << "   ";
                cout << "   | ";
            } else cout << "|     /    \\     | ";
        }
        cout << "\n";
        printSpacing(base, pyramide);
        for (int i = 0; i < min(base, size_t(6)); i++) {
            cout << bottomLine << " ";
        }
        cout << "\n";
        if (base > 6)
            afficherLigneCartes(vector(listeCartes.begin() + 6, listeCartes.end()), base - 6, pyramide);
    }

    void quelleCouleur(const vector<CarteJoaillerie *>& container, const size_t index) {

        char rep = ' ';
        cout << "Choisissez la couleur de votre bonus. Exemple : pour du rouge R. Attention W pour Blanc\n";
        cin >> rep;
        switch (rep) {
            case 'B': container[index]->setCouleurReduc(Couleur::bleu);
                break;
            case 'W': container[index]->setCouleurReduc(Couleur::blanc);
                break;
            case 'V': container[index]->setCouleurReduc(Couleur::vert);
                break;
            case 'N': container[index]->setCouleurReduc(Couleur::noir);
                break;
            case 'R': container[index]->setCouleurReduc(Couleur::rouge);
                break;
            default: break;
        }
    }

    void quelleCouleurIA(const vector<CarteJoaillerie *>& container, const size_t index) {
       vector<char> choixValides = {'B', 'W', 'V', 'N', 'R'};
        random_device dev;
        mt19937 rng(dev());
        uniform_int_distribution<> dis(0, choixValides.size()-1);
       char rep = choixValides[dis(rng)];
        switch (rep) {
            case 'B': container[index]->setCouleurReduc(Couleur::bleu);
                break;
            case 'W': container[index]->setCouleurReduc(Couleur::blanc);
                break;
            case 'V': container[index]->setCouleurReduc(Couleur::vert);
                break;
            case 'N': container[index]->setCouleurReduc(Couleur::noir);
                break;
            case 'R': container[index]->setCouleurReduc(Couleur::rouge);
                break;
            default: break;
        }
    }

    void Partie::afficherCartesRoyale(const vector<Carte*>& listeCartes) const {
        const string bottomLine = "|________________|";
        const size_t base = listeCartes.size();
        for (int i = 0; i < base; i++) {
            cout << " ________________  ";
        }
        cout << "\n";
        for (int i = 0; i < base; i++) {
            cout << "| P:" << listeCartes[i]->getPrestige() << "            | ";
        }
        cout << "\n";
        for (size_t line = 0; line < 5; line++) {
            for (int i = 0; i < base; i++) {
                string capacite;
                toStringShort(capacite, listeCartes[i]->getCapacite(), line);
                cout << capacite << " ";
            }
            cout << "\n";
        }
        for (int i = 0; i < base; i++) {
            cout << "|                | ";
        }
        cout << "\n";
        for (int i = 0; i < base; i++) {
            cout << bottomLine << " ";
        }
        cout << "\n";
    }

    void Partie::instancierJoaillerie(const string& filename) {
        string line_info;
        int niveau, prestige, capacite, couronnes, couleur, nbReduc, bleu, blanc, vert, noir, rouge, perle;
        ifstream infile(filename);
        getline(infile, line_info);
        size_t lineNb = 1;
        auto assetsPath = getBasePath() / "assets";

        istringstream line_stream;
        while (getline(infile, line_info)) {
            line_stream.str(line_info);
            line_stream >> niveau >> prestige >> capacite >> couronnes >> couleur >> nbReduc >> bleu >> blanc >> vert >>
                    noir >> rouge >> perle;
            map<Couleur, size_t> prix;
            int couleurs[] = {bleu, blanc, vert, noir, rouge, perle};
            for (int i = 0; i < 6; i++) {
                if (couleurs[i] != 0)
                    prix[static_cast<Couleur>(i)] = couleurs[i];
            }
            char imageFilename[20];
            char cardLevelPath[20];
            sprintf(cardLevelPath, "cards_lvl%d", niveau);
            if (niveau == 1) {
                sprintf(imageFilename, "carte-%llu.png", lineNb);
                auto imagePath = assetsPath / cardLevelPath / imageFilename;
                cartesNv1.push_back(new CarteJoaillerie(
                    prestige,
                    new map(prix),
                    static_cast<Capacite>(capacite),
                    couronnes,
                    static_cast<Couleur>(couleur),
                    nbReduc,
                    imagePath.string()
                ));
            } else if (niveau == 2) {
                sprintf(imageFilename, "carte-%llu.png", lineNb - cartesNv1.size());
                auto imagePath = assetsPath / cardLevelPath / imageFilename;
                cartesNv2.push_back(new CarteJoaillerie(
                    prestige,
                    new map(prix),
                    static_cast<Capacite>(capacite),
                    couronnes,
                    static_cast<Couleur>(couleur),
                    nbReduc,
                    imagePath.string()
                ));
            } else {
                sprintf(imageFilename, "carte-%llu.png", lineNb - cartesNv1.size() - cartesNv2.size());
                auto imagePath = assetsPath / cardLevelPath / imageFilename;
                cartesNv3.push_back(new CarteJoaillerie(
                    prestige,
                    new map(prix),
                    static_cast<Capacite>(capacite),
                    couronnes,
                    static_cast<Couleur>(couleur),
                    nbReduc,
                    imagePath.string()
                ));
            }
            lineNb++;
            line_stream.clear();
        }
        infile.close();
        random_device dev;
        mt19937 rng(dev());
        shuffle(cartesNv1.begin(), cartesNv1.end(), rng);
        shuffle(cartesNv2.begin(), cartesNv2.end(), rng);
        shuffle(cartesNv3.begin(), cartesNv3.end(), rng);
    }

    void Partie::instancierRoyale(const string& filename) {
        string line_info;
        int prestige, capacite;
        ifstream infile(filename);
        getline(infile, line_info);
        size_t lineNb = 1;
        auto assetsPath = getBasePath() / "assets";

        istringstream line_stream;
        while (getline(infile, line_info)) {
            char imageFilename[20];
            sprintf(imageFilename, "carte-%llu.png", lineNb);
            auto imagePath = assetsPath / "royal_cards" / imageFilename;
            line_stream.str(line_info);
            line_stream >> prestige >> capacite;
            cartesRoyales.push_back(new Carte(prestige, static_cast<Capacite>(capacite), imagePath.string()));
            line_stream.clear();
            lineNb++;
        }
        infile.close();
    }

    void Partie::jouer() {
        bool aGagne = false;
        while (!aGagne) {
          cout << "Au tour de " << auTourDe->getPseudo() << "\n";
            afficherPyramide();
            cout << "Plateau :\n";
            plateau.afficherPlateau();
            cout << "\nCartes achetees par le joueur :\n";
            if (!auTourDe->getCartesJoaillerie().empty())
                afficherLigneCartes(auTourDe->getCartesJoaillerie(), auTourDe->getCartesJoaillerie().size(), false);
            cout << "Carte(s) royale(s) du joueur :\n";
            if (!auTourDe->getCartesRoyale().empty())
                afficherCartesRoyale(auTourDe->getCartesRoyale());
            cout << "Reserve du joueur :\n";
            if (!auTourDe->getReserve().empty())
                afficherLigneCartes(auTourDe->getReserve(), auTourDe->getReserve().size(), false);
            cout << "Jetons : ";
            for (const auto [couleur, nb]: (auTourDe->getJetons())) {
                cout << nb << couleur << " ";
            }
            cout << "\nNombre de Privilege : " << auTourDe->getNbPrivileges() << "\n";

            if (!auTourDe->isIA()) {
                bool continuer = true;
                bool quitter = false;
                while (actionsOptionnellesAccomplies < 2 && continuer) {
                    tie(continuer, quitter) = actionOptionnelle();
                    if (quitter && !continuer)
                        return;
                    if (!quitter || !continuer)
                        actionsOptionnellesAccomplies++;
                }
                continuer = false;
                continuer = actionObligatoire();
                if (!continuer) {
                    return; // pour ne pas changer 2 fois de joueur
                }
                actionsOptionnellesAccomplies = 0;
            } else if (auTourDe->getNiveauIA() == 1) {
                bool continuer = true;
                bool quitter = false;
                size_t k = 0;
                while (actionsOptionnellesAccomplies < 2 && continuer) {
                    k++;
                    tie(continuer, quitter) = actionOptionnelleIA();
                    if (quitter && !continuer)
                        return;
                    if (!quitter || !continuer)
                        actionsOptionnellesAccomplies++;
                }
                continuer = false;
                continuer = actionObligatoireIA();
                if (!continuer) {
                    return; // pour ne pas changer 2 fois de joueur
                }
                actionsOptionnellesAccomplies = 0;
            }
            aGagne = verifFinTour();
            if (!aGagne)
                changeJoueur();
            for (int i = 0; i < 10; i++)
                cout << "\n";
        }
        fini = true;
        cout << "Le joueur " << auTourDe->getPseudo() << " a gagne ! Bien joue !";
    }

    tuple<bool, bool> Partie::actionOptionnelle() {
        int choix = 0;
        vector choixValide{0, 3};
        cout << "Actions possibles:\n";
        if (auTourDe->getNbPrivileges() > 0) {
            cout << "1 : Utiliser un ou plusieurs privileges\n";
            choixValide.push_back(1);
        }
        if (sachet.getNbJetons() > 0) {
            cout << "2 : Remplir le plateau\n";
            choixValide.push_back(2);
        }
        cout << "3 : Accomplir une action obligatoire\n0 : Quitter\n";
        while (true) {
            if (cin >> choix && find(choixValide.cbegin(), choixValide.cend(), choix) != choixValide.cend())
                break;

            cout << "Choix invalide, reessayez :\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        if (choix == 1) {
            int res = 0;
            while (cout << "Merci de saisir un le nombre de privilèges à utiliser :\n" && !(cin >> res) && res <= 0 &&
                   res > auTourDe->getNbPrivileges()) {
                cin.clear();
            }
            for (int i = 0; i < res; i++) {
                vector<tuple<size_t, size_t>> choixJeton = choisirJetons(false, true);
                if (choixJeton.empty()) {
                    return {true, true};
                }
                size_t x, y = 0;
                tie(x, y) = choixJeton[0];
                utiliserPrivilege(*plateau.getJetons()[plateau.getPosition(x, y)]);
            }
            return {true, false};
        }
        if (choix == 2) {
            plateau.remplirPlateau(sachet);
            return {true, false};
        }
        if (choix == 0) {
            return {false, true};
        }
        return {false, false};
    }


    tuple<bool, bool> Partie::actionOptionnelleIA() {
        size_t choix = 0;
        vector choixValide{3};
        if (auTourDe->getNbPrivileges() > 0) {
            choixValide.push_back(1);
        }
        if (sachet.getNbJetons() > 0) {
            choixValide.push_back(2);
        }
        if (find(choixValide.begin(), choixValide.end(), 2) != choixValide.end())
            choix = 2;
        else {
            random_device dev;
            mt19937 rng(dev());
            uniform_int_distribution<> dis(0, choixValide.size() - 1);
            choix = choixValide[dis(rng)];
        }
        if (choix == 1) {
            random_device dev;
            mt19937 rng(dev());
            uniform_int_distribution<> dis(0, auTourDe->getNbPrivileges());
            int res = dis(rng);
            for (int i = 0; i < res; i++) {
                vector<tuple<size_t, size_t>> choixJeton = choisirJetonsIA(false, true, false, Couleur::bleu);
                if (choixJeton.empty()) {
                    return {true, true};
                }
                size_t x, y = 0;
                tie(x, y) = choixJeton[0];
                utiliserPrivilege(*plateau.getJetons()[plateau.getPosition(x, y)]);
            }
            return {true, false};
        }
        if (choix == 2) {
            plateau.remplirPlateau(sachet);
            return {true, false};
        }
        return {false, false};
    }



    bool Partie::actionObligatoireIA() {
        int choix = 0; {
            vector choixValide{2};
            if (plateau.getNbJetons() > 0 && plateau.getNbJetons() != plateau.getNbJetonsOr()) {
                choixValide.push_back(1);
            }
            if (peutReserver()) {
                choixValide.push_back(3);
            }
            random_device dev;
            mt19937 rng(dev());
            uniform_int_distribution<> dis(0, choixValide.size()-1);
            choix = choixValide[dis(rng)];
        } // toujours pour rester léger en mémoire (c'est marginal mais quand meme)
        bool choisi = false; // nous permet de vérifier si le joueur a annulé on non
        if (choix == 1) {
            const auto jetonsChoisis = choisirJetonsIA(false, false, false, Couleur::bleu);
            for (const auto coords: jetonsChoisis) {
                auto [x, y] = coords;
                prendreJetons(x, y);
                choisi = true;
            }
            if (!choisi)
                return actionObligatoireIA();
            return choisi;
        }
        if (choix == 2) {
            const tuple<vector<CarteJoaillerie *>, size_t> carteChoisie = choisirCarteIA(true);
            if (auto [container, index] = carteChoisie; index != -1) {
                choisi = true;
                if (container[index]->getCouleurReduc() == Couleur::perle)
                    quelleCouleurIA(container, index);
                acheterCarteJoaillerie(container, index);
                // Choix de la carte Royale si plus de 3 couronnes
                if (auTourDe->getNbCouronnes() >= 3 && auTourDe->getCartesRoyale().size() < 2) {
                    random_device dev;
                    mt19937 rng(dev());
                    uniform_int_distribution<> dis(0, cartesRoyales.size()-1);
                    prendreCarteRoyale(dis(rng));
                    auTourDe->retirerCouronne(3);
                }
                appelCapaciteCarte(container, index);
            } else
                return actionObligatoireIA();
            return choisi;
        }
        if (choix == 3) {
            const tuple<vector<CarteJoaillerie *>, size_t> carteChoisie = choisirCarteIA(false);
            if (auto [container, index] = carteChoisie; index != -1) {
                const vector<tuple<size_t, size_t>> choixJeton = choisirJetonsIA(true, true, false, Couleur::bleu);
                choisi = !choixJeton.empty();
                if (!choisi) {
                    return actionObligatoireIA();
                }
                recupererCarte(container, index, false);
                auto [x, y] = choixJeton[0];
                prendreJetons(x, y); //prendre le jeton Or apres avoir réservé la carte;
            } else
                return actionObligatoireIA();
            return choisi;
        }
        return false;
    }

    bool Partie::actionObligatoire() {
        int choix = 0; {
            vector choixValide{0, 1, 2};
            cout << "Actions possibles:\n";
            cout << "1: Prendre jusqu'a 3 jetons Gemme et/ou Perle\n";
            cout << "2: Acheter 1 carte Joaillerie (de la pyramide ou deja reservee)\n";
            if (peutReserver()) {
                cout << "3: Reserver une carte Joaillerie\n";
                choixValide.push_back(3);
            }
            cout << "0: Quitter\n";
            while (true) {
                if (cin >> choix && choix >= 0 && choix <= 3 &&
                    find(choixValide.cbegin(), choixValide.cend(), choix) != choixValide.cend())
                    break;
                cout << "Choix invalide, reessayez :\n";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        } // toujours pour rester léger en mémoire (c'est marginal mais quand meme)
        bool choisi = false; // nous permet de vérifier si le joueur a annulé on non
        if (choix == 1) {
            const auto jetonsChoisis = choisirJetons(false, false);
            for (const auto coords: jetonsChoisis) {
                auto [x, y] = coords;
                prendreJetons(x, y);
                choisi = true;
            }
            if (!choisi)
                return actionObligatoire();
            return choisi;
        }
        if (choix == 2) {
            const tuple<vector<CarteJoaillerie *>, size_t> carteChoisie = choisirCarte(true);
            if (auto [container, index] = carteChoisie; index != -1) {
                choisi = true;
                if (container[index]->getCouleurReduc() == Couleur::perle)
                    quelleCouleur(container, index);
                acheterCarteJoaillerie(container, index);
                // Choix de la carte Royale si plus de 3 couronnes
                if (auTourDe->getNbCouronnes() >= 3 && auTourDe->getCartesRoyale().size() < 2) {
                    prendreCarteRoyale(choisirCarteRoyale());
                    auTourDe->retirerCouronne(3);
                }
                appelCapaciteCarte(container, index);
            } else
                return actionObligatoire();
            return choisi;
        }
        if (choix == 3) {
            const tuple<vector<CarteJoaillerie *>, size_t> carteChoisie = choisirCarte(false);
            if (auto [container, index] = carteChoisie; index != -1) {
                const vector<tuple<size_t, size_t>> choixJeton = choisirJetons(true, true);
                choisi = !choixJeton.empty();
                if (!choisi)
                    return actionObligatoire();
                recupererCarte(container, index, false);
                auto [x, y] = choixJeton[0];
                prendreJetons(x, y); //prendre le jeton Or apres avoir réservé la carte;
            } else
                return actionObligatoire();
            return choisi;
        }
        return false;
    }

    vector<tuple<size_t, size_t>> Partie::choisirJetonsIA(const bool peutPrendreOr, const bool juste1Jeton, const bool prendreCouleurCapacite, jeton::Couleur c) {
        vector<tuple<size_t, size_t>> choix;
        map<Couleur, int> nbCouleur;
        int res = 1;
        if (!peutPrendreOr) {
            res = 3;
        } else {
            res = 1;
        }

        int i = 0;
        int x = -1;
        int y = -1;
        int x0 = 0;
        int y0 = 0;

        if (prendreCouleurCapacite) {
            for (size_t a = 0; a < 5; a++) {
                for (size_t b = 0; b < 5; b++) {
                    if (plateau.getJetons()[plateau.getPosition(a, b)] != nullptr
                        && plateau.getJetons()[plateau.getPosition(a, b)]->getCouleur() == c) {
                        choix.emplace_back(a, b);
                        return choix;
                    }
                }
            }
            return choix;
        }
           if (!peutPrendreOr) {
               random_device dev;
               mt19937 rng(dev());
               uniform_int_distribution<> dis(0, 4);
               int cpt = 0;
               do {
                   x = dis(rng);
                   y = dis(rng);
                   cpt++;
                   if (cpt == 25) return choix;
               } while (plateau.getJetons()[plateau.getPosition(x, y)] == nullptr
               || plateau.getJetons()[plateau.getPosition(x, y)]->getCouleur() == Couleur::gold);
               x0 = x;
               y0 = y;
               choix.emplace_back(x, y);
               i++;
               if (juste1Jeton) return choix;
           } if (peutPrendreOr) {
               for (size_t a = 0; a < 5; a++) {
                   for (size_t b = 0; b < 5; b++) {
                       if (plateau.getJetons()[plateau.getPosition(a, b)] != nullptr
                           && plateau.getJetons()[plateau.getPosition(a, b)]->getCouleur() == Couleur::gold) {
                           choix.emplace_back(a, b);
                           return choix;
                       }
                   }
               }
           }
        for (int a = -1; a <= 1; a++) {
            for (int b = -1; b <= 1; b++) {
                if ((a != 0 || b != 0) && x0+a >= 0 && x0+a <= 4 && y0+b >= 0 && y0+b <= 4 &&
                    plateau.getJetons()[plateau.getPosition(x0+a, y0+b)] != nullptr
                    && plateau.getJetons()[plateau.getPosition(x0+a, y0+b)]->getCouleur() != Couleur::gold) {
                    choix.emplace_back(x0+a, y0+b);
                    if (verifColineaire(choix) && verifProxi(choix)) {
                        i++;
                        nbCouleur[plateau.getJetons()[plateau.getPosition(x0+a, y0+b)]->getCouleur()] += 1;
                        if (i == res) break;
                    } else {
                        choix.pop_back();
                    }
                }
            }
            if (i == res) break;
        }
        if (!juste1Jeton && verifNbCouleurs(nbCouleur)) {
            cout << "L'IA a pris 3 jetons de la meme couleur ou\n"
                    "2 jetons Perle avec cette action, votre prenez donc 1 Privilege.\n";
            changeJoueur();
            prendreUnPrivilege();
            changeJoueur();
        }
        return choix;
    }

    vector<tuple<size_t, size_t>> Partie::choisirJetons(const bool peutPrendreOr, const bool juste1Jeton) {
        vector<tuple<size_t, size_t>> choix;
        map<Couleur, int> nbCouleur;
        plateau.afficherPlateau();
        int res = 1;
        if (!peutPrendreOr) {
            if (!juste1Jeton) {
                cout << "Combien de jetons voulez-vous prendre ?\n";
                res = -1;
                while (res <= 0 || res > 3) {
                    cout << "Merci de saisir un entier entre 1 et 3 (0 pour quitter le mode) :\n";
                    if (cin >> res && res == 0) return choix;
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
            } else {
                res = 1;
            }
            cout << "Vous ne devez pas prendre de jeton Or.\n";
        } else cout << "Vous devez prendre un jeton Or sur le plateau\n";
        int i = 0;
        int x = -1;
        int y = -1;
        while (i < res) {
            cout << "Saisie pour le jeton numero : " << i + 1 << "\n";
            while (x <= 0 || x > 5) {
                cout << "Merci de saisir le numero de ligne entre 1 et 5 (0 pour quitter le mode) :\n";
                if (cin >> x && x == 0) {
                    choix.clear();
                    return choix;
                }
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            while (y <= 0 || y > 5) {
                cout << "Merci de saisir le numero de colonne entre 1 et 5 (0 pour quitter le mode) :\n";
                if (cin >> y && y == 0) {
                    choix.clear();
                    return choix;
                }
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            x--; // passage en index depart a 0
            y--;
            if (plateau.getJetons()[plateau.getPosition(x, y)] != nullptr) {
                if (!peutPrendreOr && plateau.getJetons()[plateau.getPosition(x, y)]->getCouleur() == Couleur::gold) {
                    cout << "Vous ne pouvez pas prendre de jeton Or.\n";
                } else if (peutPrendreOr && plateau.getJetons()[plateau.getPosition(x, y)]->getCouleur() !=
                           Couleur::gold) {
                    cout << "Vous devez prendre un jeton Or\n";
                } else {
                    if (i == 0) {
                        choix.emplace_back(x, y);
                        i++;
                    } else {
                        choix.emplace_back(x, y);
                        if (verifProxi(choix) && verifColineaire(choix)) {
                            i++;
                        } else {
                            choix.pop_back();
                            cout <<
                                    "Les coordonnees ne sont pas correctes.\nMerci de recommencer la saisie de ce jeton.\n";
                        }
                    }
                    nbCouleur[plateau.getJetons()[plateau.getPosition(x, y)]->getCouleur()] += 1;
                }
            }
            x = -1;
            y = -1;
        }
        if (!juste1Jeton && verifNbCouleurs(nbCouleur)) {
            cout << "Vous avez pris 3 jetons de la meme couleur ou\n"
                    "les 2 jetons Perle avec cette action, votre adversaire prend donc 1 Privilege.\n";
            changeJoueur();
            prendreUnPrivilege();
            changeJoueur();
        }
        return choix;
    }

    bool verifNbCouleurs(map<Couleur, int> nbCouleur) {
        return any_of(
            nbCouleur.cbegin(), nbCouleur.cend(),
            [](const auto kv) { return kv.second == 3 || (kv.first == Couleur::perle && kv.second == 2); }
        );
    }

    /*
    bool sontAlignes(size_t x1, size_t y1, size_t x2, size_t y2) {
        return x1 * (y2 - y1) == y1 * (x2 - x1);
    }

    bool sontAdjacent(int x1, int y1, int x2, int y2) {
        return abs(x1 - x2) <= 1 && abs(y1 - y2) <= 1;
    }

    bool verifCoord2(const vector<tuple<size_t, size_t>>& choix, const size_t x, const size_t y, const size_t nb) {
        bool adjacent = false;
        for (size_t i = 0; i < choix.size(); i++) {
            auto [coordX0, coordY0] = choix[i];
            if (coordX0 == x && coordY0 == y) return false;
            if (!sontAlignes(coordX0, coordY0, x, y)) return false;
            if (sontAdjacent(coordX0, coordY0, x, y)) adjacent = true;
        }
        return adjacent;
    }
    bool verifCoord(const vector<tuple<size_t, size_t>>& choix, const size_t x, const size_t y, const size_t nb) {
        auto [coordX0, coordY0] = choix[0];

        if (nb == 1) {
            if ((x == coordX0 - 1 && y == coordY0 + 1) || (x == coordX0 + 1 && y == coordY0 - 1)) return true;
            if ((x == coordX0 + 1 && y == coordY0 + 1) || (x == coordX0 - 1 && y == coordY0 - 1)) return true;
            if ((x == coordX0 + 1 && y == coordY0) || (x == coordX0 - 1 && y == coordY0)) return true;
            if ((x == coordX0 && y == coordY0 + 1) || (x == coordX0 && y == coordY0 - 1)) return true;
        } else if (nb == 2) {
            auto [coordX1, coordY1] = choix[1];
            if ((coordX1 == coordX0 - 1 && coordY1 == coordY0 + 1 && x == coordX1 - 1 && y == coordY1 + 1) || (
                    coordX1 == coordX0 + 1 && coordY1 == coordY0 - 1 && x == coordX1 + 1 && y == coordY1 - 1))
                return
                        true;
            if ((coordX1 == coordX0 + 1 && coordY1 == coordY0 + 1 && x == coordX1 + 1 && y == coordY1 + 1) || (
                    coordX1 == coordX0 - 1 && coordY1 == coordY0 - 1 && x == coordX1 - 1 && y == coordY1 - 1))
                return
                        true;
            if ((coordX1 == coordX0 + 1 && coordY1 == coordY0 && x == coordX1 + 1 && y == coordY1) || (
                    coordX1 == coordX0 - 1 && coordY1 == coordY0 && x == coordX1 - 1 && y == coordY1))
                return true;
            if ((coordX1 == coordX0 && coordY1 == coordY0 + 1 && x == coordX1 && y == coordY1 + 1) || (
                    coordX1 == coordX0 && coordY1 == coordY0 - 1 && x == coordX1 && y == coordY1 - 1))
                return true;
        } else throw SplendorException("Nombre de jetons choisis invalide");

        return false;
    }
*/
    tuple<vector<CarteJoaillerie *>, size_t> Partie::choisirCarteIA(const bool achat) {
        int choix;
        vector<int> choixValides;
        int cartesParLigne = 3;
        vector<vector<CarteJoaillerie *>> cartes = {cartesNv3, cartesNv2, cartesNv1};
        for (int nbContainer = 0; nbContainer < cartes.size(); nbContainer++) {
            for (int i = 0; i < cartesParLigne; i++) {
                if (cartes[nbContainer][i]) {
                    if (peutAcheter(cartes[nbContainer][i]) || !achat) {
                        int choixCarte = 10 * (nbContainer + 1) + (i + 1);
                        choixValides.push_back(choixCarte);
                    }
                }
            }
        }

       if (choixValides.size() == 0){
           choix = 0;
       } else {
           random_device dev;
           mt19937 rng(dev());
           uniform_int_distribution<> dis(0, choixValides.size()-1);
           choix = choixValides[dis(rng)];
       }
        if (choix == 0)
            return {cartesNv3, -1};
        return {cartes[(choix - 11) / 10], (choix - 11) % 10};
    }

    double distance(const tuple<size_t, size_t>& pt1, const tuple<size_t, size_t>& pt2) {
        const auto [x1, y1] = pt1;
        const auto [x2, y2] = pt2;
        return sqrt((y2 - y1) * (y2 - y1) + (x2 - x1) * (x2 - x1));
    }

    bool verifProxi(const vector<tuple<size_t, size_t>>& choix) {
        if (choix.size() == 1)
            return true;
        if (choix.size() == 2)
            return distance(choix[0], choix[1]) < 2; // si 2, trop loin. en digonale, dist = racine de 2
        if (choix.size() == 3) {
            int nbProches = 0;
            for (int i = 0; i < 3; i++) {
                for (int j = i + 1; j < 3; j++)
                    if (const auto dist = distance(choix[i], choix[j]); dist < 2)
                        nbProches++;
            }
            return nbProches == 2;
        }
        throw SplendorException("Nombre de jetons sélectionnés invalide");
    }

    bool verifColineaire(const vector<tuple<size_t, size_t>>& choix) {
        if (choix.size() > 3)
            throw SplendorException("Trop de jetons sélectionnés");
        if (choix.size() != 3)
            return true;
        vector<size_t> coords;
        for (const auto [x, y]: choix) {
            coords.push_back(x);
            coords.push_back(y);
        }
        // formule trouvée ici : https://math.stackexchange.com/a/405981
        return (coords[3] - coords[1]) * (coords[4] - coords[2]) == (coords[5] - coords[3]) * (coords[2] - coords[0]);
    }

    tuple<vector<CarteJoaillerie *>, size_t> Partie::choisirCarte(const bool achat) {
        int choix;
        vector<int> choixValides;
        cout << "Pyramide :\n";
        afficherPyramide();
        auto reserveJoueur = auTourDe->getReserve();
        auto nbReserve = reserveJoueur.size();
        if (nbReserve > 0) {
            cout << "Reserve :\n";
            afficherLigneCartes(reserveJoueur, nbReserve, false);
        }
        int cartesParLigne = 3;
        cout << "\nChoississez la carte que vous voulez en entrant le nombre correspondant:\n";
        vector<vector<CarteJoaillerie *>> cartes = {cartesNv3, cartesNv2, cartesNv1, reserveJoueur};
        cout << "\nPyramide :";
        for (int nbContainer = 0; nbContainer < cartes.size() - 1; nbContainer++) {
            cout << "\n";
            for (int space = cartesParLigne; space < 5; space++)
                cout << "  ";
            for (int i = 0; i < cartesParLigne; i++) {
                if (cartes[nbContainer][i]) {
                    if (peutAcheter(cartes[nbContainer][i]) || !achat) {
                        int choixCarte = 10 * (nbContainer + 1) + (i + 1);
                        choixValides.push_back(choixCarte);
                        cout << choixCarte;
                    } else
                        cout << "XX";
                } else
                    cout << "XX";
                cout << "  ";
            }
            cout << "\n";
            cartesParLigne++;
        }
        if (nbReserve > 0) {
            cout << "\nReserve :\n";
            for (int i = 0; i < nbReserve; i++) {
                if (peutAcheter(reserveJoueur[i]) || !achat) {
                    int choixCarte = 40 + (i + 1);
                    choixValides.push_back(choixCarte);
                    cout << choixCarte;
                } else
                    cout << "XX";
                cout << "  ";
            }
            cout << "\n";
        }
        cout << "\nCarte choisie (0 pour annuler):";
        cin >> choix;
        while (find(choixValides.cbegin(), choixValides.cend(), choix) == choixValides.cend() && choix != 0) {
            cout << "\nChoix invalide, veuillez entrer un choix valide:";
            cin >> choix;
        }
        if (choix == 0)
            return {cartesNv3, -1};
        return {cartes[(choix - 11) / 10], (choix - 11) % 10};
    }

    size_t Partie::choisirCarteRoyale() const {
        int choix;
        vector<int> choixValides;
        cout << "\nVous avez accumulé assez de couronnes pour récupérer une carte Royale !\n";
        afficherCartesRoyale(cartesRoyales);
        cout << "\nChoississez la carte que vous voulez en entrant le nombre correspondant :\n";
        for (int i = 0; i < cartesRoyales.size(); i++) {
            cout << "  " << i + 1;
            choixValides.push_back(i + 1);
        }
        cout << "\nCarte choisie:";
        while (find(choixValides.cbegin(), choixValides.cend(), choix) == choixValides.cend() && choix != 0) {
            cout << "\nChoix invalide, veuillez entrer un choix valide:";
            cin >> choix;
        }
        return choix - 1;
    }

    void Partie::utiliserPrivilege(const Jeton& j) {
        if (auTourDe->getNbPrivileges() != 0) {
            auTourDe->retirerPrivilege();
            plateau.retirerJetons(j);
            auTourDe->prendreJeton(j.getCouleur());
        }
    }

    void Partie::prendreJetons(const size_t a, const size_t b) {
        if (plateau.getJetons()[plateau.getPosition(a, b)] != nullptr) {
            const Couleur c = plateau.getJetons()[plateau.getPosition(a, b)]->getCouleur();
            auTourDe->prendreJeton(c);
            plateau.retirerJetons(*plateau.getJetons()[plateau.getPosition(a, b)]);
        }
    }

    bool Partie::peutAcheter(const CarteJoaillerie* c) const {
        auto jetons = auTourDe->getJetons();
        size_t nbJetonsOr = jetons[Couleur::gold];
        for (auto kv : *c->getPrix()) {
            const int diff = kv.second - jetons[kv.first] - auTourDe->getBonus()[kv.first];
            if (diff > long(nbJetonsOr) && diff >= 0)
                return false;
            if (diff > 0) {
                nbJetonsOr -= diff;
            }
        }
        return true;
    }

    bool Partie::peutReserver() const {
        const auto jetons = plateau.getJetons();
        return auTourDe->getReserve().size() < 3 &&
               any_of(
                   jetons.cbegin(),
                   jetons.cend(),
                   [](auto jeton) {
                       if (jeton) return jeton->isGold();
                       return false;
                   }
               );
    }

    void Partie::prendreCarteRoyale(const size_t index) {
        auTourDe->recupererCarte(cartesRoyales[index]);
        cartesRoyales[index] = cartesRoyales[cartesRoyales.size() - 1];
        cartesRoyales.pop_back();
    }

    void Partie::recupererCarte(const vector<CarteJoaillerie *>& container, const size_t index, const bool achat) {
        if (achat)
            auTourDe->recupererCarteJoaillerie(container[index]);
        else
            auTourDe->reserverCarteJoaillerie(container[index]);
        if (container == cartesNv1) {
            if (cartesNv1.size() > 5) {
                cartesNv1[index] = cartesNv1[cartesNv1.size() - 1];
                cartesNv1.pop_back();
            } else cartesNv1[index] = nullptr;
        } else if (container == cartesNv2) {
            if (cartesNv2.size() > 4) {
                cartesNv2[index] = cartesNv2[cartesNv2.size() - 1];
                cartesNv2.pop_back();
            } else cartesNv2[index] = nullptr;
        } else if (container == cartesNv3) {
            if (cartesNv3.size() > 3) {
                cartesNv3[index] = cartesNv3[cartesNv3.size() - 1];
                cartesNv3.pop_back();
            } else cartesNv3[index] = nullptr;
        } else if (container == auTourDe->getReserve()){
            auTourDe->enleverDeLaReserve(container[index]);
        } else throw SplendorException("Conteneur de cartes inconnu");
    }

    void Partie::acheterCarteJoaillerie(const vector<CarteJoaillerie *>& container, const size_t index) {
        for (const auto [couleur, prixCouleur]: *container[index]->getPrix()) {
            for (int i = 0; i < max(int(prixCouleur) - int(auTourDe->getBonus()[couleur]), 0); i++) {
                Couleur couleurPrise = couleur;
                if (auTourDe->getJetons()[couleur] == 0)
                    couleurPrise = Couleur::gold;
                sachet.remettreJetonDansSachet(couleurPrise);
                auTourDe->enleverJeton(couleurPrise);
            }
        }
        recupererCarte(container, index, true);
    }

    bool Partie::verifFinTour() {
        if (const size_t nb = auTourDe->getNbJetons(); nb > 10) {
            if (!auTourDe->isIA()) demanderReposerJetonsTerminal(nb - 10);
            else demanderReposerJetonsTerminalIA(nb - 10);
        }
        return verifVictoire();
    }

    void Partie::demanderReposerJetonsTerminalIA(const size_t nb) {
        char rep = ' ';
        vector<char> choixValides = {'B', 'W', 'V', 'N', 'R', 'P', 'G'};
        size_t i = 0;
        int c = -1;
        random_device dev;
        mt19937 rng(dev());
        uniform_int_distribution<> dis(0, choixValides.size()-1);
        while (i < nb) {
            rep = choixValides[dis(rng)];
            switch (rep) {
                case 'B': c = 0;
                    break;
                case 'W': c = 1;
                    break;
                case 'V': c = 2;
                    break;
                case 'N': c = 3;
                    break;
                case 'R': c = 4;
                    break;
                case 'P': c = 5;
                    break;
                case 'G': c = 6;
                    break;
                default: c = -1;
                    break;
            }
            if (c != -1) {
                if (auTourDe->getJetons()[static_cast<Couleur>(c)] != 0) {
                    auTourDe->enleverJeton(static_cast<Couleur>(c));
                    sachet.remettreJetonDansSachet(static_cast<Couleur>(c));
                    i++;
                }
            }
        }
    }

    void Partie::demanderReposerJetonsTerminal(const size_t nb) {
        char rep = ' ';
        cout << "Vous possedez plus de 10 jetons. Vous devez en remettre dans le sachet.\n";
        size_t i = 0;
        int c = -1;
        while (i < nb) {
            cout << "Vous devez remettre encore " << nb - i <<
                    "jetons dans le sachet.\nMerci de saisir le jeton que vous souhaitez remettre par exemple : R pour 1 jeton rouge.\n";
            cin >> rep;
            switch (rep) {
                case 'B': c = 0;
                    break;
                case 'W': c = 1;
                    break;
                case 'V': c = 2;
                    break;
                case 'N': c = 3;
                    break;
                case 'R': c = 4;
                    break;
                case 'P': c = 5;
                    break;
                case 'G': c = 6;
                    break;
                default: c = -1;
                    break;
            }
            if (c != -1) {
                if (auTourDe->getJetons()[static_cast<Couleur>(c)] != 0) {
                    auTourDe->enleverJeton(static_cast<Couleur>(c));
                    sachet.remettreJetonDansSachet(static_cast<Couleur>(c));
                    i++;
                }
            }
        }
    }

    bool Partie::verifVictoire() const {
        return auTourDe->cumul10PointsPrestigeCouleur() || auTourDe->getNbCouronnes() >= 10 || auTourDe->
               getNbPrestiges() >= 20;
    }

    void Partie::prendreUnPrivilege() {
        if (auTourDe->getNbPrivileges() < 3) {
            // si le joueur a déjà les 3 privileges, il n'en prend pas
            if (nbPrivilegesDispos != 0) {
                auTourDe->ajouterPrivilege();
                nbPrivilegesDispos--;
            } else {
                changeJoueur();
                auTourDe->retirerPrivilege();
                changeJoueur();
                auTourDe->ajouterPrivilege();
            }
        }
    }

    void Partie::changeJoueur() {
        if (auTourDe == &joueurs[0]) auTourDe = &joueurs[1];
        else auTourDe = &joueurs[0];
    }


    void Partie::appelCapaciteCarte(const vector<CarteJoaillerie *>& container, const size_t index) {
        if (container[index]->getCapacite() == Capacite::jeton_adversaire) {
            if (!auTourDe->isIA()) jetonAdversaire();
            else jetonAdversaireIA();
        }
        if (container[index]->getCapacite() == Capacite::prendre_un_jeton) {
            jetonPlateauCapacite(container[index]->getCouleurReduc());
        }
        if (container[index]->getCapacite() == Capacite::prendre_un_privilege) {
            prendreUnPrivilege();
        }
        if (container[index]->getCapacite() == Capacite::rejouer) {
            //a discuter mais peut etre changer de autourde la
            changeJoueur();
        }
    }

    void Partie::jetonAdversaireIA() {
        int i = 0;
        char rep = ' ';
        if (auTourDe == &joueurs[0]) {
            i = 1;
        }
        if (joueurs[i].getJetons().empty())  {
            return;
        }
        vector<char> choixValides = {'B', 'W', 'V', 'N', 'R', 'P'};
        random_device dev;
        mt19937 rng(dev());
        uniform_int_distribution<> dis(0, choixValides.size()-1);
        rep = choixValides[dis(rng)];
        Couleur couleurPrise;
        switch (rep) {
            case 'B': couleurPrise = Couleur::bleu;
                break;
            case 'W': couleurPrise = Couleur::blanc;
                break;
            case 'V': couleurPrise = Couleur::vert;
                break;
            case 'N': couleurPrise = Couleur::noir;
                break;
            case 'R': couleurPrise = Couleur::rouge;
                break;
            case 'P': couleurPrise = Couleur::perle;
                break;
            default: throw SplendorException("Couleur impossible a choisir");
        }
        auTourDe->prendreJeton(couleurPrise);
        joueurs[i].enleverJeton(couleurPrise);
    }


    void Partie::jetonAdversaire() {
        int i = 0;
        char rep = ' ';
        if (auTourDe == &joueurs[0]) {
            i = 1;
        }
        if (joueurs[i].getJetons().empty())
            return;
        cout << "Choisir la couleur du jeton à prendre parmi ceux de l'adversaire : \n";
        for (const auto [couleur, nbJetons]: joueurs[i].getJetons()) {
            if (nbJetons != 0 && couleur != Couleur::gold) {
                cout << couleur << ", ";
            }
        }
        cout << "\n";
        cin >> rep;
        Couleur couleurPrise;
        switch (rep) {
            case 'B': couleurPrise = Couleur::bleu;
                break;
            case 'W': couleurPrise = Couleur::blanc;
                break;
            case 'V': couleurPrise = Couleur::vert;
                break;
            case 'N': couleurPrise = Couleur::noir;
                break;
            case 'R': couleurPrise = Couleur::rouge;
                break;
            case 'P': couleurPrise = Couleur::perle;
                break;
            default: throw SplendorException("Couleur impossible a choisir");
        }
        auTourDe->prendreJeton(couleurPrise);
        joueurs[i].enleverJeton(couleurPrise);
    }

    void Partie::jetonPlateauCapacite(Couleur c) {
        bool bienChoisi = false;
        auto jetons = plateau.getJetons();
        if (any_of(
            jetons.cbegin(),
            jetons.cend(),
            [c](const Jeton* jeton) {
                if (jeton != nullptr) return jeton->getCouleur() == c;
                return false;
            }
        ))
            while (!bienChoisi) {
                int x;
                int y;
                if (!auTourDe->isIA()) {
                    cout << "Vous devez choisir un jeton " << c << ".\n";
                    const tuple<size_t, size_t> choix = choisirJetons(false, true)[0];
                    tie(x, y) = choix;
                } else {
                    const tuple<size_t, size_t> choix = choisirJetonsIA(false, true, true, c)[0];
                    tie(x, y) = choix;
                }
                if (x != -1 && plateau.getJetons()[plateau.getPosition(x, y)]->getCouleur() == c) {
                    bienChoisi = true;
                    auTourDe->prendreJeton(c);
                    plateau.retirerJetons(*plateau.getJetons()[plateau.getPosition(x, y)]);
                } else cout << "Choix invalide, reesayez.\n";
            }
        else cout << "Il n'y a pas de jetons de la bonne couleur sur le plateau !\n";
    }
}
