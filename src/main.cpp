#include <QApplication>
#include <QDebug>
#include <QSettings>
#include "app.h"
#include "mainwindow.h"
#include "sequence.h"
#include "genom.h"
#include "quazip.h"

using namespace big::core;
using namespace big::gui;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("BIG");
    QCoreApplication::setOrganizationDomain("labsquare.org");
    QCoreApplication::setApplicationName("Big Browser");

    App::i()->loadSettings();

    MainWindow win;
    win.show();



    return a.exec();
}
