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

  // turning
  void turnLeft();
  void turnRight();
  void setTurn(float turn);

  // direction
  void goUp();
  void goDown();
  void goLeft();
  void goRight();
  void goCenter();
  void setMove(float step, int direction);

  void generate();

  void displayResult();

  // step thru iterations
  void previous_iteration();
  void next_iteration();


private:
  Ui::MainWindow *ui;

  bool gostep = false;

  float the_zoom;
  float v_direct;
  float h_direct;
};
#endif // MAINWINDOW_H

