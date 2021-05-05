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

protected:
  void initializeGL() override;
  void paintGL() override;

private slots:
  void updateTurntable();

private:
  float m_turntableAngle = 0.0f;

  QVector<float> m_points;
  QVector<float> m_colors;

  QOpenGLShaderProgram m_pointProgram;
  QElapsedTimer m_elapsedTimer;

  QElapsedTimer m_fpsTimer;
  int m_frameCount;
  float m_fps;
};

#endif // VIEWWIDGET_H
