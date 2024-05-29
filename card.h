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

namespace carte {
    enum class Capacite {
        rien,
        rejouer,
        associe,
        prendre_un_jeton,
        prendre_un_privilege,
        jeton_adversaire,
        pas_de_carte
    };

    string toString(const Capacite&);

    void toStringShort(string&, Capacite, size_t);

    class Carte {
    protected:
        size_t prestige;
        Capacite capacite;
        filesystem::path nomFichier;

    public:
        Carte(const size_t prest, const Capacite capa, const string& nf): prestige(prest), capacite(capa),
                                                                   nomFichier(nf) {}

        virtual ~Carte() = default;

        size_t getPrestige() const { return prestige; }
        Capacite getCapacite() const { return capacite; }
        filesystem::path getNomFichier(){return nomFichier;}

        virtual void sauvegarderEtat(QXmlStreamWriter& writer) {
            writer.writeAttribute("prestige", QString::number(prestige));
            writer.writeAttribute("capacite", QString::number(static_cast<int>(capacite)));
            writer.writeAttribute("nom_fichier", QString::fromStdString(nomFichier.string()));
        }

        static Carte* chargerEtat(const QXmlStreamReader& reader) {
            return new Carte(
                reader.attributes().value("prestige").toULongLong(),
                static_cast<Capacite>(reader.attributes().value("capacite").toInt()),
                reader.attributes().value("nom_fichier").toString().toStdString()
            );
        }
    };

    class CarteJoaillerie : public Carte {
        map<jeton::Couleur, size_t>* prix;
        size_t couronnes;
        jeton::Couleur couleurReduc;
        size_t nbReduc = 0;

    public:
        CarteJoaillerie(const size_t prestige, map<jeton::Couleur, size_t>* prix, const Capacite capacite,
                        const size_t couronnes,
                        const jeton::Couleur couleur, const size_t reduc,
                        const string& nf): Carte(prestige, capacite, nf), prix(prix),
                                    couronnes(couronnes), couleurReduc(couleur),
                                    nbReduc(reduc) {}

        ~CarteJoaillerie() override = default;

        size_t getCouronnes() const { return couronnes; }
        const map<jeton::Couleur, size_t>* getPrix() const { return prix; }
        jeton::Couleur getCouleurReduc() const { return couleurReduc; }
        size_t getNbReduc() const { return nbReduc; }

        void setCouleurReduc(const jeton::Couleur nouvelleCouleur) {
            couleurReduc = nouvelleCouleur;
        }

        void sauvegarderEtat(QXmlStreamWriter& writer) override {
            Carte::sauvegarderEtat(writer);
            writer.writeAttribute("couronnes", QString::number(couronnes));
            writer.writeAttribute("couleur_reduc", QString::number(static_cast<int>(couleurReduc)));
            writer.writeAttribute("nb_reduc", QString::number(nbReduc));
            writer.writeStartElement("prix");
            sauvegarderCompteJetons(*prix, writer);
        }

        static CarteJoaillerie* chargerEtat(QXmlStreamReader& reader) {
            const size_t prestige = reader.attributes().value("prestige").toULongLong();
            const auto capa = static_cast<Capacite>(reader.attributes().value("capacite").toInt());
            const string nomFichier = reader.attributes().value("nom_fichier").toString().toStdString();
            const size_t couronnes = reader.attributes().value("couronnes").toULongLong();
            const size_t nbReduc = reader.attributes().value("nb_reduc").toULongLong();
            const auto couleurReduc = static_cast<jeton::Couleur>(reader.attributes().value("couleur_reduc").toInt());
            map<jeton::Couleur, size_t> prix;
            reader.readNextStartElement();
            if (reader.name().toString() == "prix") {
                prix = jeton::chargerCompteJetons(reader);
                reader.skipCurrentElement();
            }
            return new CarteJoaillerie(prestige, new map(prix), capa, couronnes, couleurReduc, nbReduc, nomFichier);
        }
    };

    void sauvegarderEtatPioche(const vector<CarteJoaillerie *>&, const QString&);

    void sauvegarderEtatPioche(const vector<Carte *>&, const QString&);

    vector<CarteJoaillerie *> chargerEtatPiocheJ(const QString&);

    vector<Carte *> chargerEtatPioche(const QString&);

    ostream& operator<<(ostream& f, Capacite c);
}
