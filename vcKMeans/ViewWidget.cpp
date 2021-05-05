#include "ViewWidget.h"

#include <QOpenGLShaderProgram>
#include <QPainter>
#include <QtMath>
#include <QTimer>
#include <chrono>
#include <random>

ViewWidget::ViewWidget(QWidget *parent, Qt::WindowFlags f) : QOpenGLWidget(parent, f)
{
  auto turntableTimer = new QTimer(this);
  turntableTimer->callOnTimeout(this, &ViewWidget::updateTurntable);

  turntableTimer->start(1000.0/1.0); // 30 ms

  m_elapsedTimer.start();

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

  m_fpsTimer.start();
}

float ViewWidget::angleForTime(qint64 msTime, float secondsPerRotation) const
{
  float millisecondsPerRotation {secondsPerRotation*1000.0f};
  float t {msTime/millisecondsPerRotation};

  return (t-qFloor(t)) * 360.0f;
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

//
// Return interleaved xyz points for polygon centered at x, y, z with given
// radius and number of sides
//
QVector<GLfloat> createPolygon(float x, float y, float z, float radius,
                               int sides)
{
  QVector<GLfloat> result;

  float angle = -M_PI_2;
  float step = 2.0f * M_PI/sides;

  for(int i = 0; i < sides; ++i)
  {
    result.push_back(x + radius * qCos(angle));
    result.push_back(y + radius * qSin(angle));
    result.push_back(z);
    angle += step;
  }

  return result;
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

  static GLfloat const triangleVertices[] = {
    60.0f,  10.0f,  1.0f,
    110.0f, 110.0f, 10.0f,
    10.0f,  110.0f, 50.0f
  };


  //
  // Set eye position
  //
  QMatrix4x4 pmvMatrix;
//  pmvMatrix.ortho(rect());
//  pmvMatrix.ortho(-width()/2.0, width()/2.0, height()/2.0, -height()/2.0, -1, 1);
  pmvMatrix.perspective(40.0f, float(width())/height(), 1.0f, 10000.0f);
  pmvMatrix.lookAt({0, 8, 4}, {0, 0, 0}, {0, 1, 0});
  pmvMatrix.rotate(angleForTime(m_elapsedTimer.elapsed(), 15), {0.0f, 1.0f, 0.0f});

  program.setUniformValue(matrixLocation, pmvMatrix);




  m_pointProgram.bind();
  m_pointProgram.enableAttributeArray("vertex");
  m_pointProgram.enableAttributeArray("color");

  m_pointProgram.setUniformValue("matrix", pmvMatrix);
  m_pointProgram.setAttributeArray("vertex", m_points.constData(), 3); // tuple size: 3 (x,y,z)
  m_pointProgram.setAttributeArray("color", m_colors.constData(), 3); // tuple size: 3 (r,g,b)

  glDrawArrays(GL_POINTS, 0, m_points.count()/3*0.05f);

  m_pointProgram.disableAttributeArray("vertex");
  m_pointProgram.disableAttributeArray("color");

//  program.setAttributeArray(vertexLocation, triangleVertices, 3);

//  // Draw red pentagon
//  auto pentagon = createPolygon(60, 60, 0, 50, 5);
//  program.setAttributeArray(vertexLocation, pentagon.constData(), 3);
//  program.setUniformValue(colorLocation, QColor(255, 0, 0, 255));
//  glDrawArrays(GL_POLYGON, 0, pentagon.count()/3);

//  // Draw green octagon
//  auto saveMatrix = pmvMatrix;
//  pmvMatrix.translate(60, 60, 0);
//  pmvMatrix.rotate(45.0f, {0, 1, 0});
//  pmvMatrix.translate(-60, -60, 0);
//  auto octagon = createPolygon(60, 60, -0.5, 60, 8);
//  program.setAttributeArray(vertexLocation, octagon.constData(), 3);
//  program.setUniformValue(colorLocation, QColor(0, 255, 0, 255));
//  program.setUniformValue(matrixLocation, pmvMatrix);
//  glDrawArrays(GL_POLYGON, 0, octagon.count()/3);

//  pmvMatrix = saveMatrix;

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
