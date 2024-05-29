#pragma once

#include "initializer_list"
#include "iostream"
#include "string"
#include <map>
#include <QXmlStreamWriter>

using namespace std;

namespace jeton {
    class SplendorException final : public exception {
        string info;

    public:
        explicit SplendorException(string s): info(std::move(s)) {
        }

        const char* what() const noexcept override { return info.c_str(); };
    };

    enum class Couleur {
        bleu,
        blanc,
        vert,
        noir,
        rouge,
        perle,
        gold
    };

    string toString(Couleur);

    ostream& operator<<(ostream& f, Couleur c);

    extern std::initializer_list<Couleur> Couleurs;

    class Jeton {
        Couleur couleur;

    public:
        Couleur getCouleur() const {
            return couleur;
        }

        explicit Jeton(const Couleur c): couleur(c) {
        }

        ~Jeton() = default;

        bool isGold() const { return couleur == Couleur::gold; }

        bool isPerle() const { return couleur == Couleur::perle; }
    };

    ostream& operator<<(ostream& f, Jeton j);

    void sauvegarderCompteJetons(const map<Couleur, size_t>&, QXmlStreamWriter&);

    map<Couleur, size_t> chargerCompteJetons(const QXmlStreamReader&);
}
