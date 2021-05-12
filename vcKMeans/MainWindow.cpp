#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QActionGroup>
#include <QObject>


#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  ViewWidget *viewWidget = new ViewWidget(this);

  // set range of speed and sample per cluster
  ui->k_spinBox->setRange(2,99999);
  ui->timer_spinBox->setRange(100, 10000);
  ui->sample_spinBox->setRange(10, 1000);

  // connect to "genterate" button
  connect(ui->generate_pushButton, &QPushButton::released, this, &MainWindow::generate);
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::generate()
{
  // v1
  int k = ui->k_spinBox->value();
  int timer = ui->timer_spinBox->value();
  int samplePerCluster = ui->sample_spinBox->value();
  // v2
  int dim = ui->dim_comboBox->currentText().toInt();
  QString dist_method = ui->dist_m_comboBox->currentText();
  QString cent_method = ui->cent_m_comboBox->currentText();
  // v3
  float point_size = ui->point_size_doubleSpinBox->value();
  float center_size = ui->cent_size_doubleSpinBox->value();
  // v4
  QString filename = ui->file_name_comboBox->currentText();
  bool if_file_data = ui->if_file_checkBox->checkState();
  // bot
  int num_iter = ui->iter_spinBox->value();
  bool if_step = ui->each_step_checkBox->checkState();
  gostep = if_step;

  qDebug() << "Sending signal...";

  if (dim == 3) {
    the_zoom = 800;
    v_direct = 55;
    h_direct = 155;
  } else {
    if (if_file_data) {
      the_zoom = 4;
      v_direct = 10;
      h_direct = 0;
    } else {
      the_zoom = 500;
      v_direct = 255;
      h_direct = 255;
    }
  }

  if (if_step) {
    timer = 0;
    ui->cur_iter_lcdNumber->display(num_iter);
  }

  // disable buttons for stepping thru iteration
  ui->prev_pushButton->setDisabled(!if_step);
  ui->next_pushButton->setDisabled(true);

  // turn
  ui->turn_left_pushButton->setEnabled(dim == 3);
  ui->turn_right_pushButton->setEnabled(dim == 3);

  ui->viewWidget->dataReceive(k, timer, samplePerCluster,
                   dim, dist_method, cent_method,
                   point_size, center_size,
                   filename, if_file_data, num_iter, if_step, the_zoom, v_direct, h_direct);

}

void MainWindow::displayResult()
{
  ui->speed_lineEdit->setText(QString::number(ui->viewWidget->speed()) + " ms");
  ui->energy_lineEdit->setText(QString::number(ui->viewWidget->energy()) + " J");
}

void MainWindow::previous_iteration()
{
  ui->viewWidget->step_iteration(ui->viewWidget->iter()-1);
  ui->prev_pushButton->setEnabled(ui->viewWidget->iter() != 1);
  ui->next_pushButton->setEnabled(ui->viewWidget->iter() != ui->viewWidget->total_iter());
  ui->cur_iter_lcdNumber->display(ui->viewWidget->iter());
}

void MainWindow::next_iteration()
{
  ui->viewWidget->step_iteration(ui->viewWidget->iter()+1);
  ui->prev_pushButton->setEnabled(ui->viewWidget->iter() != 1);
  ui->next_pushButton->setEnabled(ui->viewWidget->iter() != ui->viewWidget->total_iter());
  ui->cur_iter_lcdNumber->display(ui->viewWidget->iter());
}

void MainWindow::zoomIn()
{
  // zoom in by 2
  setZoom(ui->viewWidget->zoom() * 0.5);
}

void MainWindow::zoomOut()
{
  // zoom out by 2
  setZoom(ui->viewWidget->zoom() * 2.0f);
}

void MainWindow::zoomActualSize()
{
  // reset zoom
  qDebug() << "zoomActualSize() Called!";
  setZoom(the_zoom);
}

void MainWindow::setZoom(int zoom)
{
  ui->viewWidget->setZoom(zoom);

  ui->actionActual_Size->setEnabled(ui->viewWidget->zoom() != the_zoom);
  ui->actionZoom_Out->setEnabled(ui->viewWidget->zoom() != the_zoom);
}

void MainWindow::turnLeft()
{
  setTurn(ui->viewWidget->turn() - 10);
}

void MainWindow::turnRight()
{
  setTurn(ui->viewWidget->turn() + 10);
}

void MainWindow::setTurn(float turn)
{
  ui->viewWidget->setTurn(turn);

  ui->turn_left_pushButton->setEnabled(ui->dim_comboBox->currentText().toInt() == 3);
  ui->turn_right_pushButton->setEnabled(ui->dim_comboBox->currentText().toInt() == 3);
}

void MainWindow::goUp()
{
  setMove(ui->viewWidget->vertical() + 100, 1);
}

void MainWindow::goDown()
{
  setMove(ui->viewWidget->vertical() - 100, 1);
}

void MainWindow::goLeft()
{
  setMove(ui->viewWidget->horizontal() - 100, 2);
}

void MainWindow::goRight()
{
  setMove(ui->viewWidget->horizontal() + 100, 2);
}

void MainWindow::goCenter()
{
  setMove(0, 1);
  setMove(0, 2);
}

void MainWindow::setMove(float step, int direction)
{
  ui->viewWidget->setMove(step, direction);
}
