#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);
  app.setWindowIcon(QIcon(":/texteditor.svg"));
  app.setApplicationName("Text Editor");
  app.setOrganizationName("QtEditor");

  MainWindow w;
  if (argc > 1)
    w.openFileFromPath(argv[1]);
  w.show();
  return app.exec();
}
