#include "token.h"

namespace token {
    std::initializer_list<Color> Colors = {
            Color::blue,
            Color::white,
            Color::green,
            Color::black,
            Color::red,
            Color::pearl,
            Color::gold
    };

    string toString(const Color c) {
        switch (c) {
            case Color::blue:
                return "B";
            case Color::white:
                return "W";
            case Color::green:
                return "G";
            case Color::black:
                return "N";
            case Color::red:
                return "R";
            case Color::pearl:
                return "P";
            case Color::gold:
                return "O";
            default:
                throw SplendorException("Unknown color");
        }
    }

    ostream &operator<<(ostream &f, const Color c) {
        f << toString(c);
        return f;
    }

    ostream &operator<<(ostream &f, const Token t) {
        f << t.getColor();
        return f;
    }

    void saveTokens(const map<Color, size_t> &compte, QXmlStreamWriter &writer) {
        for (const auto [couleur, nb]: compte) {
            writer.writeAttribute(toString(couleur), QString::number(nb));
        }
        writer.writeEndElement();
    }

    map<Color, size_t> loadTokens(const QXmlStreamReader &reader) {
        map<Color, size_t> nouveauCompte;
        for (const auto couleur: Colors) {
            if (const auto nbJetons = reader.attributes().value(toString(couleur)).toInt(); nbJetons > 0)
                nouveauCompte[couleur] = nbJetons;
        }
        return nouveauCompte;
    }
}
