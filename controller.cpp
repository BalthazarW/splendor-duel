#include "iostream"
#include "algorithm"
#include "controller.h"

using namespace std;

namespace splendor_duel {
    Controller *Controller::uniqueInstance = nullptr;

    bool memoryFileExists(const filesystem::path &path) {
        if (FILE *file = fopen(path.string().c_str(), "r")) {
            fclose(file);
            return true;
        }
        return false;
    }

    void Controller::continuedGame() {
        splendorGame = game::Game::loadState();
    }

    Controller *Controller::giveInstance() {
        if (uniqueInstance == nullptr)
            uniqueInstance = new Controller;
        return uniqueInstance;
    }

    void Controller::freeInstance() {
        delete uniqueInstance;
        uniqueInstance = nullptr;
    }

    void Controller::start() {
        cout << "Splendor Duel\n1: Nouvelle game\n";
        if (const filesystem::path memoryPath = game::getBasePath() / "save" / "game.xml"; memoryFileExists(memoryPath))
            cout << "2: Continuer une game\n";
        cout << "0: Quitter\n";
        int choice;
        cin >> choice;
        if (choice == 1) {
            newGame();
        } else if (choice == 2) {
            continuedGame();
        } else return;
        splendorGame->play();
        if (splendorGame->hasEnded())
            remove("../save/game.xml");
        else
            splendorGame->saveState();
    }

    void Controller::newGame() {
        //demander players
        cout << "\n1: 1 Player (contre ordinateur)\n2: 2 players\n3: Ordinateur contre ordinateur\n0: Annuler\n";
        {
            int choice;
            const vector validChoices{1, 2, 3, 0};
            while (true) {
                if (cin >> choice && find(validChoices.cbegin(), validChoices.cend(), choice) != validChoices.cend())
                    break;
                cout << "Choix invalide, reessayez :\n";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
            if (choice == 0) {
                start();
                return;
            }
            int choiceAI = 0;
            if (choice != 2) {
                cout << "\nNiveau de l'IA : \n1: niveau 1 (aleatoire)\n0: Annuler\n";
                const vector validChoicesAI{1};
                while (true) {
                    if (cin >> choiceAI && find(validChoicesAI.begin(), validChoicesAI.end(), choiceAI) != validChoicesAI.end())
                        break;
                    cout << "Choix invalide, reessayez :\n";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }
                if (choiceAI == 0) {
                    newGame();
                    return;
                }
            }
            string namePlayer1 = "Bot Alpha";
            string namePlayer2 = "Bot Beta";
            player::Player player1(namePlayer1, true, choiceAI);
            player::Player player2(namePlayer2, true, choiceAI);
            if (choice != 3) {
                cout << "Nom du Player 1 (sans espaces) :";
                cin >> namePlayer1;
                player1 = player::Player(namePlayer1, false);
            }
            if (choice == 2) {
                cout << "Nom du Player 2 (sans espaces) :";
                cin >> namePlayer2;
                player2 = player::Player(namePlayer2, false);
            }
            splendorGame = new game::Game(player1, player2, true);
        } // entre accolades pour permettre de free toutes les variables locales
        // vu que la fonction se termine seulement lorsqu'on ferme le jeu
    }
}
