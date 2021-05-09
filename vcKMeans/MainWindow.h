#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ViewWidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

public slots:
  void zoomIn();
  void zoomOut();
  void zoomActualSize();

  void setZoom(int zoom);

  void generate();

private:
  Ui::MainWindow *ui;
  float the_zoom;

};
#endif // MAINWINDOW_H

