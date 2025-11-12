#include <QApplication>
#include "MainWindow.h"

#if defined(FAKECAD_SINGLE_EXE) && defined(QT_STATIC) && defined(Q_OS_WIN)
#  include <QtPlugin>
   Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#endif

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.resize(1024, 768);
    w.show();
    return app.exec();
}
