#pragma once

#include "vector"
#include "random"
#include "tuple"
#include "player.h"
#include "board.h"
#include "card.h"

using namespace std;

class SplendorDuel;

namespace game {
    filesystem::path getBasePath();

    void printSpacing(size_t, bool);

    void printCardsLine(const vector<card::CardJewels *> &, size_t, bool);

    class Game {
        vector <player::Player> players;
        player::Player *activePlayer;
        size_t optionalActionsDone = 0;
        board::Board board;
        board::Bag bag;
        size_t nbFreePrivileges = 3;
        vector<card::CardJewels *> cardsLvl1;
        vector<card::CardJewels *> cardsLvl2;
        vector<card::CardJewels *> cardsLvl3;
        vector<card::CardRoyals *> cardsRoyals;
        bool gameEnded = false;

        friend class ::SplendorDuel;

    public:
        Game(const player::Player &player1, const player::Player &player2, const bool newGame) {
            players.push_back(player1);
            players.push_back(player2);
            if (newGame) {
                random_device dev;
                mt19937 rng(dev());
                uniform_int_distribution<mt19937::result_type> dist2(0, 1);
                const auto result = dist2(rng);
                activePlayer = &players[result];
                board.fillBoard(bag);
                generateJewels();
                generateRoyals();
            }
        }

        vector<card::CardJewels *> getCardsLvl1() const { return cardsLvl1; }

        vector<card::CardJewels *> getCardsLvl2() const { return cardsLvl2; }

        vector<card::CardJewels *> getCardsLvl3() const { return cardsLvl3; }

        player::Player *getActivePlayer() const { return activePlayer; }

        vector <player::Player> getPlayers() { return players; }


        bool hasEnded() const { return gameEnded; }

        void generateJewels(const string & = "../data/cards.data");

        void generateRoyals(const string & = "../data/royal_cards.data");

        void printPyramid() const {
            printCardsLine(cardsLvl3, 3, true);
            printCardsLine(cardsLvl2, 4, true);
            printCardsLine(cardsLvl1, 5, true);
        }

        static void printRoyals(const vector<card::CardRoyals *> &cardsList) ;

        void play();

        tuple<bool, bool> actionOptional();

        tuple<bool, bool> actionOptionalAI();

        bool actionMandatory();

        bool actionMandatoryAI();

        vector <tuple<size_t, size_t>> chooseTokens(bool, bool);

        vector <tuple<size_t, size_t>> chooseTokensAI(bool, bool, bool, token::Color);

        tuple<vector < card::CardJewels * >, size_t> chooseCardJewels(bool);

        tuple<vector < card::CardJewels * >, size_t> chooseCardJewelsAI(bool);

        size_t chooseCardRoyals() const;

        void usePrivilege(const token::Token &);

        void takeToken(size_t, size_t);

        bool canBuyJewel(const card::CardJewels *) const;

        bool canReserveJewel() const;

        void takeRoyal(size_t);

        void takeJewel(const vector<card::CardJewels *> &, size_t, bool);

        void buyJewel(const vector<card::CardJewels *> &, size_t);

        bool endOfTurnChecks();

        bool winCheck() const;

        void takePrivilege();

        void changePlayer();

        void discardExcessTokens(size_t nb);

        void discardExcessTokensAI(size_t nb);

        void cardAbilityActivation(const vector<card::CardJewels *> &container, size_t index);

        void takeOppToken();

        void takeOppTokenAI();

        void takeTokenFromBoardAbility(token::Color token);

