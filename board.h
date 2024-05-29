#pragma once

#include "token.h"
#include <vector>
#include <QCoreApplication>
#include <QFile>
#include <QXmlStreamWriter>
#include <QDebug>

using namespace std;

namespace plateau {
    class Sachet {
        vector<const jeton::Jeton *> jetons;
        size_t nb;

    public:
        Sachet();

        ~Sachet();

        size_t getNbJetons() const { return nb; }

        void melangeSachet();

        const jeton::Jeton* piocherJetonDansSachet();

        void remettreJetonDansSachet(const jeton::Couleur&);

        void sauvegarderEtat(const QString& nomFichier) const {
            QFile file(nomFichier);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QXmlStreamWriter xmlWriter(&file);
                xmlWriter.setAutoFormatting(true);

                // Écriture des données dans le fichier XML
                xmlWriter.writeStartDocument();
                xmlWriter.writeStartElement("sachet");

                xmlWriter.writeAttribute("nb_jetons", QString::number(nb));

                // Exemple : sauvegarde des jetons présents dans le sachet
                for (const jeton::Jeton* jeton: jetons) {
                    xmlWriter.writeStartElement("jeton");
                    int couleur = -1;
                    if (jeton)
                        couleur = static_cast<int>(jeton->getCouleur());
                    xmlWriter.writeAttribute("valeur", QString::number(couleur));
                    xmlWriter.writeEndElement();
                }

                xmlWriter.writeEndElement();
                xmlWriter.writeEndDocument();

                file.close();
            } else {
                qDebug() << "Impossible d'ouvrir le fichier pour sauvegarder l'etat du sachet.";
            }
        }

        void chargerEtat(const QString& nomFichier) {
            QFile file(nomFichier);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QXmlStreamReader xmlReader(&file);
                jetons.clear();

                while (!xmlReader.atEnd() && !xmlReader.hasError()) {
                    const QXmlStreamReader::TokenType token = xmlReader.readNext();

                    if (token == QXmlStreamReader::StartDocument) {
                        continue;
                    }
                    if (token == QXmlStreamReader::StartElement && xmlReader.name().toString() == "sachet")
                        nb = xmlReader.attributes().value("nb_jetons").toInt();

                    if (token == QXmlStreamReader::StartElement && xmlReader.name().toString() == "jeton") {
                        if (auto valeurJeton = xmlReader.attributes().value("valeur").toInt(); valeurJeton != -1) {
                            jetons.push_back(new jeton::Jeton(static_cast<jeton::Couleur>(valeurJeton)));
                        } else
                            jetons.push_back(nullptr);
                        xmlReader.skipCurrentElement();
                    }
                }
                file.close();
            } else {
                qDebug() << "Impossible d'ouvrir le fichier pour charger l'etat du sachet.";
            }
        }
    };

    class Plateau {
        vector<const jeton::Jeton *> jetons;
        size_t nbJetons;
        size_t nbJetonsOr;
        const size_t nbMax;
        const size_t position[5][5] = {
            {20, 21, 22, 23, 24}, {19, 6, 7, 8, 9}, {18, 5, 0, 1, 10}, {17, 4, 3, 2, 11}, {16, 15, 14, 13, 12}
        };

    public:
        Plateau() : nbJetonsOr(0), nbJetons(0), nbMax(25) {
            for (size_t i = 0; i < nbMax; i++) {
                jetons.emplace_back(nullptr);
            }
        }

        ~Plateau() = default;

        void remplirPlateau(Sachet& s);

        size_t getNbJetons() { return nbJetons; }

        size_t getNbJetonsOr() { return nbJetonsOr; }

        void setNbJetonsOr(size_t i) { nbJetonsOr = i;}

        void retirerJetons(const jeton::Jeton& j);

        void retirerJetonsOr();

        void afficherPlateau() const;

        vector<const jeton::Jeton *> getJetons() const { return jetons; }
        size_t getPosition(size_t i, size_t j) const { return position[i][j]; }

        void sauvegarderEtat(const QString& nomFichier) const {
            QFile file(nomFichier);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QXmlStreamWriter xmlWriter(&file);
                xmlWriter.setAutoFormatting(true);

                // Écriture des données dans le fichier XML
                xmlWriter.writeStartDocument();
                xmlWriter.writeStartElement("plateau");

                // Exemple : sauvegarde des jetons présents sur le plateau
                for (const jeton::Jeton* jeton: jetons) {
                    xmlWriter.writeStartElement("jeton");
                    int couleur = -1;
                    if (jeton)
                        couleur = static_cast<int>(jeton->getCouleur());
                    xmlWriter.writeAttribute("valeur", QString::number(couleur));
                    xmlWriter.writeEndElement();
                }

                xmlWriter.writeEndElement();
                xmlWriter.writeEndDocument();

                file.close();
            } else {
                qDebug() << "Impossible d'ouvrir le fichier pour sauvegarder l'etat du plateau.";
            }
        }

        void chargerEtat(const QString& nomFichier) {
            QFile file(nomFichier);
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QXmlStreamReader xmlReader(&file);
                jetons.clear();

                while (!xmlReader.atEnd() && !xmlReader.hasError()) {
                    QXmlStreamReader::TokenType token = xmlReader.readNext();

                    if (token == QXmlStreamReader::StartDocument) {
                        continue;
                    }
                    if (token == QXmlStreamReader::StartElement && xmlReader.name().toString() == "jeton") {
                        if (auto valeurJeton = xmlReader.attributes().value("valeur").toInt(); valeurJeton != -1) {
                            jetons.push_back(new jeton::Jeton(static_cast<jeton::Couleur>(valeurJeton)));
                            nbJetons++;
                        } else
                            jetons.push_back(nullptr);
                        xmlReader.skipCurrentElement();
                    }
                }
                file.close();
            } else {
                qDebug() << "Impossible d'ouvrir le fichier pour charger l'etat du plateau.";
            }
        }
    };
}
