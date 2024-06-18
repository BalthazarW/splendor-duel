#pragma once

#include "token.h"
#include <string>
#include <utility>
#include <vector>
#include <QCoreApplication>
#include <QFile>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDebug>
#include <map>
#include <utility>

using namespace std;

namespace card {
    enum class Ability {
        nothing,
        play_again,
        associate,
        take_a_token,
        take_a_privilege,
        take_opp_token,
        no_card
    };

    string toString(const Ability &);

    void toStringShort(string &, Ability, size_t);

    class CardRoyals {
    protected:
        size_t prestige;
        Ability ability;
        filesystem::path filePath;

    public:
        CardRoyals(const size_t prestige, const Ability ability, const string &fp) : prestige(prestige), ability(ability),
                                                                                     filePath(fp) {}

        virtual ~CardRoyals() = default;

        size_t getPrestige() const { return prestige; }

        Ability getAbility() const { return ability; }

        filesystem::path getFilePath() { return filePath; }

        virtual void saveState(QXmlStreamWriter &writer) {
            writer.writeAttribute("prestige", QString::number(prestige));
            writer.writeAttribute("ability", QString::number(static_cast<int>(ability)));
            writer.writeAttribute("file_path", QString::fromStdString(filePath.string()));
        }

        static CardRoyals *loadState(const QXmlStreamReader &reader) {
            return new CardRoyals(
                    reader.attributes().value("prestige").toULongLong(),
                    static_cast<Ability>(reader.attributes().value("ability").toInt()),
                    reader.attributes().value("file_path").toString().toStdString()
            );
        }
    };

    class CardJewels : public CardRoyals {
        map<token::Color, size_t> *price;
        size_t crowns;
        token::Color discountColor;
        size_t nbDiscount = 0;

    public:
        CardJewels(const size_t prestige, map<token::Color, size_t> *price, const Ability ability,
                   const size_t crowns,
                   const token::Color color, const size_t discount,
                   const string &fp) : CardRoyals(prestige, ability, fp), price(price),
                                       crowns(crowns), discountColor(color),
                                       nbDiscount(discount) {}

        ~CardJewels() override = default;

        size_t getCrowns() const { return crowns; }

        const map<token::Color, size_t> *getPrice() const { return price; }

        token::Color getDiscountColor() const { return discountColor; }

        size_t getNbDiscount() const { return nbDiscount; }

        void setDiscountColor(const token::Color newColor) {
            discountColor = newColor;
        }

        void saveState(QXmlStreamWriter &writer) override {
            CardRoyals::saveState(writer);
            writer.writeAttribute("crowns", QString::number(crowns));
            writer.writeAttribute("discount_color", QString::number(static_cast<int>(discountColor)));
            writer.writeAttribute("nb_discount", QString::number(nbDiscount));
            writer.writeStartElement("price");
            saveTokens(*price, writer);
        }

        static CardJewels *loadState(QXmlStreamReader &reader) {
            const size_t prestige = reader.attributes().value("prestige").toULongLong();
            const auto ability = static_cast<Ability>(reader.attributes().value("ability").toInt());
            const string filePath = reader.attributes().value("file_path").toString().toStdString();
            const size_t crowns = reader.attributes().value("crowns").toULongLong();
            const size_t nbDiscount = reader.attributes().value("nb_discount").toULongLong();
            const auto discountColor = static_cast<token::Color>(reader.attributes().value("discount_color").toInt());
            map < token::Color, size_t > price;
            reader.readNextStartElement();
            if (reader.name().toString() == "price") {
                price = token::loadTokens(reader);
                reader.skipCurrentElement();
            }
            return new CardJewels(prestige, new map(price), ability, crowns, discountColor, nbDiscount, filePath);
        }
    };

    void saveJewelsStack(const vector<CardJewels *> &, const QString &);

    void saveRoyalsStack(const vector<CardRoyals *> &, const QString &);

    vector<CardJewels *> loadJewelsStack(const QString &);

    vector<CardRoyals *> loadRoyalsStack(const QString &);

    ostream &operator<<(ostream &f, Ability a);
}
