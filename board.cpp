#include "board.h"
#include <random>
#include <ctime>
#include <algorithm>

namespace plateau {
    void Plateau::remplirPlateau(Sachet& s) {
        size_t nbJ = s.getNbJetons();
        for (size_t i = 0; i < nbMax && nbJ != 0; i++) {
            if (jetons[i] == nullptr) {
                jetons[i] = s.piocherJetonDansSachet();
                nbJ--;
                nbJetons++;
                if (jetons[i]->isGold()) nbJetonsOr++;
            }
        }
    }

    void Plateau::retirerJetons(const jeton::Jeton& j) {
        size_t i = 0;
        while (i < nbMax && jetons[i] != &j) i++;
        if (i == nbMax) throw jeton::SplendorException("Pas de jeton de cette couleur sur le plateau");
        if (j.isGold()) nbJetonsOr--;
        jetons[i] = nullptr;
        nbJetons--;
    }

    void Plateau::afficherPlateau() const {
        for (size_t i = 0; i < 5; i++) {
            cout << i + 1 << "  ";
            for (size_t j = 0; j < 5; j++) {
                if (jetons[position[i][j]] == nullptr) {
                    cout << "# ";
                } else {
                    cout << *jetons[position[i][j]] << " ";
                }
            }
            cout << "\n";
        }
        cout << "   ";
        for (size_t i = 1; i <= 5; i++)
            cout << i << " ";
        cout << "\n";
    }

    Sachet::Sachet() {
        nb = 25;
        for (size_t i = 0; i < 5; i++) {
            for (size_t j = 0; j < 4; j++) {
                jetons.push_back(new const jeton::Jeton(static_cast<jeton::Couleur>(i)));
            }
        }
        for (size_t i = 0; i < 2; i++) {
            jetons.push_back(new const jeton::Jeton(jeton::Couleur::gold));
            jetons.push_back(new const jeton::Jeton(jeton::Couleur::perle));
        }
        jetons.push_back(new const jeton::Jeton(jeton::Couleur::gold));
        melangeSachet();
    }

    Sachet::~Sachet() {
        for (size_t i = 0; i < 25; i++) {
            delete jetons[i];
        }
    }

    void Sachet::melangeSachet() {
        random_device dev;
        mt19937 rng(dev());
        shuffle(jetons.begin(), jetons.end(), rng);
    }

    const jeton::Jeton* Sachet::piocherJetonDansSachet() {
        if (nb == 0) throw jeton::SplendorException("Pas assez de jetons pour piocher");
        return jetons[--nb];
    }

    void Sachet::remettreJetonDansSachet(const jeton::Couleur& j) {
        if (nb == 25) throw jeton::SplendorException("Le sachet est deja plein");
        size_t i = nb;
        while (i < 25 && jetons[i]->getCouleur() != j) i++;
        if (i == 25) throw jeton::SplendorException("Impossible d'ajouter cette couleur au sachet");
        const jeton::Jeton* tmp = jetons[i];
        jetons[i] = jetons[nb];
        jetons[nb] = tmp;
        nb++;
    }
}
