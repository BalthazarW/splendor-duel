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

class CardWidget : public QLabel {
Q_OBJECT

public:
    explicit CardWidget(card::CardJewels *card, QWidget *parent = nullptr)
            : QLabel(parent), pCard(card) {
        if (card)
            pImage = initUI(card->getFilePath());
    }

    ~CardWidget() override {
        delete pCard;
    }

    static QLabel *initUI(const filesystem::path &path);

    card::CardJewels *getCard() { return pCard; }

    QLabel *getImage() { return pImage; };

private:
    card::CardJewels *pCard;
    QLabel *pImage;

};


class PlayerWidget : public QLabel {
Q_OBJECT

private:
    player::Player *pPlayer;
    QMap<QString, int> crownPoints;
    QDialog *windowBoughtCards;
    QDialog *windowReservedCards;
    QGridLayout *gridLayoutBoughtCards;
    QGridLayout *gridLayoutReservedCards;
public:
    explicit PlayerWidget(player::Player *player, QWidget *parent = nullptr)
            : QLabel(parent), pPlayer(player) {
        initCrownPoints();

        windowBoughtCards = new QDialog(this);
        windowBoughtCards->setWindowTitle("Cartes Achetées");

        auto *layoutVertical = new QVBoxLayout(windowBoughtCards);
        gridLayoutBoughtCards = new QGridLayout;
        layoutVertical->addLayout(gridLayoutBoughtCards);
        windowReservedCards = new QDialog(this);
        windowReservedCards->setWindowTitle("Cartes Reservées");
        auto *layout = new QVBoxLayout(windowReservedCards);
        gridLayoutReservedCards = new QGridLayout;
        layout->addLayout(gridLayoutReservedCards);
    }

    ~PlayerWidget() override {
        delete pPlayer;
    }

    const QMap<QString, int> &getCrownPoints() const {
        return crownPoints;
    }

    void setCrownPoints(const QString &key, int value) {
        crownPoints[key] = value;
    }

    QDialog *getWindowBoughtCards() const {
        return windowBoughtCards;
    }

    QDialog *getWindowReservedCards() const {
        return windowReservedCards;
    }

    QGridLayout *getGridLayoutBoughtCards() const {
        return gridLayoutBoughtCards;
    }

    QGridLayout *getGridLayoutReservedCards() const {
        return gridLayoutReservedCards;
    }

    void initCrownPoints();
};


class SplendorDuel : public QWidget {
Q_OBJECT

private:
    QGridLayout *mainLayout;
    QGridLayout *layoutTokens;
    QGridLayout *pendingTokens;
    vector<tuple<size_t, size_t>> coordsPendingTokens;
    bool bonusToken = false;
    token::Color colorToTake = token::Color::pearl;
    QGridLayout *boardTokens;
    QGridLayout *gridLayout2;
    QGridLayout *gridLayout3;
    game::Game *pGame = nullptr;
    QMap<QString, QLabel *> labelsPlayers;
    size_t countMandatoryActions;
    QLabel *labelPrestige;
    QLabel *labelCrowns;
    QLabel *labelDiscount;
    PlayerWidget *activePlayer;
    vector<PlayerWidget *> players;
    QDialog *windowRoyalCards;
    QGridLayout *gridLayoutRoyalsCards;
private slots:

    void showWindowBoughtCards();

    void showWindowReservedCards();

    void showWindowRoyalsCards();

public:
    explicit SplendorDuel(QWidget *parent = nullptr);

    void updatePlayerTokens();

    void clickTokenButton(int row, int col, QPushButton *clickedButton);

    void reserveCard(CardWidget *widgetCard, const vector<card::CardJewels *> &container, int index);

    void buyCard(CardWidget *widgetCard, const vector<card::CardJewels *> &container, int index);

    bool activateCardAbility(CardWidget *card);

    token::Color chooseBetweenColors(const vector<QString> &validChoices, const string &title, const string &text);

    void chooseBonusColor(const vector<card::CardJewels *> &container, size_t index);

    void clickDeck();

    QGridLayout *createGridPlayerCards();

    QGridLayout *createGridRoyalsCards();

    void initBoard(bool gold = false);

    void initPendingTokens();

    void setupUI();

    void switchPlayers();

    void checkEndOfTurn();

    bool checkWin();

    static void putBackToken(QPushButton *buttonSelected, QPushButton *buttonBoard);

    void takePendingTokens();

    void createPyramid();

    void updatePyramid();

    void takeOneToken(const string &title, bool gold);

    void startAI();

    ~SplendorDuel() override {
        if (pGame) {
            if (!pGame->gameEnded)
                pGame->saveState();
        }
    }
};

