#include "player.h"

namespace player {
    void Player::takeCardRoyals(card::CardRoyals *c) {
        cardsRoyals.push_back(c);
        nbPrestiges += c->getPrestige();
    }

    void Player::takeCardJewels(card::CardJewels *c) {
        cardsJewels.push_back(c);
        nbPrestiges += c->getPrestige();
        bonus[c->getDiscountColor()] += c->getNbDiscount();
        prestigeByColor[c->getDiscountColor()] += c->getPrestige();
        nbCrowns += c->getCrowns();
    }

    void Player::reserveCardJewels(card::CardJewels *c) {
        cardsReserve.push_back(c);
    }

    void Player::removeFromReserve(const card::CardJewels *c) {
        for (size_t i = 0; i < cardsReserve.size(); i++) {
            if (cardsReserve[i] == c) {
                cardsReserve[i] = cardsReserve[cardsReserve.size() - 1];
                cardsReserve.pop_back();
            }
        }
    }

    bool Player::hasWonByColorPrestige() {
        for (size_t i = 0; i < 7; i++) {
            if (prestigeByColor[static_cast<token::Color>(i)] >= 10) return true;
        }
        return false;
    }

    void Player::saveState(const QString &dir) const {
        QFile file(dir + "/player.xml");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QXmlStreamWriter xmlWriter(&file);
            xmlWriter.setAutoFormatting(true);

            xmlWriter.writeStartDocument();
            xmlWriter.writeStartElement("player");

            xmlWriter.writeAttribute("name", QString::fromStdString(name));
            xmlWriter.writeAttribute("ai", QString::number(ai));
            xmlWriter.writeAttribute("nb_privi", QString::number(nbPrivileges));
            xmlWriter.writeAttribute("nb_prest", QString::number(nbPrestiges));
            xmlWriter.writeAttribute("nb_crowns", QString::number(nbCrowns));
            xmlWriter.writeAttribute("nb_tokens", QString::number(nbTokens));
            xmlWriter.writeStartElement("tokens");
            saveTokens(tokens, xmlWriter);
            xmlWriter.writeStartElement("discount");
            saveTokens(bonus, xmlWriter);
            xmlWriter.writeStartElement("prestByColor");
            saveTokens(prestigeByColor, xmlWriter);

            xmlWriter.writeEndElement();
            xmlWriter.writeEndDocument();

            file.close();
        } else {
            qDebug() << "Couldn't open the file to save the player.";
        }
        saveJewelsStack(cardsJewels, dir + "/jewels_cards.xml");
        saveJewelsStack(cardsReserve, dir + "/reserved_jewels_cards.xml");
        saveRoyalsStack(cardsRoyals, dir + "/royals_cards.xml");
    }

    Player Player::loadState(const QString &dir) {
        QFile file(dir + "/player.xml");
        Player nouveauJoueur("test", true);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QXmlStreamReader xmlReader(&file);

            while (!xmlReader.atEnd() && !xmlReader.hasError()) {
                const QXmlStreamReader::TokenType token = xmlReader.readNext();

                if (token == QXmlStreamReader::StartDocument) {
                    continue;
                }
                if (token == QXmlStreamReader::StartElement && xmlReader.name().toString() == "player") {
                    nouveauJoueur = Player(
                            xmlReader.attributes().value("name").toString().toStdString(),
                            xmlReader.attributes().value("ai").toInt()
                    );
                    nouveauJoueur.nbPrivileges = xmlReader.attributes().value("nb_privi").toInt();
                    nouveauJoueur.nbPrestiges = xmlReader.attributes().value("nb_prest").toInt();
                    nouveauJoueur.nbCrowns = xmlReader.attributes().value("nb_crowns").toInt();
                    nouveauJoueur.nbTokens = xmlReader.attributes().value("nb_tokens").toInt();
                    xmlReader.readNextStartElement();
                    nouveauJoueur.tokens = token::loadTokens(xmlReader);
                    xmlReader.skipCurrentElement();
                    xmlReader.readNextStartElement();
                    nouveauJoueur.bonus = token::loadTokens(xmlReader);
                    xmlReader.skipCurrentElement();
                    xmlReader.readNextStartElement();
                    nouveauJoueur.prestigeByColor = token::loadTokens(xmlReader);
                    xmlReader.skipCurrentElement();
                }
            }
            file.close();
            nouveauJoueur.cardsJewels = card::loadJewelsStack(dir + "/jewels_cards.xml");
            nouveauJoueur.cardsReserve = card::loadJewelsStack(dir + "/reserved_jewels_cards.xml");
            nouveauJoueur.cardsRoyals = card::loadRoyalsStack(dir + "/royals_cards.xml");
            return nouveauJoueur;
        }
        qDebug() << "Couldn't open the file to save the player.";
        throw token::SplendorException("Loading failed");
    }

}
