#pragma once

#include "token.h"
#include <vector>
#include <QCoreApplication>
#include <QFile>
#include <QXmlStreamWriter>
#include <QDebug>

using namespace std;

namespace board {
    class Bag {
        vector<const token::Token *> tokens;
        size_t nb;

    public:
        Bag();

        ~Bag();

        size_t getNbTokens() const { return nb; }

        void shuffleBag();

        const token::Token *drawTokenFromBag();

        void returnTokenToBag(const token::Color &);

        void saveState(const QString &fileName) const {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QXmlStreamWriter xmlWriter(&file);
                xmlWriter.setAutoFormatting(true);

                xmlWriter.writeStartDocument();
                xmlWriter.writeStartElement("bag");

                xmlWriter.writeAttribute("nb_tokens", QString::number(nb));

                for (const token::Token *pToken: tokens) {
                    xmlWriter.writeStartElement("token");
                    int color = -1;
                    if (pToken)
                        color = static_cast<int>(pToken->getColor());
                    xmlWriter.writeAttribute("value", QString::number(color));
                    xmlWriter.writeEndElement();
                }

                xmlWriter.writeEndElement();
                xmlWriter.writeEndDocument();

                file.close();
            } else {
                qDebug() << "Couldn't open the file to save the bag.";
            }
        }

        void loadState(const QString &fileName) {
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QXmlStreamReader xmlReader(&file);
                tokens.clear();

                while (!xmlReader.atEnd() && !xmlReader.hasError()) {
                    const QXmlStreamReader::TokenType token = xmlReader.readNext();

                    if (token == QXmlStreamReader::StartDocument) {
                        continue;
                    }
                    if (token == QXmlStreamReader::StartElement && xmlReader.name().toString() == "bag")
                        nb = xmlReader.attributes().value("nb_tokens").toInt();

                    if (token == QXmlStreamReader::StartElement && xmlReader.name().toString() == "token") {
                        if (auto tokenValue = xmlReader.attributes().value("value").toInt(); tokenValue != -1) {
                            tokens.push_back(new token::Token(static_cast<token::Color>(tokenValue)));
                        } else
                            tokens.push_back(nullptr);
                        xmlReader.skipCurrentElement();
                    }
                }
                file.close();
            } else {
                qDebug() << "Couldn't open the file to save the bag.";
            }
        }
    };

    class Board {
        vector<const token::Token *> tokens;
        size_t nbTokens;
        size_t nbGoldTokens;
        const size_t nbMax;
        const size_t position[5][5] = {
                {20, 21, 22, 23, 24},
                {19, 6,  7,  8,  9},
                {18, 5,  0,  1,  10},
                {17, 4,  3,  2,  11},
                {16, 15, 14, 13, 12}
        };

    public:
        Board() : nbGoldTokens(0), nbTokens(0), nbMax(25) {
            for (size_t i = 0; i < nbMax; i++) {
                tokens.emplace_back(nullptr);
            }
        }

        ~Board() = default;

        void fillBoard(Bag &bag);

        size_t getNbTokens() const { return nbTokens; }

        size_t getNbGoldTokens() const { return nbGoldTokens; }

        void removeToken(const token::Token &token);

        void displayBoard() const;

        vector<const token::Token *> getTokens() const { return tokens; }

        size_t getPosition(size_t i, size_t j) const { return position[i][j]; }

        void saveState(const QString &fileName) const {
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QXmlStreamWriter xmlWriter(&file);
                xmlWriter.setAutoFormatting(true);

                xmlWriter.writeStartDocument();
                xmlWriter.writeStartElement("board");

                for (const token::Token *pToken: tokens) {
                    xmlWriter.writeStartElement("token");
                    int color = -1;
                    if (pToken)
                        color = static_cast<int>(pToken->getColor());
                    xmlWriter.writeAttribute("value", QString::number(color));
                    xmlWriter.writeEndElement();
                }

                xmlWriter.writeEndElement();
                xmlWriter.writeEndDocument();

                file.close();
            } else {
                qDebug() << "Couldn't open the file to save the board.";
            }
        }

        void loadState(const QString &fileName) {
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QXmlStreamReader xmlReader(&file);
                tokens.clear();

                while (!xmlReader.atEnd() && !xmlReader.hasError()) {
                    QXmlStreamReader::TokenType token = xmlReader.readNext();

                    if (token == QXmlStreamReader::StartDocument) {
                        continue;
                    }
                    if (token == QXmlStreamReader::StartElement && xmlReader.name().toString() == "token") {
                        if (auto tokenValue = xmlReader.attributes().value("value").toInt(); tokenValue != -1) {
                            tokens.push_back(new token::Token(static_cast<token::Color>(tokenValue)));
                            nbTokens++;
                        } else
                            tokens.push_back(nullptr);
                        xmlReader.skipCurrentElement();
                    }
                }
                file.close();
            } else {
                qDebug() << "Couldn't open the file to save the board.";
            }
        }
    };
}
