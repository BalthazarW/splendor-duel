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

QLabel *CardWidget::initUI(const filesystem::path &fileName) {
    auto *labelCard = new QLabel;

    QPixmap tmpImage(QString::fromStdString(fileName.string()));

    labelCard->setMaximumSize(72, 120);

    QSize imageSize = tmpImage.size();
    imageSize.scale(labelCard->maximumSize(), Qt::KeepAspectRatio);

    QPixmap scaledImage = tmpImage.scaled(imageSize, Qt::KeepAspectRatio);
    labelCard->setPixmap(scaledImage);
    labelCard->setScaledContents(true);
    return labelCard;
}

SplendorDuel::SplendorDuel(QWidget *parent) : QWidget(parent), countMandatoryActions(0) {
    setWindowTitle("Splendor duel"); // TODO: handle quit before launching app
    int continueGame = 0;
    if (const filesystem::path memoryPath = game::getBasePath() / "save" / "game.xml";
            splendor_duel::memoryFileExists(memoryPath))
        continueGame = QMessageBox::question(this, "Game en mémoire", "Une game est en cours, continuer ?",
                                             QMessageBox::Yes | QMessageBox::No);
    PlayerWidget *widgetPlayer1;
    PlayerWidget *widgetPlayer2;
    if (continueGame == QMessageBox::Yes) {
        pGame = game::Game::loadState();
        widgetPlayer1 = new PlayerWidget(&pGame->players[0]);
        widgetPlayer2 = new PlayerWidget(&pGame->players[1]);
    } else {
        bool ok;

        auto player1AI = QMessageBox::question(nullptr, "Status Player 1", "Le Player 1 sera-t-il une IA ?",
                                               QMessageBox::Yes | QMessageBox::No);
        int lvlPlayer1AI = 0;
        QString namePlayer1 = "Bot Alpha";
        if (player1AI == QMessageBox::Yes) {
            lvlPlayer1AI = QInputDialog::getInt(nullptr, "Niveau d'IA", "Niveau de l'IA:\n1 (aleatoire)", 1, 1, 1, 1,
                                                &ok);
        } else {
            namePlayer1 = QInputDialog::getText(nullptr, "Saisir Player 1", "Entrer le nom du Player 1:",
                                                QLineEdit::Normal, "", &ok);

            if (ok && !namePlayer1.isEmpty()) {
                qDebug() << "Nom saisi: " << namePlayer1;
            } else {
                qDebug() << "Saisie annulée";
            }
        }
        player::Player player1(namePlayer1.toStdString(), bool(lvlPlayer1AI), lvlPlayer1AI);
        widgetPlayer1 = new PlayerWidget(&player1);

        auto player2AI = QMessageBox::question(nullptr, "Status Player 2", "Le Player 2 sera-t-il une IA ?",
                                               QMessageBox::Yes | QMessageBox::No);
        int lvlPlayer2AI = 0;
        QString namePlayer2 = "Bot Beta";
        if (player2AI == QMessageBox::Yes) {
            lvlPlayer2AI = QInputDialog::getInt(nullptr, "Niveau d'IA", "Niveau de l'IA:\n1 (aleatoire)", 1, 1, 1, 1,
                                                &ok);
        } else {
            namePlayer2 = QInputDialog::getText(nullptr, "Saisir Player 2", "Entrer le nom du Player 2:",
                                                QLineEdit::Normal, "", &ok);

            if (ok && !namePlayer2.isEmpty()) {
                qDebug() << "Nom saisi: " << namePlayer2;
            } else {
                qDebug() << "Saisie annulée";
            }
        }
        player::Player player2(namePlayer2.toStdString(), bool(lvlPlayer2AI), lvlPlayer2AI);
        widgetPlayer2 = new PlayerWidget(&player2);
        pGame = new game::Game(player1, player2, true);
    }
    setupUI();
    players.push_back(widgetPlayer1);
    players.push_back(widgetPlayer2);

    if (pGame->getActivePlayer() == &pGame->getPlayers()[0]) {
        activePlayer = players[0];
    } else {
        activePlayer = players[1];
    }
}

void SplendorDuel::startAI() {
    bool continueGame = true;
    bool quitGame = false;
    size_t k = 0;
    while (pGame->optionalActionsDone < 2 && continueGame) {
        k++;
        tie(continueGame, quitGame) = pGame->actionOptionalAI();
        if (quitGame && !continueGame)
            return;
        if (!quitGame || !continueGame)
            pGame->optionalActionsDone++;
    }
    continueGame = false;
    continueGame = pGame->actionMandatoryAI();
    if (!continueGame) {
        return; // pour ne pas changer 2 fois de Player
    }
    pGame->optionalActionsDone = 0;
    pGame->endOfTurnChecks();
    initBoard();
    updatePlayerTokens();
    updatePyramid();
}

