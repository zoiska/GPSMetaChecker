#include <QApplication>
#include "MainWindow.h"

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    MainWindow w;
    w.show();
    return QApplication::exec();
}