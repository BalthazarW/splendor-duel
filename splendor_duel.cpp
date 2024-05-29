#include "splendor_duel.h"
#include <QMessageBox>
#include <QApplication>
#include <QLabel>
#include <QInputDialog>
#include <QString>
#include <algorithm>
#include "player.h"
#include "game.h"
#include "controller.h"

QLabel* CarteWidget::initUI(const filesystem::path& nomFichier) {
    QLabel* labelt = new QLabel;

    QPixmap tmpImage(QString::fromStdString(nomFichier.string()));

    labelt->setMaximumSize(72, 120);

    QSize imageSize = tmpImage.size();
    imageSize.scale(labelt->maximumSize(), Qt::KeepAspectRatio);

    QPixmap scaledImage = tmpImage.scaled(imageSize, Qt::KeepAspectRatio);
    labelt->setPixmap(scaledImage);
    labelt->setScaledContents(true);
    return labelt;
}

SplendorDuel::SplendorDuel(QWidget *parent) : QWidget(parent),compteur_action_obligatoire(0) {
    setWindowTitle("Splendor duel"); // TODO : handle quit before launching app
    int continuerPartie;
    if (const filesystem::path memoryPath = partie::getBasePath() / "save" / "partie.xml"; splendor_duel::fichierMemoireExiste(memoryPath))
        continuerPartie = QMessageBox::question(this, "Partie en mémoire", "Une partie est en cours, continuer ?", QMessageBox::Yes | QMessageBox::No);
    JoueurWidget* widgetJoueur1;
    JoueurWidget* widgetJoueur2;
    if (continuerPartie == QMessageBox::Yes) {
        mypartie = partie::Partie::chargerEtat();
        widgetJoueur1 = new JoueurWidget(&mypartie->joueurs[0]);
        widgetJoueur2 = new JoueurWidget(&mypartie->joueurs[1]);
    }
    else {
        bool ok;

        auto joueur1IA = QMessageBox::question(nullptr, "Status Joueur 1", "Le Joueur 1 sera-t-il une IA ?", QMessageBox::Yes | QMessageBox::No);
        int nvIAJ1 = 0;
        QString nomJoueur1 = "Bot Alpha";
        if (joueur1IA == QMessageBox::Yes) {
            nvIAJ1 = QInputDialog::getInt(nullptr, "Niveau d'IA", "Niveau de l'IA :\n1 (aleatoire)", 1, 1, 1, 1, &ok);
        } else {
            nomJoueur1 = QInputDialog::getText(nullptr, "Saisir Joueur 1", "Entrer le nom du joueur 1:",
                                               QLineEdit::Normal, "", &ok);

            if (ok && !nomJoueur1.isEmpty()) {
                qDebug() << "Nom saisi : " << nomJoueur1;
            } else {
                qDebug() << "Saisie annulée";
            }
        }
        std::string nomJ1 = nomJoueur1.toStdString();
        joueur::Joueur joueur1(nomJ1, bool(nvIAJ1), nvIAJ1);
        widgetJoueur1 = new JoueurWidget(&joueur1);

        auto joueur2IA = QMessageBox::question(nullptr, "Status Joueur 2", "Le Joueur 2 sera-t-il une IA ?", QMessageBox::Yes | QMessageBox::No);
        int nvIAJ2 = 0;
        QString nomJoueur2 = "Bot Beta";
        if (joueur2IA == QMessageBox::Yes) {
            nvIAJ2 = QInputDialog::getInt(nullptr, "Niveau d'IA", "Niveau de l'IA :\n1 (aleatoire)", 1, 1, 1, 1, &ok);
        } else {
            nomJoueur2 = QInputDialog::getText(nullptr, "Saisir Joueur 2", "Entrer le nom du joueur 2:",
                                               QLineEdit::Normal, "", &ok);

            if (ok && !nomJoueur2.isEmpty()) {
                qDebug() << "Nom saisi : " << nomJoueur2;
            } else {
                qDebug() << "Saisie annulée";
            }
        }
        std::string nomJ2 = nomJoueur2.toStdString();
        joueur::Joueur joueur2(nomJ2, bool(nvIAJ2), nvIAJ2);
        widgetJoueur2 = new JoueurWidget(&joueur2);
        mypartie = new partie::Partie(joueur1, joueur2, true);
    }
    setupUI();
    joueurs.push_back(widgetJoueur1);
    joueurs.push_back(widgetJoueur2);

    if (mypartie->getauTourDe()==&mypartie->getJoueurs()[0]){
        auTourDe_= joueurs[0];
    }else{
        auTourDe_= joueurs[1];
    }
}

void SplendorDuel::lancerIA() {
    bool continuer = true;
    bool quitter = false;
    size_t k = 0;
    while (mypartie->actionsOptionnellesAccomplies < 2 && continuer) {
        k++;
        tie(continuer, quitter) = mypartie->actionOptionnelleIA();
        if (quitter && !continuer)
            return;
        if (!quitter || !continuer)
            mypartie->actionsOptionnellesAccomplies++;
    }
    continuer = false;
    continuer = mypartie->actionObligatoireIA();
    if (!continuer) {
        return; // pour ne pas changer 2 fois de joueur
    }
    mypartie->actionsOptionnellesAccomplies = 0;
    mypartie->verifFinTour();
    initPlateau();
    updateJetonsJoueur();
    updatePyramide();
}

