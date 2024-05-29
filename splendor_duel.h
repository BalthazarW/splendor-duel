#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QPushButton>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QMap>
#include <QDialog>
#include <QLabel>
#include "card.h"
#include "token.h"
#include "player.h"
#include "game.h"

class CarteWidget : public QLabel {
Q_OBJECT

public:
    CarteWidget(carte::CarteJoaillerie* carte, QWidget* parent = nullptr)
            : QLabel(parent), carte_(carte) {
    if (carte)
        image =initUI(carte->getNomFichier());
    }

    ~CarteWidget() {
        delete carte_;
    }

    QLabel* initUI(const filesystem::path&);
    carte::CarteJoaillerie* getCarte() {return carte_;}
    QLabel* getImage(){ return image;};

private:
    carte::CarteJoaillerie* carte_;
    QLabel* image;

};


class JoueurWidget : public QLabel {
Q_OBJECT

public:
    JoueurWidget(joueur::Joueur* j, QWidget* parent = nullptr)
            : QLabel(parent), joueur_(j),nombreCartesReservees(0) {
        initialiserPointsCouronne();

        fenetreCartesAchetees = new QDialog(this);
        fenetreCartesAchetees->setWindowTitle("Cartes Achetées");

        QVBoxLayout *layoutVertical = new QVBoxLayout(fenetreCartesAchetees);
        gridLayoutCartesAchetees = new QGridLayout;
        layoutVertical->addLayout(gridLayoutCartesAchetees);
        fenetreCartesReservees = new QDialog(this);
        fenetreCartesReservees->setWindowTitle("Cartes Reservées");
        QVBoxLayout *layout = new QVBoxLayout(fenetreCartesReservees);
        gridLayoutCartesReservees = new QGridLayout;
        layout->addLayout(gridLayoutCartesReservees);
    }

    ~JoueurWidget() {
        delete joueur_;
    }
    joueur::Joueur* getJoueur() const {
        return joueur_;
    }
    const QMap<QString, int>& getPointsCouronne() const {
        return pointsCouronne;
    }
    void setPointsCouronne(const QString& key, int value) {
        pointsCouronne[key] = value;
    }
    QDialog* getFenetreCartesAchetees() const {
        return fenetreCartesAchetees;
    }
    QDialog* getFenetreCartesReservees() const {
        return fenetreCartesReservees;
    }
    void setFenetreCartesAchetees(QDialog* newFenetreCartesAchetees) {
        fenetreCartesAchetees = newFenetreCartesAchetees;
    }
    QGridLayout* getGridLayoutCartesAchetees() const {
        return gridLayoutCartesAchetees;
    }
    QGridLayout* getGridLayoutCartesReservees() const {
        return gridLayoutCartesReservees;
    }
    void setGridLayoutCartesAchetees(QGridLayout* newGridLayoutCartesAchetees) {
        gridLayoutCartesAchetees = newGridLayoutCartesAchetees;
    }
    void initialiserPointsCouronne();

    int getNombreCartesReservees(){return nombreCartesReservees;}
    void setNombreCartesReservees(int i){nombreCartesReservees=i;}

private:
    joueur::Joueur* joueur_;
    QMap<QString, int> pointsCouronne;
    QDialog *fenetreCartesAchetees;
    QDialog *fenetreCartesReservees;
    QGridLayout *gridLayoutCartesAchetees;
    QGridLayout *gridLayoutCartesReservees;
    int nombreCartesReservees;
};



class SplendorDuel : public QWidget {
Q_OBJECT

public:
    SplendorDuel(QWidget *parent = nullptr);
    void updateJetonsJoueur();
    void boutonClique(int row, int col, QPushButton *button);
    void reserverCarte(CarteWidget *c, const vector<carte::CarteJoaillerie*> &container, int index);
    void acheterCarte(CarteWidget* c, const vector<carte::CarteJoaillerie*> &container, int index);
    bool appelCapaciteCarte(CarteWidget*);
    jeton::Couleur choixParmiCouleurs(const vector<QString>&, const string&, const string&);
    void choisirCouleurBonus(const vector<carte::CarteJoaillerie *>&, size_t);
    void piocheClique();
    QGridLayout* createGridCartesJoueur();
    QGridLayout* createGridCartesRoyales();
    void initPlateau(bool = false);
    void initJetonsCoursAchat();
    void setupUI();

    void changementJoueur();
    void verifFinTour();
    bool verifVictoire();

    void replacerJeton(QPushButton*, QPushButton*);
    void acheterJetonsEnAttente();

    void createPyramide();
    void updatePyramide();

    void prendreUnJeton(const string&, bool);

    void lancerIA();

    ~SplendorDuel() { if (mypartie) {
        if (!mypartie->fini)
            mypartie->sauvegarderEtat();
    }
    }

private:
    QGridLayout *mainLayout;
    QGridLayout *layoutJetons;
    QGridLayout *jetonsEnAttente;
    vector<tuple<size_t, size_t>> coordsJetonsEnAttente;
    bool jetonBonus = false;
    jeton::Couleur couleurAPrendre = jeton::Couleur::perle;
    QGridLayout *plateauJetons;
    QGridLayout *gridLayout2;
    QGridLayout *gridLayout3;
    partie::Partie* mypartie=nullptr;
    QMap<QString, QLabel*> labelsJoueurs;
    size_t compteur_action_obligatoire;
    QLabel *labelPrestige;
    QLabel *labelCouronne;
    QLabel *labelCapacite;
    QLabel *labelReduction;
    JoueurWidget* auTourDe_;
    vector <JoueurWidget*> joueurs;
    QDialog *fenetreCartesRoyales;
    QGridLayout *gridLayoutCartesRoyales;


private slots:
    void afficherFenetreCartesAchetees();
    void afficherFenetreCartesReservees();
    void afficherFenetreCartesRoyales();
};