void SplendorDuel::setupUI() {
    mainLayout = new QGridLayout(this);
    layoutTokens = new QGridLayout;
    initBoard();
    initPendingTokens();
    layoutTokens->addLayout(pendingTokens, 0, 0);
    mainLayout->addLayout(layoutTokens, 1, 0);
    auto layoutButtons = new QGridLayout;
    auto *buttonDeck = new QPushButton("Fill board");
//    buttonDeck->setFixedSize(120, 30);
    connect(buttonDeck, &QPushButton::clicked, this, &SplendorDuel::clickDeck);
    layoutButtons->addWidget(buttonDeck, 0, 0);
    auto *buttonPrivilege = new QPushButton("Use Privilege");
//    buttonDeck->setFixedSize(120, 30);
    connect(buttonPrivilege, &QPushButton::clicked, this, [=]() {
        bonusToken = true;
        takeOneToken("Take a single token", false);
        layoutTokens->setParent(nullptr);
        mainLayout->addLayout(layoutTokens, 1, 0);
        initBoard();
        bonusToken = false;
    });
    buttonPrivilege->setDisabled(true);
    layoutButtons->addWidget(buttonPrivilege, 0, 1);
    mainLayout->addLayout(layoutButtons, 0, 0);
    auto *buttonStartAI = new QPushButton("Start AIs");
//    buttonDeck->setFixedSize(120, 30);
    auto gamePlayers = pGame->players;
    if (std::all_of(gamePlayers.begin(), gamePlayers.end(),
                    [](auto player) { return player.isAI(); })) {
        connect(buttonStartAI, &QPushButton::clicked, this, [=]() {
            bool hasWon = false;
            while (!hasWon) {
                startAI();
                hasWon = pGame->endOfTurnChecks();
                if (!hasWon)
                    switchPlayers();
            }
            pGame->gameEnded = true;
            string winText = "Player ";
            winText.append(pGame->activePlayer->getName());
            winText.append(" has won ! Well played !");
            QMessageBox::information(this, "Fin de la game", QString::fromStdString(winText), QMessageBox::Ok);
        });
        mainLayout->addWidget(buttonStartAI, 0, 1);
    } else if (pGame->activePlayer->isAI()) {
        connect(buttonStartAI, &QPushButton::clicked, this, [=]() {
            startAI();
            switchPlayers();
            buttonStartAI->setDisabled(true);
        });
        mainLayout->addWidget(buttonStartAI, 0, 1);
    }

    string playerName = "Player:\n";
    playerName.append(pGame->activePlayer->getName());
    auto labelName = new QLabel(QString::fromStdString(playerName));
//    labelName->setFixedSize(120, 30);
    mainLayout->addWidget(labelName, 0, 3);

    createPyramid();
    mainLayout->addLayout(gridLayout2, 1, 1);

    gridLayout3 = createGridPlayerCards();
    mainLayout->addLayout(gridLayout3, 2, 3);

    auto *buttonReservedCards = new QPushButton("Voir Cartes Reservées");
    buttonReservedCards->setFixedSize(120, 30);
    mainLayout->addWidget(buttonReservedCards, 2, 1);
    connect(buttonReservedCards, &QPushButton::clicked, this, &SplendorDuel::showWindowReservedCards);

    updatePlayerTokens();

    windowRoyalCards = new QDialog(this);
    windowRoyalCards->setWindowTitle("Cartes Royales");

    auto *layoutVertical = new QVBoxLayout(windowRoyalCards);
    gridLayoutRoyalsCards = new QGridLayout;
    gridLayoutRoyalsCards = createGridRoyalsCards();
    layoutVertical->addLayout(gridLayoutRoyalsCards);
    auto *buttonRoyals = new QPushButton("Voir Cartes Royales");
    buttonRoyals->setFixedSize(120, 30);
    mainLayout->addWidget(buttonRoyals, 1, 2);
    connect(buttonRoyals, &QPushButton::clicked, this, &SplendorDuel::showWindowRoyalsCards);
}

QGridLayout *SplendorDuel::createGridRoyalsCards() {
    auto *grid = new QGridLayout;
    for (int i = 0; i < 4; i++) {
        auto *labelRoyalCard = new QLabel;
        auto *miniLayout = new QGridLayout;
        QPixmap tmpImage(QString::fromStdString(pGame->cardsRoyals[i]->getFilePath().string()));
        labelRoyalCard->setMaximumSize(72, 120);
        QSize imageSize = tmpImage.size();
        imageSize.scale(labelRoyalCard->maximumSize(), Qt::KeepAspectRatio);
        QPixmap scaledImage = tmpImage.scaled(imageSize, Qt::KeepAspectRatio);
        labelRoyalCard->setPixmap(scaledImage);
        labelRoyalCard->setScaledContents(true);
        miniLayout->addWidget(labelRoyalCard, 0, 0);
        auto *buttonTake = new QPushButton("Take");
        buttonTake->setFixedSize(50, 20);
        connect(buttonTake, &QPushButton::clicked, this, [=]() {
            if (pGame->activePlayer->getNbCrowns() >= 3 && pGame->activePlayer->getCardsRoyals().size() < 2) {
                pGame->takeRoyal(i);
                pGame->activePlayer->crownRemove(3);
                buttonTake->setDisabled(true);
            } else {
                QMessageBox::information(nullptr, "Achat impossible", "Impossible de récupérer une card Royale",
                                         QMessageBox::Ok);
            }
        });
        miniLayout->addWidget(buttonTake, 1, 0);
        auto *widgetContainer = new QWidget;
        widgetContainer->setLayout(miniLayout);
        grid->addWidget(widgetContainer, 0, i, 1, 1);

    }
    return grid;
}