void SplendorDuel::setupUI() {
    mainLayout = new QGridLayout(this);
    layoutJetons = new QGridLayout;
    initPlateau();
    initJetonsCoursAchat();
    layoutJetons->addLayout(jetonsEnAttente, 0, 0);
    mainLayout->addLayout(layoutJetons, 1, 0);
    auto boutonsLayout = new QGridLayout;
    QPushButton *piocheButton = new QPushButton("Remplir plateau");
//    piocheButton->setFixedSize(120, 30);
    connect(piocheButton, &QPushButton::clicked, this, &SplendorDuel::piocheClique);
    boutonsLayout->addWidget(piocheButton, 0, 0);
    QPushButton *priviButton = new QPushButton("Utiliser Privilège");
//    piocheButton->setFixedSize(120, 30);
    connect(priviButton, &QPushButton::clicked, this, [=]() {
        jetonBonus = true;
        prendreUnJeton("Prendre un unique jeton", false);
        layoutJetons->setParent(nullptr);
        mainLayout->addLayout(layoutJetons, 1, 0);
        initPlateau();
        jetonBonus = false;
    });
    priviButton->setDisabled(true);
    boutonsLayout->addWidget(priviButton, 0, 1);
    mainLayout->addLayout(boutonsLayout, 0, 0);
    QPushButton *startIA = new QPushButton("Lancer IAs");
//    piocheButton->setFixedSize(120, 30);
    auto joueurs = mypartie->joueurs;
    if (std::all_of(joueurs.begin(), joueurs.end(),
                    [](auto joueur) { return joueur.isIA(); })) {
        connect(startIA, &QPushButton::clicked, this, [=]() {
            bool aGagne = false;
            while (!aGagne) {
                lancerIA();
                aGagne = mypartie->verifFinTour();
                if (!aGagne)
                    changementJoueur();
            }
            mypartie->fini = true;
            string winText = "Le joueur ";
            winText.append(mypartie->auTourDe->getPseudo());
            winText.append(" a gagné ! Bien joué !");
            QMessageBox::information(this, "Fin de la partie", QString::fromStdString(winText), QMessageBox::Ok);
        });
        mainLayout->addWidget(startIA, 0, 1);
    } else if (mypartie->auTourDe->isIA()) {
        connect(startIA, &QPushButton::clicked, this, [=]() {
            lancerIA();
            changementJoueur();
            startIA->setDisabled(true);
        });
        mainLayout->addWidget(startIA, 0, 1);
    }

    string pseudoJoueur = "Joueur :\n";
    pseudoJoueur.append(mypartie->auTourDe->getPseudo());
    auto pseudoLabel = new QLabel(QString::fromStdString(pseudoJoueur));
//    pseudoLabel->setFixedSize(120, 30);
    mainLayout->addWidget(pseudoLabel, 0, 3);

    createPyramide();
    mainLayout->addLayout(gridLayout2, 1, 1);

    gridLayout3 = createGridCartesJoueur();
    mainLayout->addLayout(gridLayout3, 2, 3);

    QPushButton *button = new QPushButton("Voir Cartes Reservées");
    button->setFixedSize(120, 30);
    mainLayout->addWidget(button, 2, 1);
    connect(button, &QPushButton::clicked, this, &SplendorDuel::afficherFenetreCartesReservees);

    updateJetonsJoueur();

    fenetreCartesRoyales = new QDialog(this);
    fenetreCartesRoyales->setWindowTitle("Cartes Royales");

    QVBoxLayout *layoutVertical = new QVBoxLayout(fenetreCartesRoyales);
    gridLayoutCartesRoyales = new QGridLayout;
    gridLayoutCartesRoyales=createGridCartesRoyales();
    layoutVertical->addLayout(gridLayoutCartesRoyales);
    QPushButton *buttonRoyales = new QPushButton("Voir Cartes Royales");
    buttonRoyales->setFixedSize(120, 30);
    mainLayout->addWidget(buttonRoyales, 1, 2);
    connect(buttonRoyales, &QPushButton::clicked, this, &SplendorDuel::afficherFenetreCartesRoyales);


}

QGridLayout* SplendorDuel::createGridCartesRoyales(){
    QGridLayout* grid = new QGridLayout;
    for( int i=0; i<4; i++){
        QLabel* labelt = new QLabel;
        QGridLayout *miniLayout = new QGridLayout;
        QPixmap tmpImage(QString::fromStdString(mypartie->cartesRoyales[i]->getNomFichier().string()));
        labelt->setMaximumSize(72, 120);
        QSize imageSize = tmpImage.size();
        imageSize.scale(labelt->maximumSize(), Qt::KeepAspectRatio);
        QPixmap scaledImage = tmpImage.scaled(imageSize, Qt::KeepAspectRatio);
        labelt->setPixmap(scaledImage);
        labelt->setScaledContents(true);
        miniLayout->addWidget(labelt, 0,0);
        QPushButton* boutonAcheter = new QPushButton("Acheter");
        boutonAcheter->setFixedSize(50, 20);
        connect(boutonAcheter, &QPushButton::clicked, this, [=]() {
            if (mypartie->auTourDe->getNbCouronnes() >= 3 && mypartie->auTourDe->getCartesRoyale().size() < 2) {
                mypartie->prendreCarteRoyale(i);
                mypartie->auTourDe->retirerCouronne(3);
                boutonAcheter->setDisabled(true);
            } else {
                QMessageBox::information(nullptr, "Achat impossible", "Impossible de récupérer une carte Royale", QMessageBox::Ok);
            }
        });
        miniLayout->addWidget(boutonAcheter, 1,0);
        QWidget* widgetContainer = new QWidget;
        widgetContainer->setLayout(miniLayout);
        grid->addWidget(widgetContainer, 0, i, 1, 1);

    }
    return grid;
}




