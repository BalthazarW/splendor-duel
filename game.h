#pragma once

#include "vector"
#include "random"
#include "tuple"
#include "player.h"
#include "board.h"
#include "card.h"

using namespace std;

class SplendorDuel;

namespace partie {
    filesystem::path getBasePath();

    void printSpacing(size_t, bool);

    void afficherLigneCartes(const vector<carte::CarteJoaillerie *>&, size_t, bool);

    class Partie {
        vector<joueur::Joueur> joueurs;
        joueur::Joueur* auTourDe;
        size_t actionsOptionnellesAccomplies = 0;
        plateau::Plateau plateau;
        plateau::Sachet sachet;
        size_t nbPrivilegesDispos = 3;
        vector<carte::CarteJoaillerie *> cartesNv1;
        vector<carte::CarteJoaillerie *> cartesNv2;
        vector<carte::CarteJoaillerie *> cartesNv3;
        vector<carte::Carte *> cartesRoyales;
        bool fini = false;

        friend class ::SplendorDuel;
    public:
        Partie(const joueur::Joueur& joueur1, const joueur::Joueur& joueur2, const bool nouvellePartie) {
            joueurs.push_back(joueur1);
            joueurs.push_back(joueur2);
            if (nouvellePartie) {
                random_device dev;
                mt19937 rng(dev());
                uniform_int_distribution<mt19937::result_type> dist2(0, 1);
                const auto result = dist2(rng);
                auTourDe = &joueurs[result];
                plateau.remplirPlateau(sachet);
                instancierJoaillerie();
                instancierRoyale();
            }
        }

        vector<carte::CarteJoaillerie *> getCartesNv1() const { return cartesNv1; }
        vector<carte::CarteJoaillerie *> getCartesNv2() const { return cartesNv2; }
        vector<carte::CarteJoaillerie *> getCartesNv3() const { return cartesNv3; }
        joueur::Joueur* getauTourDe() const { return auTourDe; }
        plateau::Sachet getSachet() { return sachet; }
        vector<joueur::Joueur> getJoueurs(){return joueurs;}


        bool estFinie() const { return fini; }

        void instancierJoaillerie(const string& = "../data/cards.data");

        void instancierRoyale(const string& = "../data/royal_cards.data");

        void afficherPyramide() const {
            afficherLigneCartes(cartesNv3, 3, true);
            afficherLigneCartes(cartesNv2, 4, true);
            afficherLigneCartes(cartesNv1, 5, true);
        }

        void afficherCartesRoyale(const vector<carte::Carte*>&) const;

        void jouer();

        tuple<bool, bool> actionOptionnelle();

        tuple<bool, bool> actionOptionnelleIA();

        bool actionObligatoire();

        bool actionObligatoireIA();

        vector<tuple<size_t, size_t>> choisirJetons(bool, bool);

        vector<tuple<size_t, size_t>> choisirJetonsIA(bool, bool, bool , jeton::Couleur);

        tuple<vector<carte::CarteJoaillerie *>, size_t> choisirCarte(bool);

        tuple<vector<carte::CarteJoaillerie *>, size_t> choisirCarteIA(bool);

        size_t choisirCarteRoyale() const;

        void utiliserPrivilege(const jeton::Jeton&);

        void prendreJetons(size_t, size_t);

        bool peutAcheter(const carte::CarteJoaillerie*) const;

        bool peutReserver() const;

        void prendreCarteRoyale(size_t);

        void recupererCarte(const vector<carte::CarteJoaillerie *>&, size_t, bool);

        void acheterCarteJoaillerie(const vector<carte::CarteJoaillerie *>&, size_t);

        bool verifFinTour();

        bool verifVictoire() const;

        void prendreUnPrivilege();

        void changeJoueur();

        void demanderReposerJetonsTerminal(size_t nb);

        void demanderReposerJetonsTerminalIA(size_t nb);

        void appelCapaciteCarte(const vector<carte::CarteJoaillerie *>& container, size_t index);

        void jetonAdversaire();

        void jetonAdversaireIA();

        void jetonPlateauCapacite(jeton::Couleur c);

