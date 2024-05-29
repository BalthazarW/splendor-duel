#include "controller.h"
#include <QApplication>
#include <QInputDialog>
#include <QMessageBox>
#include "splendor_duel.h"

int main(int argc, char *argv[]) {
    if (argc > 1) {
        for (int i = 0; i < argc; ++i) {
            if (strcmp(argv[i], "--terminal") || strcmp(argv[i], "-t")) {
                 splendor_duel::Controleur* splendor = splendor_duel::Controleur::donneInstance();
                 splendor->start();
                 return 0;
            }
        }
    }
    QApplication app(argc, argv);

    SplendorDuel window;
    window.show();

    return QApplication::exec();
}