void SplendorDuel::checkEndOfTurn() {
    while (pGame->activePlayer->getNbTokens() > 10) {
        string text = "Choisissez la color du token à reposer (";
        text.append(to_string(pGame->activePlayer->getNbTokens() - 10));
        text.append(" restant(s)): ");
        vector<QString> validChoices;
        for (const auto color: token::Colors) {
            text.append(toString(color));
            text.append("  ");
            validChoices.push_back(QString::fromStdString(toString(color)));
        }
        const token::Color chosenColor = chooseBetweenColors(validChoices, "Bonus libre", text);
        pGame->activePlayer->giveToken(chosenColor);
        pGame->bag.returnTokenToBag(chosenColor);
        updatePlayerTokens();
    }
}

bool SplendorDuel::checkWin() {
    return pGame->activePlayer->hasWonByColorPrestige() || pGame->activePlayer->getNbCrowns() >= 10 ||
           pGame->activePlayer->getNbPrestiges() >= 20;
}

void SplendorDuel::switchPlayers() {
    checkEndOfTurn();
    if (checkWin()) {
        string winText = "Le Player ";
        winText.append(pGame->activePlayer->getName());
        winText.append(" a gagné ! Bien joué !");
        QMessageBox::information(this, "Fin de la game", QString::fromStdString(winText), QMessageBox::Ok);
    }
    pGame->changePlayer();
    countMandatoryActions = 0;
    if (activePlayer == players[0]) {
        activePlayer = players[1];
    } else {
        activePlayer = players[0];
    }
    QMessageBox::information(this, "Changement de Player", "Veuillez laisser la place à l'autre Player.",
                             QMessageBox::Ok);
    auto labelName = dynamic_cast<QLabel *>(mainLayout->itemAtPosition(0, 3)->widget());
    if (labelName) {
        string playerName = "Player:\n";
        playerName.append(pGame->activePlayer->getName());
        labelName->setText(QString::fromStdString(playerName));
    }
    labelPrestige->setText(
            "Point de Prestige total: " + QString::number(activePlayer->getCrownPoints()["Point de Prestige total"]));
    labelCrowns->setText("Couronne: " + QString::number(activePlayer->getCrownPoints()["Couronne"]));
    labelDiscount->setText("Couronne: " + QString::number(activePlayer->getCrownPoints()["Reduction"]));
    updatePlayerTokens();
    if (pGame->activePlayer->isAI()) {
        startAI();
        switchPlayers();
    }
    //if (pGame->winCheck())
}

QGridLayout *SplendorDuel::createGridPlayerCards() {
    auto *gridLayout = new QGridLayout;
    labelPrestige = new QLabel("Point de Prestige total: 0");
    labelCrowns = new QLabel("Couronne: 0");
    labelDiscount = new QLabel("Reduction: 0");
    gridLayout->addWidget(labelPrestige, 0, 0);
    gridLayout->addWidget(labelCrowns, 1, 0);
    gridLayout->addWidget(labelDiscount, 2, 0);

    auto *buttonBoughtCards = new QPushButton("Voir Cartes Achetées");
    buttonBoughtCards->setFixedSize(120, 30);
    gridLayout->addWidget(buttonBoughtCards, 2, 0);
    connect(buttonBoughtCards, &QPushButton::clicked, this, &SplendorDuel::showWindowBoughtCards);
    return gridLayout;
}

void SplendorDuel::showWindowRoyalsCards() {
    windowRoyalCards->exec();
}


void SplendorDuel::showWindowBoughtCards() {
    activePlayer->getWindowBoughtCards()->exec();
}

void SplendorDuel::showWindowReservedCards() {
    activePlayer->getWindowReservedCards()->exec();
}

void SplendorDuel::initPendingTokens() {
    pendingTokens = new QGridLayout;
    for (int col = 0; col < 3; col++) {
        auto *buttonPendingToken = new QPushButton;
        buttonPendingToken->setText("#");
        buttonPendingToken->setDisabled(true);
        buttonPendingToken->setFixedSize(30, 30);
        buttonPendingToken->setContentsMargins(0, 0, 0, 0);

        pendingTokens->addWidget(buttonPendingToken, 0, col);
    }
    auto buttonTakeSelectedTokens = new QPushButton;
    buttonTakeSelectedTokens->setText("Take selected token(s)");
    buttonTakeSelectedTokens->setDisabled(true);
    buttonTakeSelectedTokens->setContentsMargins(0, 0, 0, 0);
    connect(buttonTakeSelectedTokens, &QPushButton::clicked, [=]() {
        takePendingTokens();
    });
    pendingTokens->addWidget(buttonTakeSelectedTokens, 0, 3);
}

