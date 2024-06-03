#pragma once

#include "initializer_list"
#include "iostream"
#include "string"
#include <map>
#include <QXmlStreamWriter>

using namespace std;

namespace token {
    class SplendorException final : public exception {
        string info;

    public:
        explicit SplendorException(string s) : info(std::move(s)) {
        }

        const char *what() const noexcept override { return info.c_str(); };
    };

    enum class Color {
        blue,
        white,
        green,
        black,
        red,
        pearl,
        gold
    };

    string toString(Color);

    ostream &operator<<(ostream &f, Color c);

    extern std::initializer_list<Color> Colors;

    class Token {
        Color color;

    public:
        Color getColor() const {
            return color;
        }

        explicit Token(const Color c) : color(c) {
        }

        ~Token() = default;

        bool isGold() const { return color == Color::gold; }

        bool isPearl() const { return color == Color::pearl; }
    };

    ostream &operator<<(ostream &f, Token t);

    void saveTokens(const map<Color, size_t> &c, QXmlStreamWriter &w);

    map<Color, size_t> loadTokens(const QXmlStreamReader &r);
}