void SplendorDuel::verifFinTour() {
    while (mypartie->auTourDe->getNbJetons() > 10) {
        string text = "Choisissez la couleur du jeton à reposer (";
        text.append(to_string(mypartie->auTourDe->getNbJetons() - 10));
        text.append(" restant(s)) : ");
        vector<QString> choixValides;
        for (const auto couleur: jeton::Couleurs) {
            text.append(toString(couleur));
            text.append("  ");
            choixValides.push_back(QString::fromStdString(toString(couleur)));
        }
        const jeton::Couleur couleurChoisie = choixParmiCouleurs(choixValides, "Bonus libre", text);
        mypartie->auTourDe->enleverJeton(couleurChoisie);
        mypartie->sachet.remettreJetonDansSachet(couleurChoisie);
        updateJetonsJoueur();
    }
}

bool SplendorDuel::verifVictoire() {
    return mypartie->auTourDe->cumul10PointsPrestigeCouleur() || mypartie->auTourDe->getNbCouronnes() >= 10 ||
    mypartie->auTourDe->getNbPrestiges() >= 20;
}

void SplendorDuel::changementJoueur() {
    verifFinTour();
    if (verifVictoire()) {
        string winText = "Le joueur ";
        winText.append(mypartie->auTourDe->getPseudo());
        winText.append(" a gagné ! Bien joué !");
        QMessageBox::information(this, "Fin de la partie", QString::fromStdString(winText), QMessageBox::Ok);
    }
    mypartie->changeJoueur();
    compteur_action_obligatoire=0;
    if (auTourDe_== joueurs[0]){
        auTourDe_= joueurs[1];
    }else{
        auTourDe_= joueurs[0];
    }
    QMessageBox::information(this, "Changement de joueur", "Veuillez laisser la place à l'autre joueur.", QMessageBox::Ok);
    auto pseudoLabel = dynamic_cast<QLabel*>(mainLayout->itemAtPosition(0, 3)->widget());
    if (pseudoLabel) {
        string pseudoJoueur = "Joueur :\n";
        pseudoJoueur.append(mypartie->auTourDe->getPseudo());
        pseudoLabel->setText(QString::fromStdString(pseudoJoueur));
    }
    labelPrestige->setText("Point de Prestige total: " + QString::number(auTourDe_->getPointsCouronne()["Point de Prestige total"]));
    labelCouronne->setText("Couronne: " + QString::number(auTourDe_->getPointsCouronne()["Couronne"]));
    labelReduction->setText("Couronne: " + QString::number(auTourDe_->getPointsCouronne()["Reduction"]));
    updateJetonsJoueur();
    if (mypartie->auTourDe->isIA()) {
        lancerIA();
        changementJoueur();
    }
    //if (mypartie->verifVictoire())
}

QGridLayout *SplendorDuel::createGridCartesJoueur() {
    QGridLayout *gridLayout = new QGridLayout;
    labelPrestige = new QLabel("Point de Prestige total: 0");
    labelCouronne = new QLabel("Couronne: 0");
    labelReduction = new QLabel ("Reduction : 0");
    gridLayout->addWidget(labelPrestige, 0, 0);
    gridLayout->addWidget(labelCouronne, 1, 0);
    gridLayout->addWidget(labelReduction, 2, 0);

    QPushButton *button = new QPushButton("Voir Cartes Achetées");
    button->setFixedSize(120, 30);
    gridLayout->addWidget(button, 2, 0);
    connect(button, &QPushButton::clicked, this, &SplendorDuel::afficherFenetreCartesAchetees);
    return gridLayout;
}

void SplendorDuel::afficherFenetreCartesRoyales() {
    fenetreCartesRoyales->exec();
}


void SplendorDuel::afficherFenetreCartesAchetees() {
    auTourDe_->getFenetreCartesAchetees()->exec();
}
void SplendorDuel::afficherFenetreCartesReservees() {
    auTourDe_->getFenetreCartesReservees()->exec();
}

void SplendorDuel::initJetonsCoursAchat() {
    jetonsEnAttente = new QGridLayout;
    for (int col = 0; col < 3; col++) {
        QPushButton* button = new QPushButton;
        button->setText("#");
        button->setDisabled(true);
        button->setFixedSize(30, 30);
        button->setContentsMargins(0, 0, 0, 0);

        jetonsEnAttente->addWidget(button, 0, col);
    }
    auto buyButton = new QPushButton;
    buyButton->setText("Prendre jeton(s)");
    buyButton->setDisabled(true);
    buyButton->setContentsMargins(0, 0, 0, 0);
    connect(buyButton, &QPushButton::clicked, [=]() {
        acheterJetonsEnAttente();
    });
    jetonsEnAttente->addWidget(buyButton, 0, 3);
}