void SplendorDuel::initBoard(bool gold) {
    boardTokens = new QGridLayout;
    for (int row = 0; row < 5; ++row) {
        for (int col = 0; col < 5; ++col) {
            auto *buttonBoardToken = new QPushButton;
            const token::Token *token = pGame->board.getTokens()[pGame->board.getPosition(row, col)];
            if (token != nullptr) {
                auto color = token->getColor();
                buttonBoardToken->setText(QString::fromStdString(toString(color)));
                if ((color == token::Color::gold && !gold) || (color != token::Color::gold && gold))
                    buttonBoardToken->setDisabled(true);
            } else {
                buttonBoardToken->setText("#");
                buttonBoardToken->setDisabled(true);
            }

            buttonBoardToken->setFixedSize(30, 30);
            buttonBoardToken->setContentsMargins(0, 0, 0, 0);

            connect(buttonBoardToken, &QPushButton::clicked, [=]() {
                clickTokenButton(row, col, buttonBoardToken);
            });

            boardTokens->addWidget(buttonBoardToken, row, col);
            boardTokens->setColumnStretch(col, 0);
        }
        boardTokens->setRowStretch(row, 0);
    }
    layoutTokens->addLayout(boardTokens, 1, 0);
}

void SplendorDuel::createPyramid() {
    gridLayout2 = new QGridLayout;
    if (!pGame->getCardsLvl3().empty()) {
        auto *layoutCard = new QGridLayout;
        auto *widgetCard = new CardWidget(pGame->getCardsLvl3()[3]);
        QLabel *label = CardWidget::initUI(game::getBasePath() / "assets" / "cards_lvl3" / "back.png");
        layoutCard->addWidget(label, 0, 0);
        auto *buttonReserve = new QPushButton("Reserve");
        buttonReserve->setFixedSize(50, 20);
        connect(buttonReserve, &QPushButton::clicked, [=]() {
            reserveCard(widgetCard, pGame->cardsLvl3, pGame->cardsLvl3.size() - 1);
        });
        layoutCard->addWidget(buttonReserve, 1, 0);
        auto *widgetContainer = new QWidget;
        widgetContainer->setLayout(layoutCard);
        gridLayout2->addWidget(widgetContainer, 1, 0, 1, 2);
    }
    if (!pGame->getCardsLvl2().empty()) {
        auto *layoutCard = new QGridLayout;
        auto *widgetCard = new CardWidget(pGame->getCardsLvl2()[4]);
        QLabel *label = CardWidget::initUI(game::getBasePath() / "assets" / "cards_lvl2" / "back.png");
        layoutCard->addWidget(label, 0, 0);
// Ajout du bouton "Reserve"
        auto *buttonReserve = new QPushButton("Reserve");
        buttonReserve->setFixedSize(50, 20);
        connect(buttonReserve, &QPushButton::clicked, [=]() {
            reserveCard(widgetCard, pGame->cardsLvl2, pGame->cardsLvl2.size() - 1);
        });
        layoutCard->addWidget(buttonReserve, 1, 0);

        auto *widgetContainer = new QWidget;
        widgetContainer->setLayout(layoutCard);
        gridLayout2->addWidget(widgetContainer, 2, 0, 1, 2);
    }
    if (0 < pGame->getCardsLvl1().size()) {
        auto *layoutCard = new QGridLayout;
        auto *widgetCard = new CardWidget(pGame->getCardsLvl1()[5]);
        QLabel *label = CardWidget::initUI(game::getBasePath() / "assets" / "cards_lvl1" / "back.png");
        layoutCard->addWidget(label, 0, 0);

        auto *buttonReserve = new QPushButton("Reserve");
        buttonReserve->setFixedSize(50, 20);

        connect(buttonReserve, &QPushButton::clicked, [=]() {
            reserveCard(widgetCard, pGame->cardsLvl1, pGame->cardsLvl1.size() - 1);
        });
        layoutCard->addWidget(buttonReserve, 1, 0);

        auto *widgetContainer = new QWidget;
        widgetContainer->setLayout(layoutCard);
        gridLayout2->addWidget(widgetContainer, 3, 0, 1, 2);
    }

    for (int i = 1; i < 4; ++i) {
        for (int j = 1; j < i + 3; ++j) {

            if (i == 1) {
                if (j <= pGame->getCardsLvl3().size()) {
                    auto *layoutCard = new QGridLayout;
                    auto *widgetCard = new CardWidget(pGame->getCardsLvl3()[j - 1]);
                    layoutCard->addWidget(widgetCard->getImage(), 0, 0);
                    auto *buttonReserve = new QPushButton("Reserve");
                    buttonReserve->setFixedSize(50, 20);
                    connect(buttonReserve, &QPushButton::clicked, [=]() {
                        reserveCard(widgetCard, pGame->cardsLvl3, j - 1);
                    });
                    layoutCard->addWidget(buttonReserve, 1, 0);

                    auto *buttonBuy = new QPushButton("Buy");
                    // connect(buttonBuy, &QPushButton::clicked, this, &CardWidget::buyCard);
                    buttonBuy->setFixedSize(50, 20);
                    connect(buttonBuy, &QPushButton::clicked, [=]() {
                        buyCard(widgetCard, pGame->cardsLvl3, j - 1);
                    });
                    layoutCard->addWidget(buttonBuy, 2, 0);
                    auto *widgetContainer = new QWidget;
                    widgetContainer->setLayout(layoutCard);
                    gridLayout2->addWidget(widgetContainer, i, j * 2, 1, 2);

                }
            } else if (i == 2) {
                if (j <= pGame->getCardsLvl2().size()) {
                    auto *layoutCard = new QGridLayout;
                    auto *widgetCard = new CardWidget(pGame->getCardsLvl2()[j - 1]);
                    layoutCard->addWidget(widgetCard->getImage(), 0, 0);
                    // Ajout du bouton "Reserve"
                    auto *buttonReserve = new QPushButton("Reserve");
                    buttonReserve->setFixedSize(50, 20);
                    buttonReserve->setFixedSize(50, 20);
                    connect(buttonReserve, &QPushButton::clicked, [=]() {
                        reserveCard(widgetCard, pGame->cardsLvl2, j - 1);
                    });
                    layoutCard->addWidget(buttonReserve, 1, 0);

                    // Ajout du bouton "Buy"
                    auto *buttonBuy = new QPushButton("Buy");
                    connect(buttonBuy, &QPushButton::clicked, [=]() {
                        buyCard(widgetCard, pGame->cardsLvl2, j - 1);
                    });
                    buttonBuy->setFixedSize(50, 20);
                    layoutCard->addWidget(buttonBuy, 2, 0);
                    auto *widgetContainer = new QWidget;
                    widgetContainer->setLayout(layoutCard);
                    gridLayout2->addWidget(widgetContainer, i, j * 2, 1, 2);
                }
            } else if (i == 3) {
                if (j <= pGame->getCardsLvl1().size()) {
                    auto *layoutCard = new QGridLayout;
                    auto *widgetCard = new CardWidget(pGame->getCardsLvl1()[j - 1]);
                    layoutCard->addWidget(widgetCard->getImage(), 0, 0);

                    auto *buttonReserve = new QPushButton("Reserve");
                    buttonReserve->setFixedSize(50, 20);
                    buttonReserve->setFixedSize(50, 20);
                    connect(buttonReserve, &QPushButton::clicked, [=]() {
                        reserveCard(widgetCard, pGame->cardsLvl1, j - 1);
                    });
                    layoutCard->addWidget(buttonReserve, 1, 0);

                    // Ajout du bouton "Buy"
                    auto *buttonBuy = new QPushButton("Buy");

                    buttonBuy->setFixedSize(50, 20);
                    layoutCard->addWidget(buttonBuy, 2, 0);
                    connect(buttonBuy, &QPushButton::clicked, [=]() {
                        buyCard(widgetCard, pGame->cardsLvl1, j - 1);
                    });
                    auto *widgetContainer = new QWidget;
                    widgetContainer->setLayout(layoutCard);
                    gridLayout2->addWidget(widgetContainer, i, j * 2, 1, 2);
                }
            }
        }
    }
}

