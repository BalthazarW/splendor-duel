#include "card.h"

namespace card {
    std::initializer_list<Ability> Abilities = {
            Ability::associate,
            Ability::nothing,
            Ability::take_opp_token,
            Ability::take_a_token,
            Ability::take_a_privilege,
            Ability::play_again
    };

    string toString(const Ability& c) {
        switch (c) {
            case Ability::nothing: return "no ability";
            case Ability::associate: return "free bonus";
            case Ability::take_opp_token: return "steal a token";
            case Ability::take_a_token: return "take a free token";
            case Ability::take_a_privilege: return "take a privilege";
            case Ability::play_again: return "play_again";
            default: throw token::SplendorException("Unknown ability");
        }
    }

    void toStringShort(string& ability, const Ability c, const size_t line) {
        const string middleLine = "|                |";
        string res[5];
        size_t height;
        if (c == Ability::nothing) {
            res[0] = "| no             |";
            res[1] = "| ability        |";
            height = 2;
        } else if (c == Ability::associate) {
            res[0] = "| free bonus     |";
            height = 1;
        } else if (c == Ability::take_opp_token) {
            res[0] = "| steal a token  |";
            res[1] = "| from your      |";
            res[2] = "| opponent       |";
            height = 3;
        } else if (c == Ability::take_a_token) {
            res[0] = "| take a free    |";
            res[1] = "| token from the |";
            res[2] = "| board          |";
            height = 4;
        } else if (c == Ability::take_a_privilege) {
            res[0] = "| take           |";
            res[1] = "| a privilege    |";
            height = 2;
        } else if (c == Ability::play_again) {
            res[0] = "| play_again        |";
            height = 1;
        } else if (c == Ability::no_card) {
            res[0] = "|     \\    /     |";
            res[1] = "|      \\  /      |";
            res[2] = "|       \\/       |";
            res[3] = "|       /\\       |";
            res[4] = "|      /  \\      |";
            height = 5;
        } else throw token::SplendorException("Unknown ability");
        for (size_t i = height; i < 5; i++)
            res[i] = middleLine;
        ability = res[line];
    }

    ostream& operator<<(ostream& f, const Ability ability) {
        f << toString(ability);
        return f;
    }

    void saveJewelsStack(const vector<CardJewels *>& stack, const QString& fileName) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QXmlStreamWriter xmlWriter(&file);
            xmlWriter.setAutoFormatting(true);

            xmlWriter.writeStartDocument();
            xmlWriter.writeStartElement("stack");

            for (const auto carte : stack) {
                xmlWriter.writeStartElement("card");
                if (carte == nullptr) {
                    xmlWriter.writeAttribute("valid", nullptr);
                }
                else
                    carte->saveState(xmlWriter);
                xmlWriter.writeEndElement();
            }

            xmlWriter.writeEndElement();
            xmlWriter.writeEndDocument();

            file.close();
        } else {
            qDebug() << "Couldn't open the file to save the stack.";
        }
    }

    void saveRoyalsStack(const vector<CardRoyals *>& stack, const QString& fileName) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QXmlStreamWriter xmlWriter(&file);
            xmlWriter.setAutoFormatting(true);

            xmlWriter.writeStartDocument();
            xmlWriter.writeStartElement("stack");

            for (const auto carte : stack) {
                xmlWriter.writeStartElement("card");
                if (carte == nullptr) {
                    xmlWriter.writeAttribute("valid", nullptr);
                }
                else
                    carte->saveState(xmlWriter);
                xmlWriter.writeEndElement();
            }

            xmlWriter.writeEndElement();
            xmlWriter.writeEndDocument();

            file.close();
        } else {
            qDebug() << "Couldn't open the file to save the stack.";
        }
    }

    vector<CardJewels *> loadJewelsStack(const QString& fileName) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            vector<CardJewels*> newStack;
            QXmlStreamReader xmlReader(&file);

            while (!xmlReader.atEnd() && !xmlReader.hasError()) {
                const QXmlStreamReader::TokenType token = xmlReader.readNext();

                if (token == QXmlStreamReader::StartDocument) {
                    continue;
                }
                if (token == QXmlStreamReader::StartElement && xmlReader.name().toString() == "card") {
                    newStack.push_back(CardJewels::loadState(xmlReader));
                    xmlReader.skipCurrentElement();
                }
            }
            file.close();
            return newStack;
        }
        qDebug() << "Couldn't open the file to load the stack.";
        throw token::SplendorException("Loading failed");
    }

    vector<CardRoyals *> loadRoyalsStack(const QString& fileName) {
        QFile file(fileName);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            vector<CardRoyals*> newStack;
            QXmlStreamReader xmlReader(&file);

            while (!xmlReader.atEnd() && !xmlReader.hasError()) {
                const QXmlStreamReader::TokenType token = xmlReader.readNext();

                if (token == QXmlStreamReader::StartDocument) {
                    continue;
                }
                if (token == QXmlStreamReader::StartElement && xmlReader.name().toString() == "card") {
                    newStack.push_back(CardRoyals::loadState(xmlReader));
                    xmlReader.skipCurrentElement();
                }
            }
            file.close();
            return newStack;
        }
        qDebug() << "Couldn't open the file to load the stack.";
        throw token::SplendorException("Loading failed");
    }

}
