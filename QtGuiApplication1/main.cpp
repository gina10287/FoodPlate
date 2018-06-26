#include "QtGuiApplication1.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]) {
	QCoreApplication::addLibraryPath(".");
    QApplication a(argc, argv);
    QtGuiApplication1 w;
	w.show();
    return a.exec();
}
