#include "board.h"
#include <random>
#include <ctime>
#include <algorithm>

namespace board {
    void Board::fillBoard(Bag &bag) {
        size_t nbTokensInBag = bag.getNbTokens();
        for (size_t i = 0; i < nbMax && nbTokensInBag != 0; i++) {
            if (tokens[i] == nullptr) {
                tokens[i] = bag.drawTokenFromBag();
                nbTokensInBag--;
                nbTokens++;
                if (tokens[i]->isGold()) nbGoldTokens++;
            }
        }
    }

    void Board::removeToken(const token::Token &token) {
        size_t i = 0;
        while (i < nbMax && tokens[i] != &token) i++;
        if (i == nbMax) throw token::SplendorException("No token of this color on the board");
        if (token.isGold()) nbGoldTokens--;
        tokens[i] = nullptr;
        nbTokens--;
    }

    void Board::displayBoard() const {
        for (size_t i = 0; i < 5; i++) {
            cout << i + 1 << "  ";
            for (size_t j = 0; j < 5; j++) {
                if (tokens[position[i][j]] == nullptr) {
                    cout << "# ";
                } else {
                    cout << *tokens[position[i][j]] << " ";
                }
            }
            cout << "\n";
        }
        cout << "   ";
        for (size_t i = 1; i <= 5; i++)
            cout << i << " ";
        cout << "\n";
    }

    Bag::Bag() {
        nb = 25;
        for (size_t i = 0; i < 5; i++) {
            for (size_t j = 0; j < 4; j++) {
                tokens.push_back(new const token::Token(static_cast<token::Color>(i)));
            }
        }
        for (size_t i = 0; i < 2; i++) {
            tokens.push_back(new const token::Token(token::Color::gold));
            tokens.push_back(new const token::Token(token::Color::pearl));
        }
        tokens.push_back(new const token::Token(token::Color::gold));
        shuffleBag();
    }

    Bag::~Bag() {
        for (size_t i = 0; i < 25; i++) {
            delete tokens[i];
        }
    }

    void Bag::shuffleBag() {
        random_device randomDevice;
        mt19937 rng(randomDevice());
        shuffle(tokens.begin(), tokens.end(), rng);
    }

    const token::Token *Bag::drawTokenFromBag() {
        if (nb == 0) throw token::SplendorException("Not enough tokens to draw");
        return tokens[--nb];
    }

    void Bag::returnTokenToBag(const token::Color &color) {
        if (nb == 25) throw token::SplendorException("The bag is already full");
        size_t i = nb;
        while (i < 25 && tokens[i]->getColor() != color) i++;
        if (i == 25) throw token::SplendorException("Too many tokens of this color in the bag");
        const token::Token *tmp = tokens[i];
        tokens[i] = tokens[nb];
        tokens[nb] = tmp;
        nb++;
    }
}
