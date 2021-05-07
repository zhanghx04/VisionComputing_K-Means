#include "ViewWidget.h"

#include <QApplication>
#include <QOpenGLShaderProgram>
#include <QtGlobal>
#include <QPainter>
#include <QtMath>
#include <QTimer>
#include <QString>
#include <QFile>
#include <chrono>
#include <random>
#include <cstdlib>
#include <algorithm>
#include <stdio.h>

#include <QDebug>


ViewWidget::ViewWidget(QWidget *parent, Qt::WindowFlags f) : QOpenGLWidget(parent, f)
{
  auto turntableTimer = new QTimer(this);
  turntableTimer->callOnTimeout(this, &ViewWidget::updateTurntable);

  // control the fps
  turntableTimer->start(1000.0/60.0); // 30 ms

  m_elapsedTimer.start();

//  samplePointsGenteration();

  //////////////////////////////
  /// IF read data from file ///
  //////////////////////////////
  m_isTXTfile = true;

  qDebug() << "[INFO] Data loading...";
  if (m_isTXTfile) {
    dataGenerateFromFile("a.txt");
  } else {
    dataGeneration(100, 3); // ( samplePerCluster, dimension)
  }


  /*
   * doKmeans(k, distance_function, center_initial_method)
   * distance_function:
   *    1. l1-norm
   *    2. l2-norm
   *    3. l-infinity-norm
   * center_initial_method:
   *    1. random_real
   *    2. random_sample
   *    3. k-means++
   */
//  doKmeans(4, "l2-norm", "random_real");



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
  m_dim = dim;
  qDebug() << "[INFO] Dimension of Data:" << m_dim << "Dimension";

  if (m_dim > 2) {
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
    for (int i=0; i<m_dim; ++i){
      pointND1.append(distribution1(engine));
      pointND2.append(distribution2(engine) + 150);
      pointND3.append(distribution3(engine) - 100);
      pointND4.append(distribution4(engine) + 100);
    }

    // Adjust the position of points
    pointND2[0] += 200;
    pointND3[1] += 300;
    pointND3[0] += 200;
    pointND4[0] += 250;

    if (m_dim == 2){
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
      m_colors.append({255, 255, 255}); // white
    }

    count++;
  }
  m_totalSample = samplesPerCluster*4;
}

void ViewWidget::dataGenerateFromFile(QString filename)
{
  m_folder = "/Users/haoxiangzhang/Documents/GitHub/VisionComputing-KMeasn/vcKMeans/dataset/";

  QFile inputFile(m_folder+filename);
  if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)){
    qDebug() << "[ERROR] File" << filename << "is not exist in current directory...";
    qDebug() << "[SYSTEM] Application Terminated.";
    return;
  }

  // Reading files
  QTextStream in(&inputFile);
  // save total sample number
  m_totalSample = in.readLine().toInt();
  // save k
  m_dim = in.readLine().toInt();

  // read 2d points
  while (!in.atEnd()){
    QString line = in.readLine();
    float x {line.split(" ")[0].toFloat()};
    float y {line.split(" ")[1].toFloat()};
    m_points.append({x, y, 0.0});
    m_colors.append({255, 255, 255}); // add color for each read point.
  }
  inputFile.close();
}

void ViewWidget::initialCenters()
{
  /*
   * Initialization centers
   *  distance type:
   *    1 - random real:    any real number x, y location for the seed within the sample space
   *    2 - random sample:  randomly select one of the observations
   *    3 - K-Means++ distance based (D^2) careful seeding
   */
  qDebug() << "[INFO] Start to initialize centers for each cluster...";

  if (m_cent_method == 1) {
    // 1 - random real:    any real number x, y location for the seed within the sample space
    // get range of the sample
    QVector<float> range_x {0, 0};
    QVector<float> range_y {0, 0};
    QVector<float> range_z {0, 0};
    for (int i=0; i<m_totalSample; ++i) {
      range_x[0] = qMin(range_x[0], m_points[i*3]);
      range_x[1] = qMax(range_x[1], m_points[i*3]);
      range_y[0] = qMin(range_y[0], m_points[i*3+1]);
      range_y[1] = qMax(range_y[1], m_points[i*3+1]);
      range_z[0] = qMin(range_z[0], m_points[i*3+2]);
      range_z[1] = qMax(range_z[1], m_points[i*3+2]);
    }

    // generating centers
    for (int i=0; i<m_k; ++i) {
      float x = range_x[0] + rand() % int(range_x[1]-range_x[0]);
      float y = range_y[0] + rand() % int(range_y[1]-range_y[0]);
      float z {};
      if (m_spin) {
        z = range_z[0] + rand() % int(range_z[1]-range_z[0]);
      } else {
        z = 0.0f;
      }
      m_centers.append({x, y, z});
    }
  }

  if (m_cent_method == 2) {
    // 2 - random sample:  randomly select one of the observations
    for (int i=0; i<m_k; ++i){
      int idx = rand() % m_totalSample-1;
      m_centers.append(m_points.mid(idx, 3));
    }

  }

  if (m_cent_method == 3) {
    // 3 - K-Means++ distance based (D^2) careful seeding
    // TODO: implement K-Means++

  }

  // Generate random color for centers
  qDebug() << "[INFO] Generating different color for centers...";
  for (int i=0; i<m_k; ++i){
    float r {1.0f * (rand() % 255) / 255};
    float g {1.0f * (rand() % 255) / 255};
    float b {1.0f * (rand() % 255) / 255};
    m_centerColors.append({r, g, b});
  }

  qDebug() << "[INFO] Centers initialized.";
}