        void sauvegarderEtat(const QString& dir = (getBasePath() / "save").string().c_str()) const {
            QFile file(dir + "/partie.xml");
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QXmlStreamWriter xmlWriter(&file);
                xmlWriter.setAutoFormatting(true);

                // Écriture des données dans le fichier XML
                xmlWriter.writeStartDocument();
                xmlWriter.writeStartElement("partie");

                xmlWriter.writeAttribute("nbAcOpt", QString::number(actionsOptionnellesAccomplies));
                xmlWriter.writeAttribute("nbPrivi", QString::number(nbPrivilegesDispos));
                int i = 0;
                if (auTourDe != &joueurs[0])
                    i = 1;
                xmlWriter.writeAttribute("joueurAct", QString::number(i));

                xmlWriter.writeEndElement();
                xmlWriter.writeEndDocument();

                file.close();
                if (QFile fileRoy(dir + "/royales.xml"); fileRoy.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QXmlStreamWriter xmlWriter2(&fileRoy);
                    xmlWriter2.setAutoFormatting(true);

                    // Écriture des données dans le fichier XML
                    xmlWriter2.writeStartDocument();
                    xmlWriter2.writeStartElement("pioche");

                    for (const auto carte : cartesRoyales) {
                        if (carte == nullptr) {
                            xmlWriter2.writeStartElement("carte");
                            xmlWriter2.writeAttribute("valide", nullptr);
                            xmlWriter2.writeEndElement();
                        }
                        // else carte->sauvegarderEtat(xmlWriter);
                    }

                    xmlWriter2.writeEndElement();
                    xmlWriter2.writeEndDocument();

                    file.close();
                } else {
                    qDebug() << "Impossible d'ouvrir le fichier pour sauvegarder l'etat de la pioche.";
                }
                joueurs[0].sauvegarderEtat(dir + "/player1");
                joueurs[1].sauvegarderEtat(dir + "/player2");
                plateau.sauvegarderEtat(dir + "/plateau.xml");
                sachet.sauvegarderEtat(dir + "/sachet.xml");
                sauvegarderEtatPioche(cartesNv1, dir + "/piocheNv1.xml");
                sauvegarderEtatPioche(cartesNv2, dir + "/piocheNv2.xml");
                sauvegarderEtatPioche(cartesNv3, dir + "/piocheNv3.xml");
                sauvegarderEtatPioche(cartesRoyales, dir + "/royales.xml");
            } else {
                qDebug() << "Impossible d'ouvrir le fichier pour sauvegarder l'etat de la partie.";
            }
        }

        static Partie* chargerEtat(const QString& dir = (getBasePath() / "save").string().c_str()) {
            joueur::Joueur joueur1 = joueur::Joueur::chargerEtat(dir + "/player1");
            joueur::Joueur joueur2 = joueur::Joueur::chargerEtat(dir + "/player2");
            auto nouvellePartie = new Partie(joueur1, joueur2, false);
            QFile file(dir + "/partie.xml");
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QXmlStreamReader xmlReader(&file);

                while (!xmlReader.atEnd() && !xmlReader.hasError()) {
                    const QXmlStreamReader::TokenType token = xmlReader.readNext();

                    if (token == QXmlStreamReader::StartDocument) {
                        continue;
                    }
                    if (token == QXmlStreamReader::StartElement && xmlReader.name().toString() == "partie") {
                        nouvellePartie->actionsOptionnellesAccomplies = xmlReader.attributes().value("nbAcOpt").toInt();
                        nouvellePartie->nbPrivilegesDispos = xmlReader.attributes().value("nbPrivi").toInt();
                        nouvellePartie->auTourDe = &nouvellePartie->joueurs[xmlReader.attributes().value("joueurAct").toInt()];

                        xmlReader.skipCurrentElement();
                    }
                }
                file.close();
                nouvellePartie->cartesNv1 = carte::chargerEtatPiocheJ(dir + "/piocheNv1.xml");
                nouvellePartie->cartesNv2 = carte::chargerEtatPiocheJ(dir + "/piocheNv2.xml");
                nouvellePartie->cartesNv3 = carte::chargerEtatPiocheJ(dir + "/piocheNv3.xml");
                nouvellePartie->cartesRoyales = carte::chargerEtatPioche(dir + "/royales.xml");
                nouvellePartie->plateau.chargerEtat(dir + "/plateau.xml");
                nouvellePartie->sachet.chargerEtat(dir + "/sachet.xml");
                return nouvellePartie;
            }
            qDebug() << "Impossible d'ouvrir le fichier pour charger l'etat de la partie.";
            throw jeton::SplendorException("Impossible de charger la partie");
        }
    };

    bool verifCoord(const vector<tuple<size_t, size_t>>&, size_t, size_t, size_t);

    double distance(const tuple<size_t, size_t>&, const tuple<size_t, size_t>&);

    bool verifProxi(const vector<tuple<size_t, size_t>>&);

    bool verifColineaire(const vector<tuple<size_t, size_t>>&);

    bool verifNbCouleurs(map<jeton::Couleur, int>);

    double distance(const tuple<size_t, size_t>&, const tuple<size_t, size_t>&);

    bool verifProxi(const vector<tuple<size_t, size_t>>&);

    bool verifColineaire(const vector<tuple<size_t, size_t>>&);

}