void SplendorDuel::initPlateau(bool gold) {
    plateauJetons = new QGridLayout;
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 5; ++col) {
            QPushButton *button = new QPushButton;
            const jeton::Jeton* jeton = mypartie->plateau.getJetons()[mypartie->plateau.getPosition(row, col)];
            if (jeton != nullptr) {
                auto couleur = jeton->getCouleur();
                button->setText(QString::fromStdString(toString(couleur)));
                if ((couleur == jeton::Couleur::gold && !gold) || (couleur != jeton::Couleur::gold && gold))
                    button->setDisabled(true);
            }
            else {
                button->setText("#");
                button->setDisabled(true);
            }

            button->setFixedSize(30, 30);
            button->setContentsMargins(0, 0, 0, 0);

            connect(button, &QPushButton::clicked, [=]() {
                boutonClique(row, col, button);
            });

            plateauJetons->addWidget(button, row, col);
            plateauJetons->setColumnStretch(col, 0);
        }
        plateauJetons->setRowStretch(row, 0);
    }
    layoutJetons->addLayout(plateauJetons, 1, 0);
}

void SplendorDuel::createPyramide() {
    gridLayout2 = new QGridLayout;
    if (0 < mypartie->getCartesNv3().size()) {
        QGridLayout *miniLayout = new QGridLayout;
        CarteWidget *carteWidget = new CarteWidget(mypartie->getCartesNv3()[3]);
        QLabel* label = carteWidget->initUI(partie::getBasePath() / "assets" / "cards_lvl3" / "dos.png");
        miniLayout->addWidget(label, 0,0);
        QPushButton *boutonReserver = new QPushButton("Réserver");
        boutonReserver->setFixedSize(50, 20);
        connect(boutonReserver, &QPushButton::clicked, [=]() {
            reserverCarte(carteWidget, mypartie->cartesNv3, mypartie->cartesNv3.size() - 1);
        });
        miniLayout->addWidget(boutonReserver, 1, 0);
        QWidget *widgetContainer = new QWidget;
        widgetContainer->setLayout(miniLayout);
        gridLayout2->addWidget(widgetContainer, 1, 0, 1, 2);
    }
    if (0 < mypartie->getCartesNv2().size()) {
        QGridLayout *miniLayout = new QGridLayout;
        CarteWidget* carteWidget = new CarteWidget(mypartie->getCartesNv2()[4]);
        QLabel* label= carteWidget->initUI(partie::getBasePath() / "assets" / "cards_lvl2" / "dos.png");
        miniLayout->addWidget(label, 0,0);
// Ajout du bouton "Réserver"
        QPushButton* boutonReserver = new QPushButton("Réserver");
        boutonReserver->setFixedSize(50, 20);
        connect(boutonReserver, &QPushButton::clicked, [=]() {
            reserverCarte(carteWidget, mypartie->cartesNv2, mypartie->cartesNv2.size() - 1);
        });
        miniLayout->addWidget(boutonReserver, 1,0);

        QWidget* widgetContainer = new QWidget;
        widgetContainer->setLayout(miniLayout);
        gridLayout2->addWidget(widgetContainer, 2, 0, 1, 2);
    }
    if (0 < mypartie->getCartesNv1().size()) {
        QGridLayout *miniLayout = new QGridLayout;
        CarteWidget* carteWidget = new CarteWidget(mypartie->getCartesNv1()[5]);
        QLabel* label= carteWidget->initUI(partie::getBasePath() / "assets" / "cards_lvl1" / "dos.png");
        miniLayout->addWidget(label, 0,0);

        QPushButton* boutonReserver = new QPushButton("Réserver");
        boutonReserver->setFixedSize(50, 20);

        connect(boutonReserver, &QPushButton::clicked, [=]() {
            reserverCarte(carteWidget, mypartie->cartesNv1, mypartie->cartesNv1.size() - 1);
        });
        miniLayout->addWidget(boutonReserver, 1,0);

        QWidget* widgetContainer = new QWidget;
        widgetContainer->setLayout(miniLayout);
        gridLayout2->addWidget(widgetContainer, 3, 0, 1, 2);
    }

    for (int i = 1; i < 4; ++i) {
        for (int j = 1; j < i + 3; ++j) {

            if (i == 1) {
                if (j <= mypartie->getCartesNv3().size()) {
                    QGridLayout *miniLayout = new QGridLayout;
                    CarteWidget* carteWidget = new CarteWidget(mypartie->getCartesNv3()[j-1]);
                    miniLayout->addWidget(carteWidget->getImage(), 0,0);
                    QPushButton* boutonReserver = new QPushButton("Réserver");
                    boutonReserver->setFixedSize(50, 20);
                    connect(boutonReserver, &QPushButton::clicked, [=]() {
                        reserverCarte(carteWidget, mypartie->cartesNv3, j - 1);
                    });
                    miniLayout->addWidget(boutonReserver, 1,0);

                    QPushButton* boutonAcheter = new QPushButton("Acheter");
                    // connect(boutonAcheter, &QPushButton::clicked, this, &CarteWidget::acheterCarte);
                    boutonAcheter->setFixedSize(50, 20);
                    connect(boutonAcheter, &QPushButton::clicked, [=]() {
                        acheterCarte(carteWidget, mypartie->cartesNv3, j - 1);
                    });
                    miniLayout->addWidget(boutonAcheter, 2,0);
                    QWidget* widgetContainer = new QWidget;
                    widgetContainer->setLayout(miniLayout);
                    gridLayout2->addWidget(widgetContainer, i, j * 2, 1, 2);

                }
            } else if (i == 2) {
                if (j <= mypartie->getCartesNv2().size()) {
                    QGridLayout *miniLayout = new QGridLayout;
                    CarteWidget* carteWidget = new CarteWidget(mypartie->getCartesNv2()[j-1]);
                    miniLayout->addWidget(carteWidget->getImage(), 0,0);
                    // Ajout du bouton "Réserver"
                    QPushButton* boutonReserver = new QPushButton("Réserver");
                    boutonReserver->setFixedSize(50, 20);
                    boutonReserver->setFixedSize(50, 20);
                    connect(boutonReserver, &QPushButton::clicked, [=]() {
                        reserverCarte(carteWidget, mypartie->cartesNv2, j - 1);
                    });
                    miniLayout->addWidget(boutonReserver, 1,0);

                    // Ajout du bouton "Acheter"
                    QPushButton* boutonAcheter = new QPushButton("Acheter");
                    connect(boutonAcheter, &QPushButton::clicked, [=]() {
                        acheterCarte(carteWidget, mypartie->cartesNv2, j - 1);
                    });
                    boutonAcheter->setFixedSize(50, 20);
                    miniLayout->addWidget(boutonAcheter, 2,0);
                    QWidget* widgetContainer = new QWidget;
                    widgetContainer->setLayout(miniLayout);
                    gridLayout2->addWidget(widgetContainer, i, j * 2, 1, 2);
                }
            }else if (i == 3) {
                if (j <= mypartie->getCartesNv1().size()) {
                    QGridLayout *miniLayout = new QGridLayout;
                    CarteWidget* carteWidget = new CarteWidget(mypartie->getCartesNv1()[j-1]);
                    miniLayout->addWidget(carteWidget->getImage(), 0,0);

                    QPushButton* boutonReserver = new QPushButton("Réserver");
                    boutonReserver->setFixedSize(50, 20);
                    boutonReserver->setFixedSize(50, 20);
                    connect(boutonReserver, &QPushButton::clicked, [=]() {
                        reserverCarte(carteWidget, mypartie->cartesNv1, j - 1);
                    });
                    miniLayout->addWidget(boutonReserver, 1,0);

                    // Ajout du bouton "Acheter"
                    QPushButton* boutonAcheter = new QPushButton("Acheter");

                    boutonAcheter->setFixedSize(50, 20);
                    miniLayout->addWidget(boutonAcheter, 2,0);
                    connect(boutonAcheter, &QPushButton::clicked, [=]() {
                        acheterCarte(carteWidget, mypartie->cartesNv1, j - 1);
                    });
                    QWidget* widgetContainer = new QWidget;
                    widgetContainer->setLayout(miniLayout);
                    gridLayout2->addWidget(widgetContainer, i, j * 2, 1, 2);
                }
            }
        }
    }
}