void ViewWidget::check_params(int k, QString distance_function, QString center_initialize_method)
{
  // set number of cluster
  m_k = k;
  qDebug() << "[INFO] Number of Cluster:" << m_k;

  if ( QString::compare(distance_function, "l1-norm", Qt::CaseInsensitive) == 0 ) {
    m_dist_method = 1;
  } else if ( QString::compare(distance_function, "l2-norm", Qt::CaseInsensitive) == 0 ) {
    m_dist_method = 2;
  } else if ( QString::compare(distance_function, "l-infinity-norm", Qt::CaseInsensitive) == 0 ) {
    m_dist_method = 3;
  } else {
    qDebug() << "[ERROR] Please select Distance Function:";
    qDebug() << "              1) l1-norm";
    qDebug() << "              2) l2-norm";
    qDebug() << "              3) l-infinity-norm";
    qDebug() << "[SYSTEM] Program terminated...";
    QApplication::exec();
  }
  qDebug() << "[INFO] Distance Function:" << distance_function;


  if ( QString::compare(center_initialize_method, "random_real", Qt::CaseInsensitive) == 0 ) {
    m_cent_method = 1;
  } else if ( QString::compare(center_initialize_method, "random_sample", Qt::CaseInsensitive) == 0 ) {
    m_cent_method = 2;
  } else if ( QString::compare(center_initialize_method, "k-means++", Qt::CaseInsensitive) == 0 ) {
    m_cent_method = 3;
  } else {
    qDebug() << "[ERROR] Please select Center Initial Type:";
    qDebug() << "              1) random_real";
    qDebug() << "              2) random_sample";
    qDebug() << "              3) k-means++";
    qDebug() << "[SYSTEM] Program terminated...";
    QApplication::exec();
  }
  qDebug() << "[INFO] Center Initial Type:" << center_initialize_method;
}

int ViewWidget::find_closest(QVector<float> point)
{
  /*
   *  compare the distance between point and each center and return the closest center
   *
   *  distance type:
   *    1 - L1 norm
   *    2 - L2 norm
   *    3 - L-infinity norm
   */
  QVector<float> dists;
  // go through each center and get the distance
  for (int i=0; i<m_centerColors.length()/3; ++i){
    float dist {0.0};
    QVector<float> center = m_centers.mid(i*3, 3);

    // get difference
    center[0] = qAbs(center[0] - point[0]);  // x
    center[1] = qAbs(center[1] - point[1]);  // y
    center[2] = qAbs(center[2] - point[2]);  // z

    if (m_dist_method == 1){
      //
      dist = center[0] + center[1] + center[2];
    }

    if (m_dist_method == 2){
      dist = qSqrt(qPow(center[0], 2) + qPow(center[1], 2) + qPow(center[2], 2));
    }

    if (m_dist_method == 3) {
      dist = *std::max_element(center.constBegin(), center.constEnd());
    }

    dists.append(dist);
  }

  // return the index of minimum distance.
  return dists.indexOf(*std::min_element(dists.constBegin(), dists.constEnd()));
}

void ViewWidget::doKmeans(int k, QString distance_function, QString center_initialize_method)
{
  // check parameters to see if the input is invalid
  check_params(k, distance_function, center_initialize_method);

  //
  // K-Means Algorithm
  //
  qDebug() << "\n[INFO] Start K-Means Algorithm!";

  // Initial centers
  initialCenters();

  QVector<float> centers_ref = m_centers;

  bool if_continue = true;
  int account = 0;
  QVector<int> closest;

  // go through each point and find their closest center
  for (int i=0; i<m_totalSample; ++i) {
    int idx = find_closest(m_points.mid(i*3, 3));
    closest.append(idx); // save the index of center for each point
  }

  // update new center


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
                                         "attribute mediump vec4 color;\n"
                                         "varying mediump vec4 vColor;\n"
                                         "uniform highp mat4 matrix;\n"
                                         "void main(void) { \n"
                                         "  gl_Position = matrix * vertex;\n"
//                                         "  vColor = (vertex + vec4(1.0))/2.0;"
                                         "  vColor = color;\n"
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
    if (m_isTXTfile) {
      pmvMatrix.lookAt({0, 0, 10}, {8, 8, 0}, {0, 1, 0}); // this is for given txt data
    } else {
      pmvMatrix.lookAt({255, 255, 500}, {255, 255, 0}, {0, 1, 0}); // ( eye, center, up), up: which direction should be pointed to up
    }
  }

  program.setUniformValue(matrixLocation, pmvMatrix);

  m_pointProgram.bind();
  m_pointProgram.enableAttributeArray("vertex");
  m_pointProgram.enableAttributeArray("color");

  m_pointProgram.setUniformValue("matrix", pmvMatrix);

  // draw points
  m_pointProgram.setAttributeArray("vertex", m_points.constData(), 3); // tuple size: 3 (x,y,z)
  m_pointProgram.setAttributeArray("color", m_colors.constData(), 3); // tuple size: 3 (r,g,b)
  glDrawArrays(GL_POINTS, 0, m_points.count()/3);

  // draw centers
  glPointSize(10.0f);
  m_pointProgram.setAttributeArray("vertex", m_centers.constData(), 3); // tuple size: 3 (x,y,z)
  m_pointProgram.setAttributeArray("color", m_centerColors.constData(), 3); // tuple size: 3 (r,g,b)
  glDrawArrays(GL_POINTS, 0, m_centers.count()/3);
  glPointSize(3.0f);

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