void SplendorDuel::updatePyramid() {
    auto cardsDecks = vector<vector<card::CardJewels *>>{pGame->cardsLvl3, pGame->cardsLvl2, pGame->cardsLvl1};
    for (int i = 0; i < 3; i++) {
        auto layoutDeck = dynamic_cast<QGridLayout *>(gridLayout2->itemAtPosition(i + 1, 0)->widget()->layout());
        if (layoutDeck) {
            auto buttonReserveDeck = dynamic_cast<QPushButton *>(layoutDeck->itemAtPosition(1, 0)->widget());
            if (buttonReserveDeck) {
                if (cardsDecks[i].size() <= 3 + i)
                    buttonReserveDeck->setDisabled(true);
                else {
                    auto *widgetCard = new CardWidget(cardsDecks[i][cardsDecks[i].size() - 1]);
                    connect(buttonReserveDeck, &QPushButton::clicked, [=]() {
                        reserveCard(widgetCard, cardsDecks[i], cardsDecks[i].size() - 1);
                    });
                }
            }
        }
        for (int j = 0; j < 3 + i; j++) {
            auto layoutCard = dynamic_cast<QGridLayout *>(gridLayout2->itemAtPosition(i + 1,
                                                                                  (j + 1) * 2)->widget()->layout());
            if (layoutCard) {
                // apparence de la card
                auto card = cardsDecks[i][j];
                auto widgetCard = new CardWidget(card);
                auto buttonReserve = dynamic_cast<QPushButton *>(layoutCard->itemAtPosition(1, 0)->widget());
                auto buttonBuy = dynamic_cast<QPushButton *>(layoutCard->itemAtPosition(2, 0)->widget());
                if (card) {
                    layoutCard->addWidget(widgetCard->getImage(), 0, 0);
                    if (buttonBuy) {
                        buttonBuy->disconnect();
                        connect(buttonBuy, &QPushButton::clicked, [=]() {
                            buyCard(widgetCard, cardsDecks[i], j);
                        });
                    } else
                        qDebug() << "acheter incorrect";
                    if (buttonReserve) {
                        buttonReserve->disconnect();
                        connect(buttonReserve, &QPushButton::clicked, [=]() {
                            reserveCard(widgetCard, cardsDecks[i], j);
                        });
                    } else
                        qDebug() << "reserver incorrect";
                } else {
                    string pathCards = "cards_lvl";
                    pathCards.append(to_string(3 - i));
                    layoutCard->addWidget(widgetCard->initUI(game::getBasePath() / "assets" / pathCards / "back.png"));
                    buttonBuy->setDisabled(true);
                    buttonReserve->setDisabled(true);
                }
            } else
                qDebug("layoutCard not good");
        }
    }
}

