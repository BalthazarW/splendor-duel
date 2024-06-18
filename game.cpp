#include <fstream>
#include <sstream>
#include <algorithm>
#include "random"
#include "game.h"
#include "token.h"

using namespace token;
using namespace card;

namespace game {
    filesystem::path getBasePath() {
        const auto assetsPath1 = filesystem::absolute("assets");
        const auto assetsPath2 = filesystem::absolute("../assets");
        const auto savePath1 = filesystem::absolute("save");
        const auto savePath2 = filesystem::absolute("../save");
        if (const vector path1 = {assetsPath1, savePath1}; all_of(
                path1.cbegin(),
                path1.cend(),
                [](const filesystem::path &path) { return exists(path); })
                )
            return assetsPath1.parent_path();
        if (const vector path2 = {assetsPath2, savePath2}; all_of(
                path2.cbegin(),
                path2.cend(),
                [](const filesystem::path &path) { return exists(path); })
                )
            return assetsPath2.parent_path();
        throw SplendorException("Couldn't find the path for the game's assets or the save path");
    }

    void printSpacing(const size_t base, const bool pyramid) {
        if (!pyramid)
            return;
        if (base == 3)
            cout << "                   ";
        else if (base == 4)
            cout << "         ";
    }

    void printCardsLine(const vector<card::CardJewels *> &cardsList, size_t base, bool pyramid) {
        const string bottomLine = "|________________|";
        printSpacing(base, pyramid);
        for (int i = 0; i < min(base, size_t(6)); i++) {
            cout << " ________________  ";
        }
        cout << "\n";
        printSpacing(base, pyramid);
        for (int i = 0; i < min(base, size_t(6)); i++) {
            if (cardsList[i] != nullptr) {
                cout << "| P:" << cardsList[i]->getPrestige() << " " << "C:" << cardsList[i]->getCrowns()
                     << "    ";
                if (cardsList[i]->getNbDiscount() != 0) {
                    cout << cardsList[i]->getNbDiscount() << "x";
                    if (cardsList[i]->getDiscountColor() != Color::pearl)
                        cout << cardsList[i]->getDiscountColor();
                    else
                        cout << "?";
                } else cout << "   ";
                cout << " | ";
            } else cout << "|                | ";
        }
        cout << "\n";
        printSpacing(base, pyramid);
        for (size_t line = 0; line < 5; line++) {
            for (int i = 0; i < min(base, size_t(6)); i++) {
                string ability;
                if (cardsList[i] != nullptr)
                    toStringShort(ability, cardsList[i]->getAbility(), line);
                else toStringShort(ability, Ability::no_card, line);
                cout << ability << " ";
            }
            cout << "\n";
            printSpacing(base, pyramid);
        }
        for (int i = 0; i < min(base, size_t(6)); i++) {
            if (cardsList[i] != nullptr && cardsList[i]->getPrice() != nullptr) {
                cout << "| ";
                size_t countPrice = 0;
                for (const auto [color, nb]: *cardsList[i]->getPrice()) {
                    cout << nb << color << " ";
                    countPrice++;
                }
                for (auto j = countPrice; j < 4; j++)
                    cout << "   ";
                cout << "   | ";
            } else cout << "|     /    \\     | ";
        }
        cout << "\n";
        printSpacing(base, pyramid);
        for (int i = 0; i < min(base, size_t(6)); i++) {
            cout << bottomLine << " ";
        }
        cout << "\n";
        if (base > 6)
            printCardsLine(vector(cardsList.begin() + 6, cardsList.end()), base - 6, pyramid);
    }

    void whichColor(const vector<CardJewels *> &container, const size_t index) {

        char answer = ' ';
        cout << "Choose the color of your bonus. Example: R for red. Reminder: N for black\n";
        cin >> answer;
        switch (answer) {
            case 'B':
                container[index]->setDiscountColor(Color::blue);
                break;
            case 'W':
                container[index]->setDiscountColor(Color::white);
                break;
            case 'G':
                container[index]->setDiscountColor(Color::green);
                break;
            case 'N':
                container[index]->setDiscountColor(Color::black);
                break;
            case 'R':
                container[index]->setDiscountColor(Color::red);
                break;
            default:
                break;
        }
    }

    void whichColorAI(const vector<CardJewels *> &container, const size_t index) {
        vector<char> validChoices = {'B', 'W', 'G', 'N', 'R'};
        random_device dev;
        mt19937 rng(dev());
        uniform_int_distribution<> dis(0, validChoices.size() - 1);
        char answer = validChoices[dis(rng)];
        switch (answer) {
            case 'B':
                container[index]->setDiscountColor(Color::blue);
                break;
            case 'W':
                container[index]->setDiscountColor(Color::white);
                break;
            case 'V':
                container[index]->setDiscountColor(Color::green);
                break;
            case 'N':
                container[index]->setDiscountColor(Color::black);
                break;
            case 'R':
                container[index]->setDiscountColor(Color::red);
                break;
            default:
                break;
        }
    }

