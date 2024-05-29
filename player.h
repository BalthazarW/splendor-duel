#pragma once

#include <utility>

#include "string"
#include "map"
#include "vector"
#include "token.h"
#include "card.h"

using namespace std;

namespace joueur {
    class Joueur {
        string pseudo;
        bool ia;
        int niveauIA;
        map<jeton::Couleur, size_t> jetons;
        map<jeton::Couleur, size_t> bonus;
        map<jeton::Couleur, size_t> nbPrestigesCouleur;
        vector<carte::CarteJoaillerie *> cartesJoaillerie;
        vector<carte::Carte *> cartesRoyale;
        vector<carte::CarteJoaillerie *> reserveCartes;
        size_t nbPrivileges = 0;
        size_t nbPrestiges = 0;
        size_t nbCouronnes = 0;
        size_t nbJetons = 0;

    public:
        Joueur(string p, const bool i, int n = 0): pseudo(std::move(p)), ia(i), niveauIA(n) {
        }

        ~Joueur() = default;

        string getPseudo() const { return pseudo; }
        bool isIA() const { return ia; }
        map<jeton::Couleur, size_t> getJetons() const { return jetons; }
        map<jeton::Couleur, size_t> getBonus() const { return bonus; }
        vector<carte::CarteJoaillerie *> getCartesJoaillerie() const { return cartesJoaillerie; }
        vector<carte::Carte *> getCartesRoyale() const { return cartesRoyale; }
        vector<carte::CarteJoaillerie *> getReserve() const { return reserveCartes; }
        size_t getNbPrivileges() const { return nbPrivileges; }
        size_t getNbCouronnes() const { return nbCouronnes; }
        size_t getNbPrestiges() const { return nbPrestiges; }
        size_t getNbJetons() const { return nbJetons; }
        int getNiveauIA() const { return niveauIA; }

        void retirerPrivilege() { nbPrivileges--; }
        void ajouterPrivilege() { nbPrivileges++; }
        void retirerCouronne(const size_t nb) { nbCouronnes -= nb; }
        void ajouterCouronne(const size_t nb) { nbCouronnes += nb; }
        void retirerPrestiges(const size_t nb) { nbPrestiges -= nb; }
        void ajouterPrestiges(const size_t nb) { nbPrestiges += nb; }

        bool cumul10PointsPrestigeCouleur();

        void recupererCarte(carte::Carte* c);

        void recupererCarteJoaillerie(carte::CarteJoaillerie* c);

        void reserverCarteJoaillerie(carte::CarteJoaillerie*);

        void enleverDeLaReserve(const carte::CarteJoaillerie* c);

        void prendreJeton(const jeton::Couleur c) {
            jetons[c] += 1;
            nbJetons++;
        }

        void enleverJeton(const jeton::Couleur c) {
            jetons[c] -= 1;
            nbJetons--;
        }

        void sauvegarderEtat(const QString&) const;

        static Joueur chargerEtat(const QString&);
    };
}
