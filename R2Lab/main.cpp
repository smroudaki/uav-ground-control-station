#include "dialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Dialog w(0, true);
    w.setWindowFlags(Qt::WindowMinimizeButtonHint);
    w.show();

    return a.exec();
}