    void Game::printRoyals(const vector<card::CardRoyals *> &cardsList) {
        const string bottomLine = "|________________|";
        const size_t base = cardsList.size();
        for (int i = 0; i < base; i++) {
            cout << " ________________  ";
        }
        cout << "\n";
        for (int i = 0; i < base; i++) {
            cout << "| P:" << cardsList[i]->getPrestige() << "            | ";
        }
        cout << "\n";
        for (size_t line = 0; line < 5; line++) {
            for (int i = 0; i < base; i++) {
                string ability;
                toStringShort(ability, cardsList[i]->getAbility(), line);
                cout << ability << " ";
            }
            cout << "\n";
        }
        for (int i = 0; i < base; i++) {
            cout << "|                | ";
        }
        cout << "\n";
        for (int i = 0; i < base; i++) {
            cout << bottomLine << " ";
        }
        cout << "\n";
    }

    void Game::generateJewels(const string &filename) {
        string line_info;
        int level, prestige, ability, crowns, color, nbDiscount, blue, white, green, black, red, pearl;
        ifstream infile(filename);
        getline(infile, line_info);
        size_t lineNb = 1;
        auto assetsPath = getBasePath() / "assets";

        istringstream line_stream;
        while (getline(infile, line_info)) {
            line_stream.str(line_info);
            line_stream >> level >> prestige >> ability >> crowns >> color >> nbDiscount >> blue >> white >> green >>
                        black >> red >> pearl;
            map<Color, size_t> price;
            int colors[] = {blue, white, green, black, red, pearl};
            for (int i = 0; i < 6; i++) {
                if (colors[i] != 0)
                    price[static_cast<Color>(i)] = colors[i];
            }
            char imageFilename[20];
            char cardLevelPath[20];
            sprintf(cardLevelPath, "cards_lvl%d", level);
            if (level == 1) {
                sprintf(imageFilename, "card-%llu.png", lineNb);
                auto imagePath = assetsPath / cardLevelPath / imageFilename;
                cardsLvl1.push_back(new CardJewels(
                        prestige,
                        new map(price),
                        static_cast<Ability>(ability),
                        crowns,
                        static_cast<Color>(color),
                        nbDiscount,
                        imagePath.string()
                ));
            } else if (level == 2) {
                sprintf(imageFilename, "card-%llu.png", lineNb - cardsLvl1.size());
                auto imagePath = assetsPath / cardLevelPath / imageFilename;
                cardsLvl2.push_back(new CardJewels(
                        prestige,
                        new map(price),
                        static_cast<Ability>(ability),
                        crowns,
                        static_cast<Color>(color),
                        nbDiscount,
                        imagePath.string()
                ));
            } else {
                sprintf(imageFilename, "card-%llu.png", lineNb - cardsLvl1.size() - cardsLvl2.size());
                auto imagePath = assetsPath / cardLevelPath / imageFilename;
                cardsLvl3.push_back(new CardJewels(
                        prestige,
                        new map(price),
                        static_cast<Ability>(ability),
                        crowns,
                        static_cast<Color>(color),
                        nbDiscount,
                        imagePath.string()
                ));
            }
            lineNb++;
            line_stream.clear();
        }
        infile.close();
        random_device dev;
        mt19937 rng(dev());
        shuffle(cardsLvl1.begin(), cardsLvl1.end(), rng);
        shuffle(cardsLvl2.begin(), cardsLvl2.end(), rng);
        shuffle(cardsLvl3.begin(), cardsLvl3.end(), rng);
    }

    void Game::generateRoyals(const string &filename) {
        string line_info;
        int prestige, ability;
        ifstream infile(filename);
        getline(infile, line_info);
        size_t lineNb = 1;
        auto assetsPath = getBasePath() / "assets";

        istringstream line_stream;
        while (getline(infile, line_info)) {
            char imageFilename[20];
            sprintf(imageFilename, "card-%llu.png", lineNb);
            auto imagePath = assetsPath / "royal_cards" / imageFilename;
            line_stream.str(line_info);
            line_stream >> prestige >> ability;
            cardsRoyals.push_back(new CardRoyals(prestige, static_cast<Ability>(ability), imagePath.string()));
            line_stream.clear();
            lineNb++;
        }
        infile.close();
    }

