#include "token.h"

namespace jeton {
    std::initializer_list<Couleur> Couleurs = {
        Couleur::bleu,
        Couleur::blanc,
        Couleur::vert,
        Couleur::noir,
        Couleur::rouge,
        Couleur::perle,
        Couleur::gold
    };

    string toString(const Couleur c) {
        switch (c) {
            case Couleur::bleu:
                return "B";
            case Couleur::blanc:
                return "W";
            case Couleur::vert:
                return "V";
            case Couleur::noir:
                return "N";
            case Couleur::rouge:
                return "R";
            case Couleur::perle:
                return "P";
            case Couleur::gold:
                return "O";
            default:
                throw SplendorException("Couleur inconnue");
        }
    }

    ostream& operator<<(ostream& f, const Couleur c) {
        f << toString(c);
        return f;
    }

    ostream& operator<<(ostream& f, const Jeton j) {
        f << j.getCouleur();
        return f;
    }

    void sauvegarderCompteJetons(const map<Couleur, size_t>& compte, QXmlStreamWriter& writer) {
        for (const auto [couleur, nb] : compte) {
            writer.writeAttribute(toString(couleur), QString::number(nb));
        }
        writer.writeEndElement();
    }

    map<Couleur, size_t> chargerCompteJetons(const QXmlStreamReader& reader) {
        map<Couleur, size_t> nouveauCompte;
        for (const auto couleur : Couleurs) {
            if (const auto nbJetons = reader.attributes().value(toString(couleur)).toInt(); nbJetons > 0)
                nouveauCompte[couleur] = nbJetons;
        }
        return nouveauCompte;
    }
}