void SplendorDuel::updatePyramide() {
    auto piochesCartes = vector<vector<carte::CarteJoaillerie*>>{mypartie->cartesNv3, mypartie->cartesNv2, mypartie->cartesNv1};
    for (int i = 0; i < 3; i++) {
        auto layoutPioche = dynamic_cast<QGridLayout*>(gridLayout2->itemAtPosition(i + 1, 0)->widget()->layout());
        if  (layoutPioche) {
            auto boutonReservePioche = dynamic_cast<QPushButton*>(layoutPioche->itemAtPosition(1, 0)->widget());
            if (boutonReservePioche) {
                if (piochesCartes[i].size() <= 3 + i)
                    boutonReservePioche->setDisabled(true);
                else {
                    CarteWidget *carteWidget = new CarteWidget(piochesCartes[i][piochesCartes[i].size() - 1]);
                    connect(boutonReservePioche, &QPushButton::clicked, [=]() {
                        reserverCarte(carteWidget, piochesCartes[i], piochesCartes[i].size() - 1);
                    });
                }
            }
        }
        for (int j = 0; j < 3 + i; j++) {
            auto layout = dynamic_cast<QGridLayout*>(gridLayout2->itemAtPosition(i+1, (j+1) * 2)->widget()->layout());
            if (layout) {
                // apparence de la carte
                auto carte = piochesCartes[i][j];
                auto carteWidget = new CarteWidget(carte);
                auto boutonReserver = dynamic_cast<QPushButton*>(layout->itemAtPosition(1, 0)->widget());
                auto boutonAcheter = dynamic_cast<QPushButton*>(layout->itemAtPosition(2, 0)->widget());
                if (carte) {
                    layout->addWidget(carteWidget->getImage(), 0, 0);
                    if (boutonAcheter) {
                        boutonAcheter->disconnect();
                        connect(boutonAcheter, &QPushButton::clicked, [=]() {
                            acheterCarte(carteWidget, piochesCartes[i], j);
                        });
                    } else qDebug() << "acheter incorrect";
                    if (boutonReserver) {
                        boutonReserver->disconnect();
                        connect(boutonReserver, &QPushButton::clicked, [=]() {
                            reserverCarte(carteWidget, piochesCartes[i], j);
                        });
                    } else qDebug() << "reserver incorrect";
                } else {
                    string pathCartes = "cartes_nv";
                    pathCartes.append(to_string(3 - i));
                    layout->addWidget(carteWidget->initUI(partie::getBasePath() / "assets" / pathCartes / "dos.png"));
                    boutonAcheter->setDisabled(true);
                    boutonReserver->setDisabled(true);
                }
            } else qDebug("layout not good");
        }
    }
}

