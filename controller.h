#pragma once

#include "string"
#include "game.h"

using namespace std;

namespace splendor_duel {
    bool fichierMemoireExiste(const filesystem::path& name);

    class Controleur {
        static Controleur* instanceUnique;
        partie::Partie* partie = nullptr;

        Controleur() = default;

        Controleur(const Controleur&) = default;

        ~Controleur() = default;

    public:
        static Controleur* donneInstance();

        static void libereInstance();

        void start();

        void nouvellePartie();

        void partieContinuee();

        void jouerPartie();
    };
}