    void Game::play() {
        bool hasWon = false;
        while (!hasWon) {
            cout << activePlayer->getName() << "'s turn" << "\n";
            printPyramid();
            cout << "Board:\n";
            board.displayBoard();
            cout << "\nPlayer's Jewel cards:\n";
            if (!activePlayer->getCardsJewels().empty())
                printCardsLine(activePlayer->getCardsJewels(), activePlayer->getCardsJewels().size(), false);
            cout << "Player's Royal cards:\n";
            if (!activePlayer->getCardsRoyals().empty())
                printRoyals(activePlayer->getCardsRoyals());
            cout << "Player's reserve:\n";
            if (!activePlayer->getReserve().empty())
                printCardsLine(activePlayer->getReserve(), activePlayer->getReserve().size(), false);
            cout << "Player's tokens: ";
            for (const auto [color, nb]: (activePlayer->getTokens())) {
                cout << nb << color << " ";
            }
            cout << "\nPrivilege(s) held: " << activePlayer->getNbPrivileges() << "\n";

            if (!activePlayer->isAI()) {
                bool continueGame = true;
                bool quitGame = false;
                while (optionalActionsDone < 2 && continueGame) {
                    tie(continueGame, quitGame) = actionOptional();
                    if (quitGame && !continueGame)
                        return;
                    if (!quitGame || !continueGame)
                        optionalActionsDone++;
                }
                continueGame = false;
                continueGame = actionMandatory();
                if (!continueGame) {
                    return; // to not change player twice
                }
                optionalActionsDone = 0;
            } else if (activePlayer->getAILevel() == 1) {
                bool continueGame = true;
                bool quitGame = false;
                size_t k = 0;
                while (optionalActionsDone < 2 && continueGame) {
                    k++;
                    tie(continueGame, quitGame) = actionOptionalAI();
                    if (quitGame && !continueGame)
                        return;
                    if (!quitGame || !continueGame)
                        optionalActionsDone++;
                }
                continueGame = false;
                continueGame = actionMandatoryAI();
                if (!continueGame) {
                    return;
                }
                optionalActionsDone = 0;
            }
            hasWon = endOfTurnChecks();
            if (!hasWon)
                changePlayer();
            for (int i = 0; i < 10; i++)
                cout << "\n";
        }
        gameEnded = true;
        cout << "Player " << activePlayer->getName() << " has won ! Well played !";
    }

    tuple<bool, bool> Game::actionOptional() {
        int choice = 0;
        vector validChoices{0, 3};
        cout << "Possible actions:\n";
        if (activePlayer->getNbPrivileges() > 0) {
            cout << "1: Use one or multiple Privileges\n";
            validChoices.push_back(1);
        }
        if (bag.getNbTokens() > 0) {
            cout << "2: Refill the board\n";
            validChoices.push_back(2);
        }
        cout << "3: Skip to mandatory actions\n0: Quit\n";
        while (true) {
            if (cin >> choice && find(validChoices.cbegin(), validChoices.cend(), choice) != validChoices.cend())
                break;

            cout << "Invalid choice, try again:\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        if (choice == 1) {
            int res = 0;
            while (cout << "How many Privileges do you want to use? \n" && !(cin >> res) && res <= 0 &&
                   res > activePlayer->getNbPrivileges()) {
                cin.clear();
            }
            for (int i = 0; i < res; i++) {
                vector<tuple<size_t, size_t>> chosenTokens = chooseTokens(false, true);
                if (chosenTokens.empty()) {
                    return {true, true};
                }
                size_t x, y = 0;
                tie(x, y) = chosenTokens[0];
                usePrivilege(*board.getTokens()[board.getPosition(x, y)]);
            }
            return {true, false};
        }
        if (choice == 2) {
            board.fillBoard(bag);
            return {true, false};
        }
        if (choice == 0) {
            return {false, true};
        }
        return {false, false};
    }


    tuple<bool, bool> Game::actionOptionalAI() {
        size_t choice;
        vector validChoices{3};
        if (activePlayer->getNbPrivileges() > 0) {
            validChoices.push_back(1);
        }
        if (bag.getNbTokens() > 0) {
            validChoices.push_back(2);
        }
        if (find(validChoices.begin(), validChoices.end(), 2) != validChoices.end())
            choice = 2;
        else {
            random_device dev;
            mt19937 rng(dev());
            uniform_int_distribution<> dis(0, validChoices.size() - 1);
            choice = validChoices[dis(rng)];
        }
        if (choice == 1) {
            random_device dev;
            mt19937 rng(dev());
            uniform_int_distribution<> dis(0, activePlayer->getNbPrivileges());
            int res = dis(rng);
            for (int i = 0; i < res; i++) {
                vector<tuple<size_t, size_t>> chosenTokens = chooseTokensAI(false, true, false, Color::blue);
                if (chosenTokens.empty()) {
                    return {true, true};
                }
                size_t x, y = 0;
                tie(x, y) = chosenTokens[0];
                usePrivilege(*board.getTokens()[board.getPosition(x, y)]);
            }
            return {true, false};
        }
        if (choice == 2) {
            board.fillBoard(bag);
            return {true, false};
        }
        return {false, false};
    }


