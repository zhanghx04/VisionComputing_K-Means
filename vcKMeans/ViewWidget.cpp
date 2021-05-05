#include "ViewWidget.h"

#include <QOpenGLShaderProgram>
#include <QPainter>
#include <QtMath>
#include <QTimer>
#include <chrono>
#include <random>

#include <QDebug>


ViewWidget::ViewWidget(QWidget *parent, Qt::WindowFlags f) : QOpenGLWidget(parent, f)
{
  auto turntableTimer = new QTimer(this);
  turntableTimer->callOnTimeout(this, &ViewWidget::updateTurntable);

  // control the fps
  turntableTimer->start(1000.0/60.0); // 30 ms

  m_elapsedTimer.start();

//  samplePointsGenteration();

  dataGeneration(100, 2);

  qDebug() << width() << height() << m_points.length();

  m_fpsTimer.start();
}

float ViewWidget::angleForTime(qint64 msTime, float secondsPerRotation) const
{
  float millisecondsPerRotation {secondsPerRotation*1000.0f};
  float t {msTime/millisecondsPerRotation};

  return (t-qFloor(t)) * 360.0f;
}

void ViewWidget::samplePointsGenteration()
{
  // Create random 3D points
  int samples {10000};

  // Get seed from clock
  long seed {std::chrono::system_clock::now().time_since_epoch().count()};

  // Seed engine and set random distribution to [-1, 1]
  std::default_random_engine engine(seed);
  std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);

  // Create points inside a sphere
  int count {0};
  while (count < samples) {
    // Uniformly sample cube
    float x {distribution(engine)};
    float y {distribution(engine)};
    float z {distribution(engine)};

    // Reject all points ouside the sphere
    if (std::sqrt(x*x + y*y + z*z) <= 1.0f) {
      m_points.append({x, y, z});

      // Re-map positions to [0, 1] and use as color
      float r { (x+1.0f)/2.0f };
      float g { (y+1.0f)/2.0f };
      float b { (z+1.0f)/2.0f };

      m_colors.append({r, g, b});

      count++;
    }
  }
}

void ViewWidget::dataGeneration(int samplesPerCluster, int dim)
{
  if (dim > 2) {
    m_spin = true;
  } else {
    m_spin = false;
  }

  // Get seed from clock
  long seed {std::chrono::system_clock::now().time_since_epoch().count()};

  // Seed engine and set random distribution to [-1, 1]
  std::default_random_engine engine(seed);
  std::normal_distribution<float> distribution1(50.0f, 50.0f);
  std::normal_distribution<float> distribution2(100.0f, 50.0f);
  std::normal_distribution<float> distribution3(140.0f, 60.0f);
  std::normal_distribution<float> distribution4(10.0f, 50.0f);

  // Create points inside a sphere
  int count {0};
  while (count < samplesPerCluster) {
    // Uniformly sample cube

    QVector<float> pointND1;
    QVector<float> pointND2;
    QVector<float> pointND3;
    QVector<float> pointND4;
    for (int i=0; i<dim; ++i){
      pointND1.append(distribution1(engine));
      pointND2.append(distribution2(engine)+150);
      pointND3.append(distribution3(engine) - 100);
      pointND4.append(distribution4(engine) + 100);
    }

    // Adjust the position of points
    pointND2[0] += 200;
    pointND3[1] += 300;
    pointND3[0] += 200;
    pointND4[0] += 250;

    if (dim == 2){
      pointND1.append(0.0f);
      pointND2.append(0.0f);
      pointND3.append(0.0f);
      pointND4.append(0.0f);
    }
    m_points.append(pointND1);
    m_points.append(pointND2);
    m_points.append(pointND3);
    m_points.append(pointND4);
    for (int i=0; i<4; ++i){
      m_colors.append({0, 0, 0});
    }

    count++;
  }
}

void ViewWidget::initializeGL()
{
  initializeOpenGLFunctions();
  glClearColor(0.2, 0.2, 0.2, 0.1);

//  glDepthFunc(GL_LEQUAL); // this change to " if (z = D(i,j)) "
  glEnable(GL_DEPTH_TEST);
  glPointSize(3.0f);
  glEnable(GL_POINT_SMOOTH);


  // set point program for 3-D points
  m_pointProgram.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                         "attribute highp vec4 vertex;\n"
//                                         "attribute mediump vec4 color;\n"
                                         "varying mediump vec4 vColor;\n"
                                         "uniform highp mat4 matrix;\n"
                                         "void main(void) { \n"
                                         "  gl_Position = matrix * vertex;\n"
                                         "  vColor = (vertex + vec4(1.0))/2.0;"
//                                         "  vColor = color;\n"
                                         "}");

  m_pointProgram.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                         "varying mediump vec4 vColor;\n"
                                         "void main(void) {\n"
                                         "  gl_FragColor = vColor;\n"
                                         "}");

  // link the source code from m_pointProgram to the program
  m_pointProgram.link();
}


