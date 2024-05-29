#include "player.h"

namespace joueur {
    void Joueur::recupererCarte(carte::Carte* c) {
        cartesRoyale.push_back(c);
        nbPrestiges += c->getPrestige();
    }

    void Joueur::recupererCarteJoaillerie(carte::CarteJoaillerie* c) {
        cartesJoaillerie.push_back(c);
        nbPrestiges += c->getPrestige();
        bonus[c->getCouleurReduc()] += c->getNbReduc();
        nbPrestigesCouleur[c->getCouleurReduc()] += c->getPrestige();
        nbCouronnes += c->getCouronnes();
    }

    void Joueur::reserverCarteJoaillerie(carte::CarteJoaillerie* c) {
        reserveCartes.push_back(c);
    }

    void Joueur::enleverDeLaReserve(const carte::CarteJoaillerie* c) {
        for (size_t i = 0; i < reserveCartes.size(); i++) {
            if (reserveCartes[i] == c) {
                reserveCartes[i] = reserveCartes[reserveCartes.size() - 1];
                reserveCartes.pop_back();
            }
        }
    }

    bool Joueur::cumul10PointsPrestigeCouleur() {
        for (size_t i = 0; i < 7; i++) {
            if (nbPrestigesCouleur[static_cast<jeton::Couleur>(i)] >= 10) return true;
        }
        return false;
    }

    void Joueur::sauvegarderEtat(const QString& dir) const {
        QFile file(dir + "/joueur.xml");
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QXmlStreamWriter xmlWriter(&file);
            xmlWriter.setAutoFormatting(true);

            // Écriture des données dans le fichier XML
            xmlWriter.writeStartDocument();
            xmlWriter.writeStartElement("joueur");

            xmlWriter.writeAttribute("pseudo", QString::fromStdString(pseudo));
            xmlWriter.writeAttribute("ia", QString::number(ia));
            xmlWriter.writeAttribute("nb_privi", QString::number(nbPrivileges));
            xmlWriter.writeAttribute("nb_prest", QString::number(nbPrestiges));
            xmlWriter.writeAttribute("nb_cour", QString::number(nbCouronnes));
            xmlWriter.writeAttribute("nb_jetons", QString::number(nbJetons));
            xmlWriter.writeStartElement("jetons");
            sauvegarderCompteJetons(jetons, xmlWriter);
            xmlWriter.writeStartElement("reduc");
            sauvegarderCompteJetons(bonus, xmlWriter);
            xmlWriter.writeStartElement("prestParCouleur");
            sauvegarderCompteJetons(nbPrestigesCouleur, xmlWriter);

            xmlWriter.writeEndElement();
            xmlWriter.writeEndDocument();

            file.close();
        } else {
            qDebug() << "Impossible d'ouvrir le fichier pour sauvegarder l'etat du joueur.";
        }
        sauvegarderEtatPioche(cartesJoaillerie, dir + "/cartesJ.xml");
        sauvegarderEtatPioche(reserveCartes, dir + "/cartesJR.xml");
        sauvegarderEtatPioche(cartesRoyale, dir + "/cartesR.xml");
    }

    Joueur Joueur::chargerEtat(const QString& dir) {
        QFile file(dir + "/joueur.xml");
        Joueur nouveauJoueur("test", true);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QXmlStreamReader xmlReader(&file);

            while (!xmlReader.atEnd() && !xmlReader.hasError()) {
                const QXmlStreamReader::TokenType token = xmlReader.readNext();

                if (token == QXmlStreamReader::StartDocument) {
                    continue;
                }
                if (token == QXmlStreamReader::StartElement && xmlReader.name().toString() == "joueur") {
                    nouveauJoueur = Joueur(
                        xmlReader.attributes().value("pseudo").toString().toStdString(),
                        xmlReader.attributes().value("ia").toInt()
                        );
                    nouveauJoueur.nbPrivileges = xmlReader.attributes().value("nb_privi").toInt();
                    nouveauJoueur.nbPrestiges = xmlReader.attributes().value("nb_prest").toInt();
                    nouveauJoueur.nbCouronnes = xmlReader.attributes().value("nb_cour").toInt();
                    nouveauJoueur.nbJetons = xmlReader.attributes().value("nb_jetons").toInt();
                    xmlReader.readNextStartElement();
                    nouveauJoueur.jetons = jeton::chargerCompteJetons(xmlReader);
                    xmlReader.skipCurrentElement();
                    xmlReader.readNextStartElement();
                    nouveauJoueur.bonus = jeton::chargerCompteJetons(xmlReader);
                    xmlReader.skipCurrentElement();
                    xmlReader.readNextStartElement();
                    nouveauJoueur.nbPrestigesCouleur = jeton::chargerCompteJetons(xmlReader);
                    xmlReader.skipCurrentElement();
                }
            }
            file.close();
            nouveauJoueur.cartesJoaillerie = carte::chargerEtatPiocheJ(dir + "/cartesJ.xml");
            nouveauJoueur.reserveCartes = carte::chargerEtatPiocheJ(dir + "/cartesJR.xml");
            nouveauJoueur.cartesRoyale = carte::chargerEtatPioche(dir + "/cartesR.xml");
            return nouveauJoueur;
        }
        qDebug() << "Impossible d'ouvrir le fichier pour charger l'etat du joueur.";
        throw jeton::SplendorException("Impossible de charger les donnees");
    }

}