    bool Game::actionMandatoryAI() {
        int choice;
        {
            vector validChoices{2};
            if (board.getNbTokens() > 0 && board.getNbTokens() != board.getNbGoldTokens()) {
                validChoices.push_back(1);
            }
            if (canReserveJewel()) {
                validChoices.push_back(3);
            }
            random_device dev;
            mt19937 rng(dev());
            uniform_int_distribution<> dis(0, validChoices.size() - 1);
            choice = validChoices[dis(rng)];
        }
        bool hasChosen = false;
        if (choice == 1) {
            const auto chosenTokens = chooseTokensAI(false, false, false, Color::blue);
            for (const auto coords: chosenTokens) {
                auto [x, y] = coords;
                takeToken(x, y);
                hasChosen = true;
            }
            if (!hasChosen)
                return actionMandatoryAI();
            return hasChosen;
        }
        if (choice == 2) {
            const tuple<vector<CardJewels *>, size_t> chosenCard = chooseCardJewelsAI(true);
            if (auto [container, index] = chosenCard; index != -1) {
                hasChosen = true;
                if (container[index]->getDiscountColor() == Color::pearl)
                    whichColorAI(container, index);
                buyJewel(container, index);
                // Choose Royal card if more than 3 crowns
                if (activePlayer->getNbCrowns() >= 3 && activePlayer->getCardsRoyals().size() < 2) {
                    random_device dev;
                    mt19937 rng(dev());
                    uniform_int_distribution<> dis(0, cardsRoyals.size() - 1);
                    takeRoyal(dis(rng));
                    activePlayer->crownRemove(3);
                }
                cardAbilityActivation(container, index);
            } else
                return actionMandatoryAI();
            return hasChosen;
        }
        if (choice == 3) {
            const tuple<vector<CardJewels *>, size_t> chosenCard = chooseCardJewelsAI(false);
            if (auto [container, index] = chosenCard; index != -1) {
                const vector<tuple<size_t, size_t>> chosenTokens = chooseTokensAI(true, true, false, Color::blue);
                hasChosen = !chosenTokens.empty();
                if (!hasChosen) {
                    return actionMandatoryAI();
                }
                takeJewel(container, index, false);
                auto [x, y] = chosenTokens[0];
                takeToken(x, y);
            } else
                return actionMandatoryAI();
            return hasChosen;
        }
        return false;
    }

