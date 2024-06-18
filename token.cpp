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
                return "A";
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

    void saveTokens(const map<Color, size_t> &tokens, QXmlStreamWriter &writer) {
        for (const auto [color, nb]: tokens) {
            writer.writeAttribute(toString(color), QString::number(nb));
        }
        writer.writeEndElement();
    }

    map<Color, size_t> loadTokens(const QXmlStreamReader &reader) {
        map<Color, size_t> newTokens;
        for (const auto color: Colors) {
            if (const auto nbTokens = reader.attributes().value(toString(color)).toInt(); nbTokens > 0)
                newTokens[color] = nbTokens;
        }
        return newTokens;
    }
}