void SplendorDuel::updatePlayerTokens() {
    QLabel *labelTokens;
    if (labelsPlayers.contains("Jetons:")) {
        labelTokens = labelsPlayers["Jetons:"];
    } else {
        labelTokens = new QLabel;
        labelsPlayers.insert("Jetons:", labelTokens);

        auto *layoutBottom = dynamic_cast<QBoxLayout *>(mainLayout->itemAtPosition(2, 0));
        if (layoutBottom) {
            layoutBottom->addWidget(labelTokens);
        } else {
            layoutBottom = new QVBoxLayout;
            layoutBottom->addWidget(labelTokens);
            mainLayout->addLayout(layoutBottom, 2, 0);
        }
    }
    string stringTokens = "Jetons:\n";
    for (const auto [color, nb]: (pGame->activePlayer->getTokens())) {
        stringTokens.append(token::toString(color));
        stringTokens.append(": ");
        stringTokens.append(to_string(nb));
        stringTokens.append("  ");
    }
    stringTokens.append("\nBonus:\n");
    for (const auto [color, nbDiscount]: (pGame->activePlayer->getBonus())) {
        stringTokens.append(token::toString(color));
        stringTokens.append(": ");
        stringTokens.append(to_string(nbDiscount));
        stringTokens.append("  ");
    }
    stringTokens.append("\nTotem(s) Privilège(s): ");
    auto nbPrivileges = pGame->activePlayer->getNbPrivileges();
    if (nbPrivileges > 0 && pGame->optionalActionsDone < 2)
        dynamic_cast<QPushButton *>(dynamic_cast<QGridLayout *>(mainLayout->itemAtPosition(0,
                                                                                           0)->layout())->itemAtPosition(
                0, 1)->widget())->setEnabled(true);
    else if (pGame->optionalActionsDone >= 2) {
        dynamic_cast<QPushButton *>(dynamic_cast<QGridLayout *>(mainLayout->itemAtPosition(0,
                                                                                           0)->layout())->itemAtPosition(
                0, 1)->widget())->setDisabled(true);
        dynamic_cast<QPushButton *>(dynamic_cast<QGridLayout *>(mainLayout->itemAtPosition(0,
                                                                                           0)->layout())->itemAtPosition(
                0, 0)->widget())->setDisabled(true);
    }
    stringTokens.append(to_string(nbPrivileges));
    labelTokens->setText(QString::fromStdString(stringTokens));
}

void PlayerWidget::initCrownPoints() {
    crownPoints["Point de Prestige total"] = 0;
    crownPoints["Couronne"] = 0;
}

void SplendorDuel::clickTokenButton(int row, int col, QPushButton *clickedButton) {
    bool threeSelectedTokens = true;
    int freeIndex = -1;
    auto maxTokens = 3;
    if (bonusToken)
        maxTokens = 1;
    for (int i = 0; i < maxTokens; i++) {
        auto buttonSelectedToken = dynamic_cast<QPushButton *>((pendingTokens->itemAtPosition(0, i))->widget());
        if (buttonSelectedToken) {
            if (!buttonSelectedToken->isEnabled()) {
                threeSelectedTokens = false;
                freeIndex = i;
                break;
            }
        }
    }
    if (!threeSelectedTokens) {
        dynamic_cast<QPushButton *>((pendingTokens->itemAtPosition(0, 3))->widget())->setEnabled(true);
        auto buttonSelectedToken = dynamic_cast<QPushButton *>((pendingTokens->itemAtPosition(0, freeIndex))->widget());
        buttonSelectedToken->setText(clickedButton->text());
        buttonSelectedToken->setEnabled(true);
        coordsPendingTokens.emplace_back(row, col);
        connect(buttonSelectedToken, &QPushButton::clicked, [=]() {
            buttonSelectedToken->disconnect();
            putBackToken(buttonSelectedToken, clickedButton);
            coordsPendingTokens.erase(
                    std::remove(coordsPendingTokens.begin(), coordsPendingTokens.end(), tuple(row, col)),
                    coordsPendingTokens.end());
            bool noSelection = true;
            for (int i = 0; i < 3; i++) {
                auto tempButton = dynamic_cast<QPushButton *>((pendingTokens->itemAtPosition(0, i))->widget());
                if (tempButton) {
                    if (tempButton->isEnabled()) {
                        noSelection = false;
                        break;
                    }
                }
            }
            if (noSelection) {
                dynamic_cast<QPushButton *>((pendingTokens->itemAtPosition(0, 3))->widget())->setEnabled(false);
            }
        });
        clickedButton->setText("#");
        clickedButton->setDisabled(true);
//        pGame->takeToken(row, col);
    }
}