void SplendorDuel::updateJetonsJoueur() {
    QLabel* jetonsLabel;
    if (labelsJoueurs.contains("Jetons :")) {
        jetonsLabel = labelsJoueurs["Jetons :"];
    } else {
        jetonsLabel = new QLabel;
        labelsJoueurs.insert("Jetons :", jetonsLabel);

        QBoxLayout *layoutBas = dynamic_cast<QBoxLayout *>(mainLayout->itemAtPosition(2, 0));
        if (layoutBas) {
            layoutBas->addWidget(jetonsLabel);
        } else {
            layoutBas = new QVBoxLayout;
            layoutBas->addWidget(jetonsLabel);
            mainLayout->addLayout(layoutBas, 2, 0);
        }
    }
    string jetonsString = "Jetons :\n";
    for (const auto [couleur, nb]: (mypartie->auTourDe->getJetons())) {
        jetonsString.append(jeton::toString(couleur));
        jetonsString.append(": ");
        jetonsString.append(to_string(nb));
        jetonsString.append("  ");
    }
    jetonsString.append("\nBonus :\n");
    for (const auto [couleur, nb]: (mypartie->auTourDe->getBonus())) {
        jetonsString.append(jeton::toString(couleur));
        jetonsString.append(": ");
        jetonsString.append(to_string(nb));
        jetonsString.append("  ");
    }
    jetonsString.append("\nTotem(s) Privilège(s) : ");
    auto nbPrivi = mypartie->auTourDe->getNbPrivileges();
    if (nbPrivi > 0 && mypartie->actionsOptionnellesAccomplies < 2)
        dynamic_cast<QPushButton*>(dynamic_cast<QGridLayout*>(mainLayout->itemAtPosition(0, 0)->layout())->itemAtPosition(0, 1)->widget())->setEnabled(true);
    else if (mypartie->actionsOptionnellesAccomplies >= 2){
        dynamic_cast<QPushButton *>(dynamic_cast<QGridLayout *>(mainLayout->itemAtPosition(0,
                                                                                           0)->layout())->itemAtPosition(
                0, 1)->widget())->setDisabled(true);
        dynamic_cast<QPushButton *>(dynamic_cast<QGridLayout *>(mainLayout->itemAtPosition(0,
                                                                                           0)->layout())->itemAtPosition(
                0, 0)->widget())->setDisabled(true);
    }
    jetonsString.append(to_string(nbPrivi));
    jetonsLabel->setText(QString::fromStdString(jetonsString));
}

void JoueurWidget::initialiserPointsCouronne() {
    pointsCouronne["Point de Prestige total"] = 0;
    pointsCouronne["Couronne"] = 0;
}

void SplendorDuel::boutonClique(int row, int col, QPushButton *button) {
    bool troisEnAttente = true;
    int indexLibre = -1;
    auto maxJetons = 3;
    if (jetonBonus)
        maxJetons = 1;
    for (int i = 0; i < maxJetons; i++) {
        auto resButton = dynamic_cast<QPushButton*>((jetonsEnAttente->itemAtPosition(0, i))->widget());
        if (resButton) {
            if (!resButton->isEnabled()) {
                troisEnAttente = false;
                indexLibre = i;
                break;
            }
        }
    }
    if (!troisEnAttente) {
        dynamic_cast<QPushButton*>((jetonsEnAttente->itemAtPosition(0, 3))->widget())->setEnabled(true);
        auto resButton = dynamic_cast<QPushButton*>((jetonsEnAttente->itemAtPosition(0, indexLibre))->widget());
        resButton->setText(button->text());
        resButton->setEnabled(true);
        coordsJetonsEnAttente.emplace_back(row, col);
        connect(resButton, &QPushButton::clicked, [=]() {
            resButton->disconnect();
            replacerJeton(resButton, button);
            coordsJetonsEnAttente.erase(
                    std::remove(coordsJetonsEnAttente.begin(), coordsJetonsEnAttente.end(), tuple(row, col)),
                    coordsJetonsEnAttente.end());
            bool aucunEnAttente = true;
            for (int i = 0; i < 3; i++) {
                auto tempButton = dynamic_cast<QPushButton*>((jetonsEnAttente->itemAtPosition(0, i))->widget());
                if (tempButton) {
                    if (tempButton->isEnabled()) {
                        aucunEnAttente = false;
                        break;
                    }
                }
            }
            if (aucunEnAttente) {
                dynamic_cast<QPushButton*>((jetonsEnAttente->itemAtPosition(0, 3))->widget())->setEnabled(false);
            }
        });
        button->setText("#");
        button->setDisabled(true);
//        mypartie->prendreJetons(row, col);
    }
}

void SplendorDuel::replacerJeton(QPushButton* boutonRes, QPushButton* boutonPlateau) {
    boutonPlateau->setEnabled(true);
    boutonPlateau->setText(boutonRes->text());
    boutonRes->setText("#");
    boutonRes->setDisabled(true);
}

