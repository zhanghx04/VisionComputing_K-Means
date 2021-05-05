#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>

#include <QElapsedTimer>

class ViewWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT

public:
  ViewWidget(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

  float angleForTime(qint64 msTime, float secondsPerRotation) const;

  void samplePointsGenteration();
//  void getPoints(std::uniform_real);
  void dataGeneration(int samplesPerCluster, int dim);


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

  // For Elapsed Timer
  QOpenGLShaderProgram m_pointProgram;
  QElapsedTimer m_elapsedTimer;

  QElapsedTimer m_fpsTimer;
  int m_frameCount;
  float m_fps;
};

#endif // VIEWWIDGET_H
