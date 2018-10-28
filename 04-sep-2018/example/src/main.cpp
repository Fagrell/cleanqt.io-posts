#include <QApplication>
#include <QSplitter>
#include <QStringListModel>
#include <QComboBox>
#include <QListView>
#include <QStandardItemModel>
#include <QStandardItem>

#include <QTableView>
#include <QtWidgets>

int main(int argc, char *argv[]) {
  QApplication app{argc, argv};
  QWidget window;
  new QLabel{"Hello World!", &window};
  window.show();
  return app.exec();
}