void SplendorDuel::putBackToken(QPushButton *buttonSelected, QPushButton *buttonBoard) {
    buttonBoard->setEnabled(true);
    buttonBoard->setText(buttonSelected->text());
    buttonSelected->setText("#");
    buttonSelected->setDisabled(true);
}

void SplendorDuel::takePendingTokens() {
    if (!game::checkProximity(coordsPendingTokens) || !game::checkCollinearity(coordsPendingTokens)) {
        QMessageBox::warning(this, "Action invalide", "Votre sélection de tokens est invalide !", QMessageBox::Ok);
        return;
    }
    map<token::Color, int> nbColors;
    for (const auto [x, y]: coordsPendingTokens) {
        nbColors[pGame->board.getTokens()[pGame->board.getPosition(x, y)]->getColor()] += 1;
    }
    if (bonusToken && colorToTake != token::Color::pearl && nbColors[colorToTake] != 1) {
        string errString = "Le token n'est pas de la bonne color (";
        errString.append(toString(colorToTake));
        errString.append(") !");
        QMessageBox::warning(this, "Action invalide", QString::fromStdString(errString), QMessageBox::Ok);
        return;
    }
    if (game::checkNbColors(nbColors)) {
        QMessageBox::information(this,
                                 "Information",
                                 "Vous avez pris 3 tokens de la meme color ou les 2 tokens Perle avec cette action, votre adversaire prend donc 1 Privilege.\n",
                                 QMessageBox::Ok);
        pGame->changePlayer();
        pGame->takePrivilege();
        pGame->changePlayer();
    }
    bool gold = false;
    for (const auto coords: coordsPendingTokens) {
        auto [x, y] = coords;
        if (pGame->board.getTokens()[pGame->board.getPosition(x, y)]->getColor() == token::Color::gold)
            gold = true;
        pGame->takeToken(x, y);
    }
    for (int i = 0; i < 3; i++)
        dynamic_cast<QPushButton *>((pendingTokens->itemAtPosition(0, i))->widget())->disconnect();
    coordsPendingTokens.clear();
    for (int i = 0; i < 3; i++) {
        auto tempButton = dynamic_cast<QPushButton *>((pendingTokens->itemAtPosition(0, i))->widget());
        if (tempButton) {
            tempButton->setText("#");
            tempButton->setDisabled(true);
        }
    }
    if (!bonusToken)
        switchPlayers();
    else {
        if (colorToTake == token::Color::pearl && !gold)
            pGame->activePlayer->privilegeRemove();
        dynamic_cast<QDialog *>(layoutTokens->parent()->parent())->close();
        updatePlayerTokens();
        pGame->optionalActionsDone += 1;
    }
}

void SplendorDuel::clickDeck() {
    if (pGame->bag.getNbTokens() > 0) {
        pGame->board.fillBoard(pGame->bag);
        initBoard();
        layoutTokens->addLayout(boardTokens, 1, 0);
    }

}

void SplendorDuel::reserveCard(CardWidget *widgetCard, const vector<card::CardJewels *> &container, int index) {
    if (countMandatoryActions == 0) {
        // Voir avec peut réserver
        if (pGame->canReserveJewel()) {
            countMandatoryActions = 1;

            auto *layoutCard = new QGridLayout;
            layoutCard->addWidget(widgetCard->getImage(), 0, 0);
            auto *buttonBuy = new QPushButton("Buy");
            buttonBuy->setFixedSize(50, 20);
            layoutCard->addWidget(buttonBuy, 2, 0);
            auto *widgetContainer = new QWidget;
            widgetContainer->setLayout(layoutCard);
            activePlayer->getGridLayoutReservedCards()->addWidget(widgetContainer,
                                                                  activePlayer->getGridLayoutReservedCards()->rowCount(),
                                                                  0);
            pGame->takeJewel(container, index, false);
            connect(buttonBuy, &QPushButton::clicked, [=]() {
                buyCard(widgetCard, pGame->activePlayer->getReserve(), pGame->activePlayer->getReserve().size() - 1);
                widgetContainer->deleteLater();
            });
            bonusToken = true;
            takeOneToken("Prendre un unique token Or", true);
            layoutTokens->setParent(nullptr);
            mainLayout->addLayout(layoutTokens, 1, 0);
            initBoard();
            bonusToken = false;
            updatePlayerTokens();
            updatePyramid();
            switchPlayers();
        } else {
            QMessageBox::warning(this, "Impossible de réserver", "Vous ne pouvez pas réserver actuellement.");
        }
    } else {
        QMessageBox::warning(this, "Limite d'actions obligatoires atteinte",
                             "Vous avez déjà effectué une action obligatoire.");
    }
}

