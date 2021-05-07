#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include <QElapsedTimer>
#include <QTimer>

class ViewWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

public:
  ViewWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

  float angleForTime(qint64 msTime, float secondsPerRotation) const;

  void samplePointsGenteration();
//  void getPoints(std::uniform_real);
  void dataGeneration(int samplesPerCluster, int dim);
  void dataGenerateFromFile(QString filename);
  void initialCenters();
  void check_params(int k, QString distance_function, QString center_initial_method);
  int find_closest(QVector<float> point);
  void k_means_iter();
  void doKmeans(int k, QString distance_function, QString center_initial_method);


protected:
  void initializeGL() override;
  void paintGL() override;

private slots:
  void updateTurntable();

private:
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
  int m_frameCount;
  float m_fps;

  // K-Means
  bool m_isTXTfile = false;
  QString m_folder;


  int m_dim;          // Dimension
  int m_totalSample;  // total number of points
  int m_k;            // number of cluster
  int m_dist_method;  // distance method
  int m_cent_method;  // center initial method
  QVector<float> m_centers;       // store centers
  QVector<float> m_centerColors;  // save centers' color

};

#endif // VIEWWIDGET_H
