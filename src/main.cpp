#include "labelmaker.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LabelMaker w;
    w.showMaximized();

    return a.exec();
}
