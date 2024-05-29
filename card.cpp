#include "card.h"

namespace carte {
    std::initializer_list<Capacite> Capacites = {
        Capacite::associe,
        Capacite::rien,
        Capacite::jeton_adversaire,
        Capacite::prendre_un_jeton,
        Capacite::prendre_un_privilege,
        Capacite::rejouer
    };

    string toString(const Capacite& c) {
        switch (c) {
            case Capacite::rien: return "aucune capacite";
            case Capacite::associe: return "bonus libre";
            case Capacite::jeton_adversaire: return "prendre 1 jeton à l'adversaire";
            case Capacite::prendre_un_jeton: return "prendre 1 jeton sur le plateau";
            case Capacite::prendre_un_privilege: return "prendre 1 privilege";
            case Capacite::rejouer: return "rejouer";
            default: throw jeton::SplendorException("Capacite inconnue");
        }
    }

    void toStringShort(string& capa, const Capacite c, const size_t line) {
        const string middleLine = "|                |";
        string res[5];
        size_t height;
        if (c == Capacite::rien) {
            res[0] = "| aucune         |";
            res[1] = "| capacite       |";
            height = 2;
        } else if (c == Capacite::associe) {
            res[0] = "| bonus libre    |";
            height = 1;
        } else if (c == Capacite::jeton_adversaire) {
            res[0] = "| prendre        |";
            res[1] = "| un jeton a     |";
            res[2] = "| l'adversaire   |";
            height = 3;
        } else if (c == Capacite::prendre_un_jeton) {
            res[0] = "| prendre        |";
            res[1] = "| un jeton       |";
            res[2] = "| sur le         |";
            res[3] = "| plateau        |";
            height = 4;
        } else if (c == Capacite::prendre_un_privilege) {
            res[0] = "| prendre        |";
            res[1] = "| un privilege   |";
            height = 2;
        } else if (c == Capacite::rejouer) {
            res[0] = "| rejouer        |";
            height = 1;
        } else if (c == Capacite::pas_de_carte) {
            res[0] = "|     \\    /     |";
            res[1] = "|      \\  /      |";
            res[2] = "|       \\/       |";
            res[3] = "|       /\\       |";
            res[4] = "|      /  \\      |";
            height = 5;
        } else throw jeton::SplendorException("Capacite inconnue");
        for (size_t i = height; i < 5; i++)
            res[i] = middleLine;
        capa = res[line];
    }

    ostream& operator<<(ostream& f, const Capacite c) {
        f << toString(c);
        return f;
    }

    void sauvegarderEtatPioche(const vector<CarteJoaillerie *>& pioche, const QString& fichier) {
        QFile file(fichier);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QXmlStreamWriter xmlWriter(&file);
            xmlWriter.setAutoFormatting(true);

            // Écriture des données dans le fichier XML
            xmlWriter.writeStartDocument();
            xmlWriter.writeStartElement("pioche");

            for (const auto carte : pioche) {
                xmlWriter.writeStartElement("carte");
                if (carte == nullptr) {
                    xmlWriter.writeAttribute("valide", nullptr);
                }
                else
                    carte->sauvegarderEtat(xmlWriter);
                xmlWriter.writeEndElement();
            }

            xmlWriter.writeEndElement();
            xmlWriter.writeEndDocument();

            file.close();
        } else {
            qDebug() << "Impossible d'ouvrir le fichier pour sauvegarder l'etat de la pioche.";
        }
    }

    void sauvegarderEtatPioche(const vector<Carte *>& pioche, const QString& fichier) {
        QFile file(fichier);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QXmlStreamWriter xmlWriter(&file);
            xmlWriter.setAutoFormatting(true);

            // Écriture des données dans le fichier XML
            xmlWriter.writeStartDocument();
            xmlWriter.writeStartElement("pioche");

            for (const auto carte : pioche) {
                xmlWriter.writeStartElement("carte");
                if (carte == nullptr) {
                    xmlWriter.writeAttribute("valide", nullptr);
                }
                else
                    carte->sauvegarderEtat(xmlWriter);
                xmlWriter.writeEndElement();
            }

            xmlWriter.writeEndElement();
            xmlWriter.writeEndDocument();

            file.close();
        } else {
            qDebug() << "Impossible d'ouvrir le fichier pour sauvegarder l'etat de la pioche.";
        }
    }

    vector<CarteJoaillerie *> chargerEtatPiocheJ(const QString& fichier) {
        QFile file(fichier);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            vector<CarteJoaillerie*> nouvellePioche;
            QXmlStreamReader xmlReader(&file);

            while (!xmlReader.atEnd() && !xmlReader.hasError()) {
                const QXmlStreamReader::TokenType token = xmlReader.readNext();

                if (token == QXmlStreamReader::StartDocument) {
                    continue;
                }
                if (token == QXmlStreamReader::StartElement && xmlReader.name().toString() == "carte") {
                    nouvellePioche.push_back(CarteJoaillerie::chargerEtat(xmlReader));
                    xmlReader.skipCurrentElement();
                }
            }
            file.close();
            return nouvellePioche;
        }
        qDebug() << "Impossible d'ouvrir le fichier pour charger l'etat de la pioche.";
        throw jeton::SplendorException("Chargement impossible");
    }

    vector<Carte *> chargerEtatPioche(const QString& fichier) {
        QFile file(fichier);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            vector<Carte*> nouvellePioche;
            QXmlStreamReader xmlReader(&file);

            while (!xmlReader.atEnd() && !xmlReader.hasError()) {
                const QXmlStreamReader::TokenType token = xmlReader.readNext();

                if (token == QXmlStreamReader::StartDocument) {
                    continue;
                }
                if (token == QXmlStreamReader::StartElement && xmlReader.name().toString() == "carte") {
                    nouvellePioche.push_back(Carte::chargerEtat(xmlReader));
                    xmlReader.skipCurrentElement();
                }
            }
            file.close();
            return nouvellePioche;
        }
        qDebug() << "Impossible d'ouvrir le fichier pour charger l'etat de la pioche.";
        throw jeton::SplendorException("Chargement impossible");
    }

}
