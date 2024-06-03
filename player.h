#pragma once

#include <utility>

#include "string"
#include "map"
#include "vector"
#include "token.h"
#include "card.h"

using namespace std;

namespace player {
    class Player {
        string name;
        bool ai;
        int AILevel;
        map<token::Color, size_t> tokens;
        map<token::Color, size_t> bonus;
        map<token::Color, size_t> prestigeByColor;
        vector<card::CardJewels *> cardsJewels;
        vector<card::CardRoyals *> cardsRoyals;
        vector<card::CardJewels *> cardsReserve;
        size_t nbPrivileges = 0;
        size_t nbPrestiges = 0;
        size_t nbCrowns = 0;
        size_t nbTokens = 0;

    public:
        Player(string p, const bool i, int n = 0): name(std::move(p)), ai(i), AILevel(n) {
        }

        ~Player() = default;

        string getName() const { return name; }
        bool isAI() const { return ai; }
        map<token::Color, size_t> getTokens() const { return tokens; }
        map<token::Color, size_t> getBonus() const { return bonus; }
        vector<card::CardJewels *> getCardsJewels() const { return cardsJewels; }
        vector<card::CardRoyals *> getCardsRoyals() const { return cardsRoyals; }
        vector<card::CardJewels *> getReserve() const { return cardsReserve; }
        size_t getNbPrivileges() const { return nbPrivileges; }
        size_t getNbCrowns() const { return nbCrowns; }
        size_t getNbPrestiges() const { return nbPrestiges; }
        size_t getNbTokens() const { return nbTokens; }
        int getAILevel() const { return AILevel; }

        void privilegeRemove() { nbPrivileges--; }
        void privilegeAdd() { nbPrivileges++; }
        void crownRemove(const size_t nb) { nbCrowns -= nb; }
        void crownAdd(const size_t nb) { nbCrowns += nb; }
        void prestigeRemove(const size_t nb) { nbPrestiges -= nb; }
        void prestigeAdd(const size_t nb) { nbPrestiges += nb; }

        bool hasWonByColorPrestige();

        void takeCardRoyals(card::CardRoyals* c);

        void takeCardJewels(card::CardJewels* c);

        void reserveCardJewels(card::CardJewels* c);

        void removeFromReserve(const card::CardJewels* c);

        void takeToken(const token::Color c) {
            tokens[c] += 1;
            nbTokens++;
        }

        void giveToken(const token::Color c) {
            tokens[c] -= 1;
            nbTokens--;
        }

        void saveState(const QString& dir) const;

        static Player loadState(const QString& dir);
    };
}