void ViewWidget::paintGL() {
  glEnable(GL_DEPTH_TEST);

  QOpenGLShaderProgram program;
  program.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                  "attribute highp vec4 vertex;\n"
                                  "uniform highp mat4 matrix;\n"
                                  "void main(void)\n"
                                  "{\n"
                                  "   gl_Position = matrix * vertex;\n"
                                  "}");
  program.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                  "uniform mediump vec4 color;\n"
                                  "void main(void)\n"
                                  "{\n"
                                  "   gl_FragColor = color;\n"
                                  "}");
  program.link();


  int vertexLocation = program.attributeLocation("vertex");
  int matrixLocation = program.uniformLocation("matrix");
  int colorLocation = program.uniformLocation("color");


  //
  // Set eye position
  //
  QMatrix4x4 pmvMatrix;
//  pmvMatrix.ortho(rect());
//  pmvMatrix.ortho(-width()/2.0, width()/2.0, height()/2.0, -height()/2.0, -1, 1);
  pmvMatrix.perspective(440.0f, float(width())/height(), 1.0f, 10000.0f);

  if (m_spin) {
    pmvMatrix.lookAt({255, 255, 500}, {200, 100, 0}, {0, 1, 0});
    pmvMatrix.rotate(angleForTime(m_elapsedTimer.elapsed(), 15), {0.5f, 1.0f, 0.5f});
  } else {
    pmvMatrix.lookAt({255, 255, 500}, {255, 255, 0}, {0, 1, 0}); // ( eye, center, up), up: which direction should be pointed to up
  }


  program.setUniformValue(matrixLocation, pmvMatrix);

  m_pointProgram.bind();
  m_pointProgram.enableAttributeArray("vertex");
  m_pointProgram.enableAttributeArray("color");

  m_pointProgram.setUniformValue("matrix", pmvMatrix);
  m_pointProgram.setAttributeArray("vertex", m_points.constData(), 3); // tuple size: 3 (x,y,z)
  m_pointProgram.setAttributeArray("color", m_colors.constData(), 3); // tuple size: 3 (r,g,b)

  glDrawArrays(GL_POINTS, 0, m_points.count()/3);

  m_pointProgram.disableAttributeArray("vertex");
  m_pointProgram.disableAttributeArray("color");

  program.bind();
  program.enableAttributeArray(vertexLocation);
  program.setUniformValue(matrixLocation, pmvMatrix);

  //
  // Draw axes
  //
  GLfloat axisWidth = 1.0;
  GLfloat axisLenght = 500.0;

  // X Axis
  GLfloat const xAxis [] = {      0.0f, 0.0f, 0.0f,   // Vertex 1
                            axisLenght, 0.0f, 0.0f }; // Vertex 2

  glEnable(GL_LINE_SMOOTH);
  glLineWidth(axisWidth);

  program.setAttributeArray("vertex", xAxis, 3);
  program.setUniformValue("color", QColor(255, 0, 0, 255));
  glDrawArrays(GL_LINES, 0, 2);

  // Y Axis
  GLfloat const yAxis [] = {      0.0f, 0.0f, 0.0f,   // Vertex 1
                            0.0f, axisLenght, 0.0f }; // Vertex 2

  program.setAttributeArray("vertex", yAxis, 3);
  program.setUniformValue("color", QColor(0, 255, 0, 255));
  glDrawArrays(GL_LINES, 0, 2);

  // Z Axis
  GLfloat const zAxis [] = {      0.0f, 0.0f, 0.0f,   // Vertex 1
                            0.0f, 0.0f, axisLenght }; // Vertex 2

  program.setAttributeArray("vertex", zAxis, 3);
  program.setUniformValue("color", QColor(0, 0, 255, 255));
  glDrawArrays(GL_LINES, 0, 2);

  // Reset Line Width as 1
  glLineWidth(1.0);

  program.disableAttributeArray(vertexLocation);

  QPainter painter(this);
  painter.drawText(QRect(0, height()-20, width(), 20),
                   QString::number(m_fps, 'G', 4) + QString(" FPS"));

  m_frameCount++;

  if ( m_fpsTimer.elapsed() > 500 ) {
    m_fps = float(m_frameCount)/m_fpsTimer.restart() * 1000.0f;
    m_frameCount = 0;
  }
}

void ViewWidget::updateTurntable()
{
  // spin 1 degree each time
//  m_turntableAngle += 1.0f;

  update();
}