        void saveState(const QString &dir = (getBasePath() / "save").string().c_str()) const {
            QFile file(dir + "/game.xml");
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QXmlStreamWriter xmlWriter(&file);
                xmlWriter.setAutoFormatting(true);

                xmlWriter.writeStartDocument();
                xmlWriter.writeStartElement("game");

                xmlWriter.writeAttribute("nbOptionalActions", QString::number(optionalActionsDone));
                xmlWriter.writeAttribute("nbPrivileges", QString::number(nbFreePrivileges));
                int i = 0;
                if (activePlayer != &players[0])
                    i = 1;
                xmlWriter.writeAttribute("activePlayer", QString::number(i));

                xmlWriter.writeEndElement();
                xmlWriter.writeEndDocument();

                file.close();
                if (QFile fileRoy(dir + "/royals.xml"); fileRoy.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    QXmlStreamWriter xmlWriter2(&fileRoy);
                    xmlWriter2.setAutoFormatting(true);

                    xmlWriter2.writeStartDocument();
                    xmlWriter2.writeStartElement("deck");

                    for (const auto card: cardsRoyals) {
                        if (card == nullptr) {
                            xmlWriter2.writeStartElement("card");
                            xmlWriter2.writeAttribute("valid", nullptr);
                            xmlWriter2.writeEndElement();
                        }
                        // else card->saveState(xmlWriter);
                    }

                    xmlWriter2.writeEndElement();
                    xmlWriter2.writeEndDocument();

                    file.close();
                } else {
                    qDebug() << "Couldn't open the file to save the deck.";
                }
                players[0].saveState(dir + "/player1");
                players[1].saveState(dir + "/player2");
                board.saveState(dir + "/board.xml");
                bag.saveState(dir + "/bag.xml");
                saveJewelsStack(cardsLvl1, dir + "/deckLvl1.xml");
                saveJewelsStack(cardsLvl2, dir + "/deckLvl2.xml");
                saveJewelsStack(cardsLvl3, dir + "/deckLvl3.xml");
                saveRoyalsStack(cardsRoyals, dir + "/royals.xml");
            } else {
                qDebug() << "Couldn't open the file to save the game state.";
            }
        }

        static Game *loadState(const QString &dir = (getBasePath() / "save").string().c_str()) {
            player::Player player1 = player::Player::loadState(dir + "/player1");
            player::Player player2 = player::Player::loadState(dir + "/player2");
            auto newGame = new Game(player1, player2, false);
            QFile file(dir + "/game.xml");
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QXmlStreamReader xmlReader(&file);

                while (!xmlReader.atEnd() && !xmlReader.hasError()) {
                    const QXmlStreamReader::TokenType token = xmlReader.readNext();

                    if (token == QXmlStreamReader::StartDocument) {
                        continue;
                    }
                    if (token == QXmlStreamReader::StartElement && xmlReader.name().toString() == "game") {
                        newGame->optionalActionsDone = xmlReader.attributes().value("nbOptionalActions").toInt();
                        newGame->nbFreePrivileges = xmlReader.attributes().value("nbPrivileges").toInt();
                        newGame->activePlayer = &newGame->players[xmlReader.attributes().value(
                                "activePlayer").toInt()];

                        xmlReader.skipCurrentElement();
                    }
                }
                file.close();
                newGame->cardsLvl1 = card::loadJewelsStack(dir + "/deckLvl1.xml");
                newGame->cardsLvl2 = card::loadJewelsStack(dir + "/deckLvl2.xml");
                newGame->cardsLvl3 = card::loadJewelsStack(dir + "/deckLvl3.xml");
                newGame->cardsRoyals = card::loadRoyalsStack(dir + "/royals.xml");
                newGame->board.loadState(dir + "/board.xml");
                newGame->bag.loadState(dir + "/bag.xml");
                return newGame;
            }
            qDebug() << "Couldn't open the file to load the game state.";
            throw token::SplendorException("Loading failed");
        }
    };

    double distance(const tuple<size_t, size_t> &, const tuple<size_t, size_t> &);

    bool checkProximity(const vector<tuple<size_t, size_t>> &choice);

    bool checkCollinearity(const vector<tuple<size_t, size_t>> &choice);

    bool checkNbColors(const map<token::Color, int>& tokens);

}
