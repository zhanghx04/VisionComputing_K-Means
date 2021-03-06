#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include <QElapsedTimer>
#include <QTimer>
#include <QMainWindow>

#include "MainWindow.h"

class ViewWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

public: // *parent = nullptr
  ViewWidget(QWidget *parent, Qt::WindowFlags f = Qt::WindowFlags());

  int zoom() const;
  float turn() const;
  float vertical() const;
  float horizontal() const;
  float speed() const;
  float energy() const;
  int iter() const;
  int total_iter() const;

  float angleForTime(qint64 msTime, float secondsPerRotation) const;



  void samplePointsGenteration();
  void initialData();
//  void getPoints(std::uniform_real);
  void dataGeneration(int samplesPerCluster, int dim);
  void dataGenerateFromFile(QString filename);
  void saveData(QString filename);

  float dist2(QVector<float> point1, QVector<float> point2);
  float nearestDistance(QVector<float> point, QVector<float> centers);
  void initialCenters();
  void check_params(int k, QString distance_function, QString center_initial_method);
  int find_closest(QVector<float> point);
  void k_means_iter();
  void doKmeans(int k, QString distance_function, QString center_initial_method);

  void total_energy(); // calculate total energy

public slots:
  void setZoom(int zoom);
  void setTurn(float angle);
  void setMove(float step, int direction);
  // receive data from GUI from MainWindow
  void dataReceive(int k, int speed, int samplePerCluster,
                   int dim, QString dist_method, QString cent_method,
                   float point_size, float center_size,
                   QString filename, bool if_file_data, int num_iter, bool if_step,
                   float the_zoom, float v_direct, float h_direct);
  void step_iteration(int num_iter);

protected:
  void initializeGL() override;
  void paintGL() override;

private slots:
  void updateTurntable();

private:
  float m_zoom;
  float m_angle = 0.0f;
  float m_v_direct = 0.0f;
  float m_h_direct = 0.0f;

  float m_turntableAngle = 0.0f;

  // Check if is 3D data, if so the view will turn
  bool m_spin = false;

  QVector<float> m_points;
  QVector<float> m_colors;
  QVector<float> m_ave_point {0.0f, 0.0f, 0.0f};

  // For Elapsed Timer
  QOpenGLShaderProgram m_pointProgram;
  QElapsedTimer m_elapsedTimer;


  QElapsedTimer m_fpsTimer;
  QElapsedTimer m_speedTimer; // record total time used
  int m_frameCount;
  float m_fps;


  // K-Means
  QString m_dist_m;
  QString m_cent_m;

  QTimer *ttime = new QTimer(this);
  bool m_isTXTfile = false;
  bool m_ifStep = false;
  int m_num_iter;
  QString m_folder;
  QString m_filename;

  int account = 0;  // accout how many iterations
  QVector<float> centers_ref {};

  float m_pointSize = 3.0f;
  float m_centerSize = 10.0f;

  int m_dim;          // Dimension
  int m_totalSample;  // total number of points
  int m_samplePerCluster; // how many sample geteraed for each cluster.
  int m_k;            // number of cluster
  int m_timer;        // how fast for each iteration pause (timer)
  int m_dist_method;  // distance method
  int m_cent_method;  // center initial method
  QVector<float> m_centers;       // store centers
  QVector<float> m_centerColors;  // save centers' color

  float m_speed = 0;
  float m_energy = 0;
  bool if_last_iter = false;

  // for storing info of each iteration
  QVector< QVector<float>> record_centers;
  QVector< QVector<float>> record_centerColor;
  QVector< QVector<float>> record_pointColor;


};

#endif // VIEWWIDGET_H
