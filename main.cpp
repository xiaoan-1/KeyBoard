#include "mainwindow.h"

#include <QApplication>

#include "keyboard.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();


    KeyBoard::instance(&w);

    return a.exec();
}
