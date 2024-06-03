#pragma once

#include "string"
#include "game.h"

using namespace std;

namespace splendor_duel {
    bool memoryFileExists(const filesystem::path &path);

    class Controller {
        static Controller *uniqueInstance;
        game::Game *splendorGame = nullptr;

        Controller() = default;

        Controller(const Controller &) = default;

        ~Controller() = default;

    public:
        static Controller *giveInstance();

        static void freeInstance();

        void start();

        void newGame();

        void continuedGame();
    };
}