void SplendorDuel::buyCard(CardWidget *widgetCard, const vector<card::CardJewels *> &container, int index) {
    //if(pGame->canBuyJewel(widgetCard->getCard()))
    if (countMandatoryActions == 0) {
        if (pGame->canBuyJewel(widgetCard->getCard())) {
            if (widgetCard->getCard()->getDiscountColor() == token::Color::pearl)
                chooseBonusColor(container, index);
            pGame->buyJewel(container, index);
            activePlayer->setCrownPoints("Point de Prestige total",
                                         activePlayer->getCrownPoints()["Point de Prestige total"] +
                                         widgetCard->getCard()->getPrestige());
            activePlayer->setCrownPoints("Couronne",
                                         activePlayer->getCrownPoints()["Couronne"] + widgetCard->getCard()->getCrowns());
            activePlayer->setCrownPoints("Reduction",
                                         activePlayer->getCrownPoints()["Reduction"] + widgetCard->getCard()->getNbDiscount());
            labelPrestige->setText("Point de Prestige total: " +
                                   QString::number(activePlayer->getCrownPoints()["Point de Prestige total"]));
            labelCrowns->setText("Couronne: " + QString::number(activePlayer->getCrownPoints()["Couronne"]));
            labelDiscount->setText("Reduction: " + QString::number(activePlayer->getCrownPoints()["Reduction"]));
            activePlayer->getGridLayoutBoughtCards()->addWidget(widgetCard->getImage(),
                                                                activePlayer->getGridLayoutBoughtCards()->rowCount(),
                                                                0);
            updatePyramid();
            if (!activateCardAbility(widgetCard)) {
                updatePlayerTokens();
                countMandatoryActions = 1;
                switchPlayers();
            } else
                updatePlayerTokens();
            pGame->optionalActionsDone = 0;
        } else {
            QMessageBox::warning(this, "Achat impossible", "Il est impossible d'acheter cette card Joaillerie",
                                 QMessageBox::Ok);
        }

    } else {
        QMessageBox::warning(this, "Limite d'actions obligatoires atteinte",
                             "Vous avez déjà effectué une action obligatoire.", QMessageBox::Ok);
    }

}

bool SplendorDuel::activateCardAbility(CardWidget *card) {
    if (card->getCard()->getAbility() == card::Ability::take_opp_token) {
        pGame->takeOppToken();
        int i = 0;
        if (pGame->activePlayer == &pGame->players[0]) {
            i = 1;
        }
        if (pGame->players[i].getTokens().empty())
            return false;
        string text = "Quelle token voulez-vous prendre à l'adversaire ? ";
        vector<QString> validChoices;
        for (const auto [color, nbTokens]: pGame->players[i].getTokens()) {
            if (nbTokens != 0 && color != token::Color::gold) {
                text.append(toString(color));
                text.append("  ");
                validChoices.push_back(QString::fromStdString(toString(color)));
            }
        }
        const token::Color chosenColor = chooseBetweenColors(validChoices, "Subtiliser un token", text);
        pGame->activePlayer->takeToken(chosenColor);
        pGame->players[i].giveToken(chosenColor);
        return false;
    }
    if (card->getCard()->getAbility() == card::Ability::take_a_token) {
        bonusToken = true;
        colorToTake = card->getCard()->getDiscountColor();
        string title = "Prendre un token ";
        title.append(toString(colorToTake));
        takeOneToken(title, false);
        colorToTake = token::Color::pearl;
        layoutTokens->setParent(nullptr);
        mainLayout->addLayout(layoutTokens, 1, 0);
        initBoard();
        bonusToken = false;
        return false;
    }
    if (card->getCard()->getAbility() == card::Ability::take_a_privilege) {
        pGame->takePrivilege();
        return false;
    }
    if (card->getCard()->getAbility() == card::Ability::play_again) {
        return true;
    }
    return false;
}

void SplendorDuel::chooseBonusColor(const vector<card::CardJewels *> &container, size_t index) {
    string text = "Choisissez la color du bonus de votre card: ";
    vector<QString> validChoices;
    for (const auto color: token::Colors) {
        if (color != token::Color::gold && color != token::Color::pearl) {
            text.append(toString(color));
            text.append("  ");
            validChoices.push_back(QString::fromStdString(toString(color)));
        }
    }
    const token::Color chosenColor = chooseBetweenColors(validChoices, "Bonus libre", text);
    container[index]->setDiscountColor(chosenColor);
}

token::Color SplendorDuel::chooseBetweenColors(const vector<QString> &validChoices, const string &title, const string &text) {
    QString answer;
    bool ok = false;
    while (!ok || answer.isEmpty() || std::find(validChoices.cbegin(), validChoices.cend(), answer) == validChoices.cend())
        answer = QInputDialog::getText(nullptr, QString::fromStdString(title), QString::fromStdString(text),
                                       QLineEdit::Normal, "", &ok);
    token::Color chosenColor;
    if (answer == "B") chosenColor = token::Color::blue;
    else if (answer == "W") chosenColor = token::Color::white;
    else if (answer == "V") chosenColor = token::Color::green;
    else if (answer == "N") chosenColor = token::Color::black;
    else if (answer == "R") chosenColor = token::Color::red;
    else if (answer == "P") chosenColor = token::Color::pearl;
    else throw token::SplendorException("Couleur impossible a choisir");
    return chosenColor;
}

void SplendorDuel::takeOneToken(const string &title, bool gold) {
    auto window = new QDialog;
    window->setWindowTitle(QString::fromStdString(title));
    auto layout = new QGridLayout;
    initBoard(gold);
    layoutTokens->setParent(nullptr);
    layout->addLayout(layoutTokens, 0, 0);
    window->setLayout(layout);
    window->exec();
}