    bool Game::actionMandatory() {
        int choice = 0;
        {
            vector validChoices{0, 1, 2};
            cout << "Possible actions:\n";
            cout << "1: Take up to 3 tokens (gems and/or pearls)\n";
            cout << "2: Buy a Jewel card (from the pyramid or your reserve)\n";
            if (canReserveJewel()) {
                cout << "3: Reserve a Jewel card\n";
                validChoices.push_back(3);
            }
            cout << "0: Quit\n";
            while (true) {
                if (cin >> choice && choice >= 0 && choice <= 3 &&
                    find(validChoices.cbegin(), validChoices.cend(), choice) != validChoices.cend())
                    break;
                cout << "Invalid choice, try again:\n";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        }
        bool hasChosen = false;
        if (choice == 1) {
            const auto chosenTokens = chooseTokens(false, false);
            for (const auto coords: chosenTokens) {
                auto [x, y] = coords;
                takeToken(x, y);
                hasChosen = true;
            }
            if (!hasChosen)
                return actionMandatory();
            return hasChosen;
        }
        if (choice == 2) {
            const tuple<vector<CardJewels *>, size_t> chosenCard = chooseCardJewels(true);
            if (auto [container, index] = chosenCard; index != -1) {
                hasChosen = true;
                if (container[index]->getDiscountColor() == Color::pearl)
                    whichColor(container, index);
                buyJewel(container, index);
                if (activePlayer->getNbCrowns() >= 3 && activePlayer->getCardsRoyals().size() < 2) {
                    takeRoyal(chooseCardRoyals());
                    activePlayer->crownRemove(3);
                }
                cardAbilityActivation(container, index);
            } else
                return actionMandatory();
            return hasChosen;
        }
        if (choice == 3) {
            const tuple<vector<CardJewels *>, size_t> chosenCard = chooseCardJewels(false);
            if (auto [container, index] = chosenCard; index != -1) {
                const vector<tuple<size_t, size_t>> chosenTokens = chooseTokens(true, true);
                hasChosen = !chosenTokens.empty();
                if (!hasChosen)
                    return actionMandatory();
                takeJewel(container, index, false);
                auto [x, y] = chosenTokens[0];
                takeToken(x, y);
            } else
                return actionMandatory();
            return hasChosen;
        }
        return false;
    }

    vector<tuple<size_t, size_t>>
    Game::chooseTokensAI(bool canTakeGold, bool onlyOneToken, bool takeColorAbility, token::Color color) {
        vector<tuple<size_t, size_t>> choice;
        map<Color, int> nbByColor;
        int nbTokenTaken = 1;
        if (!canTakeGold) {
            nbTokenTaken = 3;
        }

        int i = 0;
        int x = -1;
        int y = -1;
        int x0 = 0;
        int y0 = 0;

        if (takeColorAbility) {
            for (size_t a = 0; a < 5; a++) {
                for (size_t b = 0; b < 5; b++) {
                    if (board.getTokens()[board.getPosition(a, b)] != nullptr
                        && board.getTokens()[board.getPosition(a, b)]->getColor() == color) {
                        choice.emplace_back(a, b);
                        return choice;
                    }
                }
            }
            return choice;
        }
        if (!canTakeGold) {
            random_device dev;
            mt19937 rng(dev());
            uniform_int_distribution<> dis(0, 4);
            int count = 0;
            do {
                x = dis(rng);
                y = dis(rng);
                count++;
                if (count == 25) return choice;
            } while (board.getTokens()[board.getPosition(x, y)] == nullptr
                     || board.getTokens()[board.getPosition(x, y)]->getColor() == Color::gold);
            x0 = x;
            y0 = y;
            choice.emplace_back(x, y);
            i++;
            if (onlyOneToken) return choice;
        }
        if (canTakeGold) {
            for (size_t a = 0; a < 5; a++) {
                for (size_t b = 0; b < 5; b++) {
                    if (board.getTokens()[board.getPosition(a, b)] != nullptr
                        && board.getTokens()[board.getPosition(a, b)]->getColor() == Color::gold) {
                        choice.emplace_back(a, b);
                        return choice;
                    }
                }
            }
        }
        for (int a = -1; a <= 1; a++) {
            for (int b = -1; b <= 1; b++) {
                if ((a != 0 || b != 0) && x0 + a >= 0 && x0 + a <= 4 && y0 + b >= 0 && y0 + b <= 4 &&
                    board.getTokens()[board.getPosition(x0 + a, y0 + b)] != nullptr
                    && board.getTokens()[board.getPosition(x0 + a, y0 + b)]->getColor() != Color::gold) {
                    choice.emplace_back(x0 + a, y0 + b);
                    if (checkCollinearity(choice) && checkProximity(choice)) {
                        i++;
                        nbByColor[board.getTokens()[board.getPosition(x0 + a, y0 + b)]->getColor()] += 1;
                        if (i == nbTokenTaken) break;
                    } else {
                        choice.pop_back();
                    }
                }
            }
            if (i == nbTokenTaken) break;
        }
        if (!onlyOneToken && checkNbColors(nbByColor)) {
            string aiName = activePlayer->getName();
            changePlayer();
            string oppName = activePlayer->getName();
            takePrivilege();
            changePlayer();
            cout << aiName << " has taken 3 tokens of the same color or 2 Pearl tokens, "
            << oppName << " receives 1 Privilege.\n";
        }
        return choice;
    }

    vector<tuple<size_t, size_t>> Game::chooseTokens(const bool canTakeGold, const bool onlyOneToken) {
        vector<tuple<size_t, size_t>> choice;
        map<Color, int> nbByColor;
        board.displayBoard();
        int answer = 1;
        if (!canTakeGold) {
            if (!onlyOneToken) {
                cout << "How many tokens do you want to take?\n";
                answer = -1;
                while (answer <= 0 || answer > 3) {
                    cout << "Please enter a number between 1 and 3 (enter 0 to cancel):\n";
                    if (cin >> answer && answer == 0) return choice;
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
            } else {
                answer = 1;
            }
            cout << "You cannot take a Gold token.\n";
        } else cout << "You need to take a Gold token from the board.\n";
        int i = 0;
        int x = -1;
        int y = -1;
        while (i < answer) {
            cout << "Token number: " << i + 1 << "\n";
            while (x <= 0 || x > 5) {
                cout << "Please enter a line number between 1 and 5 (enter 0 to cancel):\n";
                if (cin >> x && x == 0) {
                    choice.clear();
                    return choice;
                }
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            while (y <= 0 || y > 5) {
                cout << "Please enter a column number between 1 and 5 (enter 0 to cancel):\n";
                if (cin >> y && y == 0) {
                    choice.clear();
                    return choice;
                }
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            x--; // passage en index depart a 0
            y--;
            if (board.getTokens()[board.getPosition(x, y)] != nullptr) {
                if (!canTakeGold && board.getTokens()[board.getPosition(x, y)]->getColor() == Color::gold) {
                    cout << "You cannot take a Gold token.\n";
                } else if (canTakeGold && board.getTokens()[board.getPosition(x, y)]->getColor() !=
                                          Color::gold) {
                    cout << "You need to take a Gold token.\n";
                } else {
                    if (i == 0) {
                        choice.emplace_back(x, y);
                        i++;
                    } else {
                        choice.emplace_back(x, y);
                        if (checkProximity(choice) && checkCollinearity(choice)) {
                            i++;
                        } else {
                            choice.pop_back();
                            cout << "Invalid coordinates. Please try again.\n";
                        }
                    }
                    nbByColor[board.getTokens()[board.getPosition(x, y)]->getColor()] += 1;
                }
            }
            x = -1;
            y = -1;
        }
        if (!onlyOneToken && checkNbColors(nbByColor)) {
            string playerName = activePlayer->getName();
            changePlayer();
            string oppName = activePlayer->getName();
            takePrivilege();
            changePlayer();
            cout << playerName << " has taken 3 tokens of the same color or 2 Pearl tokens, "
                 << oppName << " receives 1 Privilege.\n";
        }
        return choice;
    }

    bool checkNbColors(const map<Color, int> &tokens) {
        return any_of(
                tokens.cbegin(), tokens.cend(),
                [](const auto kv) { return kv.second == 3 || (kv.first == Color::pearl && kv.second == 2); }
        );
    }

    tuple<vector<CardJewels *>, size_t> Game::chooseCardJewelsAI(const bool buying) {
        int choice;
        vector<int> validChoices;
        int nbCardsPerLine = 3;
        vector<vector<CardJewels *>> cards = {cardsLvl3, cardsLvl2, cardsLvl1};
        for (int nbContainer = 0; nbContainer < cards.size(); nbContainer++) {
            for (int i = 0; i < nbCardsPerLine; i++) {
                if (cards[nbContainer][i]) {
                    if (canBuyJewel(cards[nbContainer][i]) || !buying) {
                        int chosenCard = 10 * (nbContainer + 1) + (i + 1);
                        validChoices.push_back(chosenCard);
                    }
                }
            }
        }

        if (validChoices.empty()) {
            choice = 0;
        } else {
            random_device dev;
            mt19937 rng(dev());
            uniform_int_distribution<> dis(0, validChoices.size() - 1);
            choice = validChoices[dis(rng)];
        }
        if (choice == 0)
            return {cardsLvl3, -1};
        return {cards[(choice - 11) / 10], (choice - 11) % 10};
    }

    double distance(const tuple<size_t, size_t> &pt1, const tuple<size_t, size_t> &pt2) {
        const auto [x1, y1] = pt1;
        const auto [x2, y2] = pt2;
        return sqrt((y2 - y1) * (y2 - y1) + (x2 - x1) * (x2 - x1));
    }

    bool checkProximity(const vector<tuple<size_t, size_t>> &choice) {
        if (choice.size() == 1)
            return true;
        if (choice.size() == 2)
            return distance(choice[0], choice[1]) < 2;
        if (choice.size() == 3) {
            int nbCloseTokens = 0;
            for (int i = 0; i < 3; i++) {
                for (int j = i + 1; j < 3; j++)
                    if (const auto dist = distance(choice[i], choice[j]); dist < 2)
                        nbCloseTokens++;
            }
            return nbCloseTokens == 2;
        }
        throw SplendorException("Number of selected tokens invalid.");
    }

    bool checkCollinearity(const vector<tuple<size_t, size_t>> &choice) {
        if (choice.size() > 3)
            throw SplendorException("Too many selected tokens.");
        if (choice.size() != 3)
            return true;
        vector<size_t> coords;
        for (const auto [x, y]: choice) {
            coords.push_back(x);
            coords.push_back(y);
        }
        // formula found here: https://math.stackexchange.com/a/405981
        return (coords[3] - coords[1]) * (coords[4] - coords[2]) == (coords[5] - coords[3]) * (coords[2] - coords[0]);
    }

    tuple<vector<CardJewels *>, size_t> Game::chooseCardJewels(const bool buying) {
        int choice;
        vector<int> validChoices;
        cout << "Pyramid:\n";
        printPyramid();
        auto playerReserve = activePlayer->getReserve();
        auto nbReserve = playerReserve.size();
        if (nbReserve > 0) {
            cout << "Reserve:\n";
            printCardsLine(playerReserve, nbReserve, false);
        }
        int nbCardsPerLine = 3;
        cout << "\nChoose the card you want by entering its number:\n";
        vector<vector<CardJewels *>> cards = {cardsLvl3, cardsLvl2, cardsLvl1, playerReserve};
        cout << "\nPyramid:";
        for (int nbContainer = 0; nbContainer < cards.size() - 1; nbContainer++) {
            cout << "\n";
            for (int space = nbCardsPerLine; space < 5; space++)
                cout << "  ";
            for (int i = 0; i < nbCardsPerLine; i++) {
                if (cards[nbContainer][i]) {
                    if (canBuyJewel(cards[nbContainer][i]) || !buying) {
                        int chosenCard = 10 * (nbContainer + 1) + (i + 1);
                        validChoices.push_back(chosenCard);
                        cout << chosenCard;
                    } else
                        cout << "XX";
                } else
                    cout << "XX";
                cout << "  ";
            }
            cout << "\n";
            nbCardsPerLine++;
        }
        if (nbReserve > 0) {
            cout << "\nReserve:\n";
            for (int i = 0; i < nbReserve; i++) {
                if (canBuyJewel(playerReserve[i]) || !buying) {
                    int chosenCard = 40 + (i + 1);
                    validChoices.push_back(chosenCard);
                    cout << chosenCard;
                } else
                    cout << "XX";
                cout << "  ";
            }
            cout << "\n";
        }
        cout << "\nChosen Royal card (enter 0 to cancel):";
        cin >> choice;
        while (find(validChoices.cbegin(), validChoices.cend(), choice) == validChoices.cend() && choice != 0) {
            cout << "\nInvalid choice, please enter a valid choice:";
            cin >> choice;
        }
        if (choice == 0)
            return {cardsLvl3, -1};
        return {cards[(choice - 11) / 10], (choice - 11) % 10};
    }

    size_t Game::chooseCardRoyals() const {
        int choice;
        vector<int> validChoices;
        cout << "\nYou have accumulated enough crowns to take a Royal card!\n";
        printRoyals(cardsRoyals);
        cout << "\nChoose your card:\n";
        for (int i = 0; i < cardsRoyals.size(); i++) {
            cout << "  " << i + 1;
            validChoices.push_back(i + 1);
        }
        cout << "\nChosen Royal card:";
        while (find(validChoices.cbegin(), validChoices.cend(), choice) == validChoices.cend() && choice != 0) {
            cout << "\nInvalid choice, please enter a valid choice:";
            cin >> choice;
        }
        return choice - 1;
    }

    void Game::usePrivilege(const token::Token &token) {
        if (activePlayer->getNbPrivileges() != 0) {
            activePlayer->privilegeRemove();
            board.removeToken(token);
            activePlayer->takeToken(token.getColor());
        }
    }

    void Game::takeToken(size_t x, size_t y) {
        if (board.getTokens()[board.getPosition(x, y)] != nullptr) {
            const Color color = board.getTokens()[board.getPosition(x, y)]->getColor();
            activePlayer->takeToken(color);
            board.removeToken(*board.getTokens()[board.getPosition(x, y)]);
        }
    }

    bool Game::canBuyJewel(const card::CardJewels *card) const {
        auto tokens = activePlayer->getTokens();
        size_t nbGoldTokens = tokens[Color::gold];
        for (auto kv: *card->getPrice()) {
            const int diff = kv.second - tokens[kv.first] - activePlayer->getBonus()[kv.first];
            if (diff > long(nbGoldTokens) && diff >= 0)
                return false;
            if (diff > 0) {
                nbGoldTokens -= diff;
            }
        }
        return true;
    }

    bool Game::canReserveJewel() const {
        const auto tokens = board.getTokens();
        return activePlayer->getReserve().size() < 3 &&
               any_of(
                       tokens.cbegin(),
                       tokens.cend(),
                       [](auto jeton) {
                           if (jeton) return jeton->isGold();
                           return false;
                       }
               );
    }

    void Game::takeRoyal(size_t index) {
        activePlayer->takeCardRoyals(cardsRoyals[index]);
        cardsRoyals[index] = cardsRoyals[cardsRoyals.size() - 1];
        cardsRoyals.pop_back();
    }

    void Game::takeJewel(const vector<card::CardJewels *> &container, size_t index, bool buying) {
        if (buying)
            activePlayer->takeCardJewels(container[index]);
        else
            activePlayer->reserveCardJewels(container[index]);
        if (container == cardsLvl1) {
            if (cardsLvl1.size() > 5) {
                cardsLvl1[index] = cardsLvl1[cardsLvl1.size() - 1];
                cardsLvl1.pop_back();
            } else cardsLvl1[index] = nullptr;
        } else if (container == cardsLvl2) {
            if (cardsLvl2.size() > 4) {
                cardsLvl2[index] = cardsLvl2[cardsLvl2.size() - 1];
                cardsLvl2.pop_back();
            } else cardsLvl2[index] = nullptr;
        } else if (container == cardsLvl3) {
            if (cardsLvl3.size() > 3) {
                cardsLvl3[index] = cardsLvl3[cardsLvl3.size() - 1];
                cardsLvl3.pop_back();
            } else cardsLvl3[index] = nullptr;
        } else if (container == activePlayer->getReserve()) {
            activePlayer->removeFromReserve(container[index]);
        } else throw SplendorException("Invalid container.");
    }

    void Game::buyJewel(const vector<card::CardJewels *> &container, size_t index) {
        for (const auto [color, colorPrice]: *container[index]->getPrice()) {
            for (int i = 0; i < max(int(colorPrice) - int(activePlayer->getBonus()[color]), 0); i++) {
                Color usedColor = color;
                if (activePlayer->getTokens()[color] == 0)
                    usedColor = Color::gold;
                bag.returnTokenToBag(usedColor);
                activePlayer->giveToken(usedColor);
            }
        }
        takeJewel(container, index, true);
    }

    bool Game::endOfTurnChecks() {
        if (const size_t nb = activePlayer->getNbTokens(); nb > 10) {
            if (!activePlayer->isAI()) discardExcessTokens(nb - 10);
            else discardExcessTokensAI(nb - 10);
        }
        return winCheck();
    }

    void Game::discardExcessTokensAI(size_t nb) {
        char answer;
        vector<char> validChoices = {'B', 'W', 'G', 'N', 'R', 'P', 'A'};
        size_t i = 0;
        int c = -1;
        random_device dev;
        mt19937 rng(dev());
        uniform_int_distribution<> dis(0, validChoices.size() - 1);
        while (i < nb) {
            answer = validChoices[dis(rng)];
            switch (answer) {
                case 'B':
                    c = 0;
                    break;
                case 'W':
                    c = 1;
                    break;
                case 'G':
                    c = 2;
                    break;
                case 'N':
                    c = 3;
                    break;
                case 'R':
                    c = 4;
                    break;
                case 'P':
                    c = 5;
                    break;
                case 'A':
                    c = 6;
                    break;
                default:
                    c = -1;
                    break;
            }
            if (c != -1) {
                if (activePlayer->getTokens()[static_cast<Color>(c)] != 0) {
                    activePlayer->giveToken(static_cast<Color>(c));
                    bag.returnTokenToBag(static_cast<Color>(c));
                    i++;
                }
            }
        }
    }

    void Game::discardExcessTokens(size_t nb) {
        char answer = ' ';
        cout << "You have more than 10 tokens. You need to put some back in the bag.\n";
        size_t i = 0;
        int colorID = -1;
        while (i < nb) {
            cout << "You need to put " << nb - i <<
                 "tokens in the bag.\nChoose the color of a token you want to put back: R for 1 red token.\n";
            cin >> answer;
            switch (answer) {
                case 'B':
                    colorID = 0;
                    break;
                case 'W':
                    colorID = 1;
                    break;
                case 'G':
                    colorID = 2;
                    break;
                case 'N':
                    colorID = 3;
                    break;
                case 'R':
                    colorID = 4;
                    break;
                case 'P':
                    colorID = 5;
                    break;
                case 'A':
                    colorID = 6;
                    break;
                default:
                    colorID = -1;
                    break;
            }
            if (colorID != -1) {
                if (activePlayer->getTokens()[static_cast<Color>(colorID)] != 0) {
                    activePlayer->giveToken(static_cast<Color>(colorID));
                    bag.returnTokenToBag(static_cast<Color>(colorID));
                    i++;
                }
            }
        }
    }

    bool Game::winCheck() const {
        return activePlayer->hasWonByColorPrestige() || activePlayer->getNbCrowns() >= 10 || activePlayer->
                getNbPrestiges() >= 20;
    }

    void Game::takePrivilege() {
        if (activePlayer->getNbPrivileges() < 3) {
            if (nbFreePrivileges != 0) {
                activePlayer->privilegeAdd();
                nbFreePrivileges--;
            } else {
                changePlayer();
                activePlayer->privilegeRemove();
                changePlayer();
                activePlayer->privilegeAdd();
            }
        }
    }

    void Game::changePlayer() {
        if (activePlayer == &players[0]) activePlayer = &players[1];
        else activePlayer = &players[0];
    }


    void Game::cardAbilityActivation(const vector<card::CardJewels *> &container, size_t index) {
        if (container[index]->getAbility() == Ability::take_opp_token) {
            if (!activePlayer->isAI()) takeOppToken();
            else takeOppTokenAI();
        }
        if (container[index]->getAbility() == Ability::take_a_token) {
            takeTokenFromBoardAbility(container[index]->getDiscountColor());
        }
        if (container[index]->getAbility() == Ability::take_a_privilege) {
            takePrivilege();
        }
        if (container[index]->getAbility() == Ability::play_again) {
            changePlayer();
        }
    }

    void Game::takeOppTokenAI() {
        int i = 0;
        char answer;
        if (activePlayer == &players[0]) {
            i = 1;
        }
        if (players[i].getTokens().empty()) {
            return;
        }
        vector<char> validChoices = {'B', 'W', 'G', 'N', 'R', 'P'};
        random_device dev;
        mt19937 rng(dev());
        uniform_int_distribution<> dis(0, validChoices.size() - 1);
        answer = validChoices[dis(rng)];
        Color chosenColor;
        switch (answer) {
            case 'B':
                chosenColor = Color::blue;
                break;
            case 'W':
                chosenColor = Color::white;
                break;
            case 'G':
                chosenColor = Color::green;
                break;
            case 'N':
                chosenColor = Color::black;
                break;
            case 'R':
                chosenColor = Color::red;
                break;
            case 'P':
                chosenColor = Color::pearl;
                break;
            default:
                throw SplendorException("Invalid color.");
        }
        activePlayer->takeToken(chosenColor);
        players[i].giveToken(chosenColor);
    }


    void Game::takeOppToken() {
        int i = 0;
        char answer = ' ';
        if (activePlayer == &players[0]) {
            i = 1;
        }
        if (players[i].getTokens().empty())
            return;
        cout << "Choose the color of the token you want to steal to your opponent: \n";
        for (const auto [color, nbTokens]: players[i].getTokens()) {
            if (nbTokens != 0 && color != Color::gold) {
                cout << color << ", ";
            }
        }
        cout << "\n";
        cin >> answer;
        Color chosenColor;
        switch (answer) {
            case 'B':
                chosenColor = Color::blue;
                break;
            case 'W':
                chosenColor = Color::white;
                break;
            case 'G':
                chosenColor = Color::green;
                break;
            case 'N':
                chosenColor = Color::black;
                break;
            case 'R':
                chosenColor = Color::red;
                break;
            case 'P':
                chosenColor = Color::pearl;
                break;
            default:
                throw SplendorException("Invalid color.");
        }
        activePlayer->takeToken(chosenColor);
        players[i].giveToken(chosenColor);
    }

    void Game::takeTokenFromBoardAbility(token::Color color) {
        bool correctlyChosen = false;
        auto tokens = board.getTokens();
        if (any_of(
                tokens.cbegin(),
                tokens.cend(),
                [color](const Token *token) {
                    if (token != nullptr) return token->getColor() == color;
                    return false;
                }
        ))
            while (!correctlyChosen) {
                int x;
                int y;
                if (!activePlayer->isAI()) {
                    cout << "You need to choose a " << color << " token.\n";
                    const tuple<size_t, size_t> choice = chooseTokens(false, true)[0];
                    tie(x, y) = choice;
                } else {
                    const tuple<size_t, size_t> choice = chooseTokensAI(false, true, true, color)[0];
                    tie(x, y) = choice;
                }
                if (x != -1 && board.getTokens()[board.getPosition(x, y)]->getColor() == color) {
                    correctlyChosen = true;
                    activePlayer->takeToken(color);
                    board.removeToken(*board.getTokens()[board.getPosition(x, y)]);
                } else cout << "Invalid choice, try again.\n";
            }
        else cout << "The are no tokens of this color on the board!\n";
    }
}