void SplendorDuel::acheterJetonsEnAttente() {
    if (!partie::verifProxi(coordsJetonsEnAttente) || ! partie::verifColineaire(coordsJetonsEnAttente)) {
        QMessageBox::warning(this, "Action invalide", "Votre sélection de jetons est invalide !", QMessageBox::Ok);
        return;
    }
    map<jeton::Couleur, int> nbCouleur;
    for (const auto [x, y] : coordsJetonsEnAttente) {
        nbCouleur[mypartie->plateau.getJetons()[mypartie->plateau.getPosition(x, y)]->getCouleur()] += 1;
    }
    if (jetonBonus && couleurAPrendre != jeton::Couleur::perle && nbCouleur[couleurAPrendre] != 1) {
        string errString = "Le jeton n'est pas de la bonne couleur (";
        errString.append(toString(couleurAPrendre));
        errString.append(") !");
        QMessageBox::warning(this, "Action invalide", QString::fromStdString(errString), QMessageBox::Ok);
        return;
    }
    if (partie::verifNbCouleurs(nbCouleur)) {
        QMessageBox::information(this,
                             "Information",
                             "Vous avez pris 3 jetons de la meme couleur ou les 2 jetons Perle avec cette action, votre adversaire prend donc 1 Privilege.\n",
                             QMessageBox::Ok);
        mypartie->changeJoueur();
        mypartie->prendreUnPrivilege();
        mypartie->changeJoueur();
    }
    bool gold = false;
    for (const auto coords: coordsJetonsEnAttente) {
        auto [x, y] = coords;
        if (mypartie->plateau.getJetons()[mypartie->plateau.getPosition(x, y)]->getCouleur() == jeton::Couleur::gold)
            gold = true;
        mypartie->prendreJetons(x, y);
    }
    for (int i = 0; i < 3; i++)
        dynamic_cast<QPushButton*>((jetonsEnAttente->itemAtPosition(0, i))->widget())->disconnect();
    coordsJetonsEnAttente.clear();
    for (int i = 0; i < 3; i++) {
        auto tempButton = dynamic_cast<QPushButton*>((jetonsEnAttente->itemAtPosition(0, i))->widget());
        if (tempButton) {
            tempButton->setText("#");
            tempButton->setDisabled(true);
        }
    }
    if (!jetonBonus)
        changementJoueur();
    else {
        if (couleurAPrendre == jeton::Couleur::perle && !gold)
            mypartie->auTourDe->retirerPrivilege();
        dynamic_cast<QDialog*>(layoutJetons->parent()->parent())->close();
        updateJetonsJoueur();
        mypartie->actionsOptionnellesAccomplies += 1;
    }
}

void SplendorDuel::piocheClique() {
    if (mypartie->sachet.getNbJetons() > 0) {
        mypartie->plateau.remplirPlateau(mypartie->sachet);
        initPlateau();
        layoutJetons->addLayout(plateauJetons, 1, 0);
    }

}

void SplendorDuel::reserverCarte(CarteWidget *c, const vector<carte::CarteJoaillerie*>& container, int index) {
    if (compteur_action_obligatoire==0){
        // Voir avec peut réserver
        if (mypartie->peutReserver()) {
            compteur_action_obligatoire=1;

            QGridLayout *miniLayout = new QGridLayout;
            miniLayout->addWidget(c->getImage(), 0,0);
            QPushButton* boutonAcheter = new QPushButton("Acheter");
            boutonAcheter->setFixedSize(50, 20);
            miniLayout->addWidget(boutonAcheter, 2,0);
            QWidget* widgetContainer = new QWidget;
            widgetContainer->setLayout(miniLayout);
            auTourDe_->getGridLayoutCartesReservees()->addWidget(widgetContainer, auTourDe_->getGridLayoutCartesReservees()->rowCount(), 0);
            mypartie->recupererCarte(container, index, false);
            connect(boutonAcheter, &QPushButton::clicked, [=]() {
                acheterCarte(c, mypartie->auTourDe->getReserve(), mypartie->auTourDe->getReserve().size() - 1);
                widgetContainer->deleteLater();
            });
            jetonBonus = true;
            prendreUnJeton("Prendre un unique jeton Or", true);
            layoutJetons->setParent(nullptr);
            mainLayout->addLayout(layoutJetons, 1, 0);
            initPlateau();
            jetonBonus = false;
            updateJetonsJoueur();
            updatePyramide();
            changementJoueur();
        } else {
            QMessageBox::warning(this, "Impossible de réserver", "Vous ne pouvez pas réserver actuellement.");
        }}
    else{
        QMessageBox::warning(this, "Limite d'actions obligatoires atteinte", "Vous avez déjà effectué une action obligatoire.");
    }
}

