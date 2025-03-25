#include "rlt.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    rlt w;
    w.show();
    return a.exec();
}