void SplendorDuel::acheterCarte(CarteWidget *c, const vector<carte::CarteJoaillerie*>& container, int index) {
    //if(mypartie->peutAcheter(c->getCarte()))
    if (compteur_action_obligatoire==0){
        if (mypartie->peutAcheter(c->getCarte())) {
            if (c->getCarte()->getCouleurReduc() == jeton::Couleur::perle)
                choisirCouleurBonus(container, index);
            mypartie->acheterCarteJoaillerie(container, index);
            auTourDe_->setPointsCouronne("Point de Prestige total",
                                         auTourDe_->getPointsCouronne()["Point de Prestige total"] +
                                         c->getCarte()->getPrestige());
            auTourDe_->setPointsCouronne("Couronne",
                                         auTourDe_->getPointsCouronne()["Couronne"] + c->getCarte()->getCouronnes());
            auTourDe_->setPointsCouronne("Reduction",
                                         auTourDe_->getPointsCouronne()["Reduction"] + c->getCarte()->getNbReduc());
            labelPrestige->setText("Point de Prestige total: " +
                                   QString::number(auTourDe_->getPointsCouronne()["Point de Prestige total"]));
            labelCouronne->setText("Couronne: " + QString::number(auTourDe_->getPointsCouronne()["Couronne"]));
            labelReduction->setText("Reduction: " + QString::number(auTourDe_->getPointsCouronne()["Reduction"]));
            auTourDe_->getGridLayoutCartesAchetees()->addWidget(c->getImage(),
                                                                auTourDe_->getGridLayoutCartesAchetees()->rowCount(),
                                                                0);
            updatePyramide();
            if (!appelCapaciteCarte(c)) {
                updateJetonsJoueur();
                compteur_action_obligatoire = 1;
                changementJoueur();
            }
            else
                updateJetonsJoueur();
                mypartie->actionsOptionnellesAccomplies = 0;
        } else {
            QMessageBox::warning(this, "Achat impossible", "Il est impossible d'acheter cette carte Joaillerie", QMessageBox::Ok);
        }

    }else{
        QMessageBox::warning(this, "Limite d'actions obligatoires atteinte", "Vous avez déjà effectué une action obligatoire.", QMessageBox::Ok);
    }

}

bool SplendorDuel::appelCapaciteCarte(CarteWidget* carte) {
    if (carte->getCarte()->getCapacite() == carte::Capacite::jeton_adversaire) {
        mypartie->jetonAdversaire();
        int i = 0;
        if (mypartie->auTourDe == &mypartie->joueurs[0]) {
            i = 1;
        }
        if (mypartie->joueurs[i].getJetons().empty())
            return false;
        string text = "Quelle jeton voulez-vous prendre à l'adversaire ? ";
        vector<QString> choixValides;
        for (const auto [couleur, nbJetons]: mypartie->joueurs[i].getJetons()) {
            if (nbJetons != 0 && couleur != jeton::Couleur::gold) {
                text.append(toString(couleur));
                text.append("  ");
                choixValides.push_back(QString::fromStdString(toString(couleur)));
            }
        }
        const jeton::Couleur couleurChoisie = choixParmiCouleurs(choixValides, "Subtiliser un jeton", text);
        mypartie->auTourDe->prendreJeton(couleurChoisie);
        mypartie->joueurs[i].enleverJeton(couleurChoisie);
        return false;
    }
    if (carte->getCarte()->getCapacite() == carte::Capacite::prendre_un_jeton) {
        jetonBonus = true;
        couleurAPrendre = carte->getCarte()->getCouleurReduc();
        string title = "Prendre un jeton ";
        title.append(toString(couleurAPrendre));
        prendreUnJeton(title, false);
        couleurAPrendre = jeton::Couleur::perle;
        layoutJetons->setParent(nullptr);
        mainLayout->addLayout(layoutJetons, 1, 0);
        initPlateau();
        jetonBonus = false;
        return false;
    }
    if (carte->getCarte()->getCapacite() == carte::Capacite::prendre_un_privilege) {
        mypartie->prendreUnPrivilege();
        return false;
    }
    if (carte->getCarte()->getCapacite() == carte::Capacite::rejouer) {
        return true;
    }
    return false;
}

void SplendorDuel::choisirCouleurBonus(const vector<carte::CarteJoaillerie *> &container, const size_t index) {
    string text = "Choisissez la couleur du bonus de votre carte : ";
    vector<QString> choixValides;
    for (const auto couleur: jeton::Couleurs) {
        if (couleur != jeton::Couleur::gold && couleur != jeton::Couleur::perle) {
            text.append(toString(couleur));
            text.append("  ");
            choixValides.push_back(QString::fromStdString(toString(couleur)));
        }
    }
    const jeton::Couleur couleurChoisie = choixParmiCouleurs(choixValides, "Bonus libre", text);
    container[index]->setCouleurReduc(couleurChoisie);
}

jeton::Couleur SplendorDuel::choixParmiCouleurs(const vector<QString>& choixValides, const string& title, const string& text) {
    QString rep;
    bool ok = false;
    while (!ok || rep.isEmpty() || std::find(choixValides.cbegin(), choixValides.cend(), rep) == choixValides.cend())
        rep = QInputDialog::getText(nullptr, QString::fromStdString(title), QString::fromStdString(text), QLineEdit::Normal, "", &ok);
    jeton::Couleur couleurPrise;
    if (rep == "B") couleurPrise = jeton::Couleur::bleu;
    else if (rep == "W") couleurPrise = jeton::Couleur::blanc;
    else if (rep == "V") couleurPrise = jeton::Couleur::vert;
    else if (rep == "N") couleurPrise = jeton::Couleur::noir;
    else if (rep == "R") couleurPrise = jeton::Couleur::rouge;
    else if (rep == "P") couleurPrise = jeton::Couleur::perle;
    else throw jeton::SplendorException("Couleur impossible a choisir");
    return couleurPrise;
}

void SplendorDuel::prendreUnJeton(const string& title, bool gold) {
    auto window = new QDialog;
    window->setWindowTitle(QString::fromStdString(title));
    auto layout = new QGridLayout;
    initPlateau(gold);
    layoutJetons->setParent(nullptr);
    layout->addLayout(layoutJetons, 0, 0);
    window->setLayout(layout);
    window->exec();
}

