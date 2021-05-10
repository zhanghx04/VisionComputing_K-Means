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
#include <QThread>
#include <QTextStream>
#include <math.h>

#include <QPainter>

#include <QDebug>


ViewWidget::ViewWidget(QWidget *parent, Qt::WindowFlags f) : QOpenGLWidget(parent, f), m_zoom(1)
{
  auto turntableTimer = new QTimer(this);
  turntableTimer->callOnTimeout(this, &ViewWidget::updateTurntable);

  // control the fps
  turntableTimer->start(1000.0/60.0); // 30 ms

  m_elapsedTimer.start();

  // seeding the rand() function
  srand(time(NULL));

//  samplePointsGenteration();

  m_fpsTimer.start();
}

int ViewWidget::zoom() const
{
  return m_zoom;
}

float ViewWidget::turn() const
{
  return m_angle;
}

float ViewWidget::vertical() const
{
  return m_v_direct;
}

float ViewWidget::horizontal() const
{
  return m_h_direct;
}

float ViewWidget::speed() const
{
  return m_speed;
}

float ViewWidget::energy() const
{
  return m_energy;
}


float ViewWidget::angleForTime(qint64 msTime, float secondsPerRotation) const
{
  float millisecondsPerRotation {secondsPerRotation*1000.0f};
  float t {msTime/millisecondsPerRotation};

  return (t-qFloor(t)) * 360.0f;
}

void ViewWidget::dataReceive(int k, int speed, int samplePerCluster, int dim,
                             QString dist_method, QString cent_method,
                             float point_size, float center_size, QString filename,
                             bool if_file_data, int num_iter, bool if_step,
                             float the_zoom, float v_direct, float h_direct)
{
  m_k = k;
  m_timer = speed;
  m_samplePerCluster = samplePerCluster;
  m_dim = dim;
  if (m_dim > 2) {
    m_spin = true;
  }

  m_dist_m = dist_method;
  m_cent_m = cent_method;

  m_pointSize = point_size;
  m_centerSize = center_size;
  m_filename = filename;

  m_isTXTfile = if_file_data;
  m_num_iter = num_iter;
  m_ifStep = if_step;

  m_zoom = the_zoom;
  m_v_direct = v_direct;
  m_h_direct = h_direct;

  initialData();


  qDebug() << k << speed << samplePerCluster <<
              dim << dist_method << cent_method <<
              point_size << center_size <<
              filename << if_file_data <<
              if_step << m_zoom;

  qDebug() << "[INFO] Data loading...";
  //////////////////////////////
  /// IF read data from file ///
  //////////////////////////////
  if (m_isTXTfile) {
    /*
     * data file name:
     *        a.txt
     *        b.txt
     *        c.txt
     *        d.txt
     *        d_ring.txt
     *        e_corners_1.txt
     *        f_corners_2.txt
     *        g_overlapping.txt
     */
    dataGenerateFromFile(m_filename);
  } else {
    dataGeneration(m_samplePerCluster, m_dim); // ( samplePerCluster, dimension)
  }
  qDebug() << "[INFO] Total number of samples:" << m_totalSample;
  qDebug() << "[INFO]       Dimension of Data:" << m_dim << "Dimension";

  /*
   * doKmeans(k, distance_function, center_initial_method)
   * distance_function:
   *    1. l1-norm
   *    2. l2-norm
   *    3. l-infinity-norm
   *
   * center_initial_method:
   *    1. random_real
   *    2. random_sample
   *    3. k-means++
   */
  doKmeans(m_k, m_dist_m, m_cent_m);
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

void ViewWidget::initialData()
{
  // clean m_points, m_colors, m_centers, and m_centerColors
  m_points.clear();
  m_colors.clear();
  m_centers.clear();
  m_centerColors.clear();

  account = 0;
  m_angle = 0.0f;
  m_v_direct = 0.0f;
  m_h_direct = 0.0f;
}

void ViewWidget::dataGeneration(int samplesPerCluster, int dim)
{
  m_dim = dim;

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
  for (int i=0; i<m_totalSample; ++i){
    m_ave_point[0] += m_points[i*3 + 0];
    m_ave_point[1] += m_points[i*3 + 1];
    m_ave_point[2] += m_points[i*3 + 2];
  }
  m_ave_point[0] /= m_totalSample;
  m_ave_point[1] /= m_totalSample;
  m_ave_point[2] /= m_totalSample;
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

  int data_scale {1};

  if (filename == "e_corners_1.txt" || filename == "f_corners_2.txt" || filename == "g_overlapping.txt") {
    data_scale = 100;
  }

  // read 2d points
  while (!in.atEnd()){
    QString line = in.readLine();
    float x {line.split(" ")[0].toFloat()/data_scale};
    float y {line.split(" ")[1].toFloat()/data_scale};
    m_points.append({x, y, 0.0f});
    m_colors.append({255, 255, 255}); // add color for each read point.

    m_ave_point[0] += x;
    m_ave_point[1] += y;
  }
  inputFile.close();

  m_ave_point[0] /= m_totalSample;
  m_ave_point[1] /= m_totalSample;
}

void ViewWidget::saveData(QString filename)
{
  // save file
  QFile ffile("/Users/haoxiangzhang/Documents/GitHub/VisionComputing-KMeasn/vcKMeans/dataset/"+filename);
  if (ffile.open(QIODevice::ReadWrite)) {
    QTextStream stream(&ffile);
    stream << m_totalSample << Qt::endl;
    stream << m_dim << Qt::endl;
    for (int i=0; i<m_totalSample; ++i){
      stream << m_points[i*3] << " " << m_points[i*3+1] << " " << m_points[i*3+2] << Qt::endl;
    }
    ffile.close();
  }
}

float ViewWidget::dist2(QVector<float> point1, QVector<float> point2)
{
  /* This function return the square euclidean distance between two points */
  float x = point1[0] - point2[0];
  float y = point1[1] - point2[1];
  float z = point1[2] - point2[2];
  return qPow(x, 2) + qPow(y, 2) + qPow(z, 2);
}

float ViewWidget::nearestDistance(QVector<float> point, QVector<float> centers)
{
  /* This function returns the diatance of the cluster centroid nearest to the
      data point passed to this function */
  float d;
  float min_d{HUGE_VAL};

  for (int i=0; i<m_k; ++i) {
    d = dist2(point, centers.mid(i*3, 3));
    if (d < min_d){
      min_d = d;
    }
  }

  return min_d;
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
    /* 1 - random real:    any real number x, y location for the seed within the sample space */
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
      float z {0.0f};
      if (m_spin) {
        z = range_z[0] + rand() % int(range_z[1]-range_z[0]);
      }
      m_centers.append({x, y, z});
    }
  }

  if (m_cent_method == 2) {
    /* 2 - random sample:  randomly select one of the observations */
    for (int i=0; i<m_k; ++i){
      int idx = rand() % m_totalSample-1;
      m_centers.append(m_points.mid(idx*3, 3));
    }

  }

  if (m_cent_method == 3) {
    /* 3 - K-Means++ distance based (D^2) careful seeding */

    QVector<float> distances;

    // randomly select cluster centroids in data points
    m_cent_method = 2;
    initialCenters();
    m_cent_method = 3;

    // select rest of clusters
    for (int i=1; i<m_k; ++i) {
      /* for each data point find the nearest centroid, save its distance in the
         distance array, then add it to the sum of total distance. */
      float sum {0.0f};

      for (int j=0; j<m_totalSample; ++j) {
        distances.append(nearestDistance(m_points.mid(j*3, 3), m_centers));
        sum += distances[j];
      }


      // find a random distance within the span of the total distance.
      sum = sum * float(rand()/(RAND_MAX-1));

      // Assign the centroids, the point with the largest distance
      for (int j=0; j<m_totalSample; ++j) {
        sum -= distances[j];
        if (sum <= 0) {
          m_centers[i*3 + 0] = m_points[j*3 + 0];
          m_centers[i*3 + 1] = m_points[j*3 + 1];
          m_centers[i*3 + 2] = m_points[j*3 + 2];
          break;
        }
      }
    }
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
  qDebug() << "[INFO]       Number of Cluster:" << m_k;

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
  qDebug() << "[INFO]       Distance Function:" << distance_function;


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
  qDebug() << "[INFO]     Center Initial Type:" << center_initialize_method;
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
  for (int i=0; i<m_k; ++i){
    float dist {0.0};
    QVector<float> center = m_centers.mid(i*3, 3);

    // get difference
    center[0] = qAbs(center[0] - point[0]);  // x
    center[1] = qAbs(center[1] - point[1]);  // y
    center[2] = qAbs(center[2] - point[2]);  // z



    if (m_dist_method == 1){
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

  ttime->callOnTimeout(this, &ViewWidget::k_means_iter);
  ttime->start(m_timer);
}

void ViewWidget::setZoom(int zoom)
{
  // Only update on change
  if(m_zoom != zoom)
  {
    // Ensure zoom is 1 or greater
    m_zoom = qMax(1, zoom);
  }

  qDebug() << m_zoom;
}

void ViewWidget::setTurn(float angle)
{
  if (angle < 0) {
    m_angle = 360 + angle;
  } else {
    m_angle = angle - 360.0;
  }

  qDebug() << m_angle;
}

void ViewWidget::setMove(float step, int direction)
{
  // vertical
  if (direction == 1) m_v_direct = step;
  // horizontal
  if (direction == 2) m_h_direct = step;
}

void ViewWidget::initializeGL()
{
  initializeOpenGLFunctions();
  glClearColor(0.2, 0.2, 0.2, 0);

//  glDepthFunc(GL_LEQUAL); // this change to " if (z = D(i,j)) "
  glEnable(GL_DEPTH_TEST);
  glPointSize(m_pointSize);
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
//  qDebug() << m_points;
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
//  int colorLocation = program.uniformLocation("color");

  //
  // Set eye position
  //
  QMatrix4x4 pmvMatrix;
//  pmvMatrix.ortho(rect());
//  pmvMatrix.ortho(-width()/2.0, width()/2.0, height()/2.0, -height()/2.0, -1, 1);
  pmvMatrix.perspective(440.0f, float(width())/height(), 1.0f, 10000.0f);

  if (m_spin) {
    pmvMatrix.lookAt({m_h_direct, m_v_direct, m_zoom}, {50, 150, 20}, {0, 1, 0});
//    pmvMatrix.rotate(angleForTime(m_elapsedTimer.elapsed(), 25), {0.5f, 1.0f, 0.5f});
    pmvMatrix.rotate(m_angle, {0.5f, 1.0f, 0.5f});
  } else {
    if (m_isTXTfile) {
      pmvMatrix.lookAt({m_h_direct, m_v_direct, m_zoom}, {0, 0, 0}, {0, 1, 0}); // this is for given txt data
    } else {
      pmvMatrix.lookAt({m_h_direct, m_v_direct, m_zoom}, {255, 255, 0}, {0, 1, 0}); // ( eye, center, up), up: which direction should be pointed to up
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
  glPointSize(m_centerSize);
  m_pointProgram.setAttributeArray("vertex", m_centers.constData(), 3); // tuple size: 3 (x,y,z)
  m_pointProgram.setAttributeArray("color", m_centerColors.constData(), 3); // tuple size: 3 (r,g,b)
  glDrawArrays(GL_POINTS, 0, m_centers.count()/3);
  glPointSize(m_pointSize);

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

void ViewWidget::k_means_iter()
{
  record_centers.append(m_centers);
  record_centerColor.append(m_centerColors);
  record_pointColor.append(m_colors);

  QVector<int> closest; // save the closest center idx for each point

  // go through each point and find their closest center
  for (int i=0; i<m_totalSample; ++i) {
    int idx = find_closest(m_points.mid(i*3, 3));
    closest.append(idx); // save the index of center for each point

    // update the color of point
    m_colors[i*3 + 0] = m_centerColors[idx*3 + 0]/3*2;
    m_colors[i*3 + 1] = m_centerColors[idx*3 + 1]/3*2;
    m_colors[i*3 + 2] = m_centerColors[idx*3 + 2]/3*2;

  }



  // update new center
  for (int i=0; i<m_k; ++i) { // go each center
    int acc {0}; // how many points that current center is the closest one
    QVector<float> total {0, 0, 0}; // for sum all points

    for (int j=0; j<m_totalSample; ++j) {
      int p = closest.at(j);
      if (p == i) {
        total[0] += m_points[j*3];      // x
        total[1] += m_points[j*3 + 1];  // y
        total[2] += m_points[j*3 + 2];  // z

        acc++;
      }
    }

    if (acc != 0) {   // in case if acc = 0, then the center updated to nan
      m_centers[i*3 + 0] = total[0]/acc;
      m_centers[i*3 + 1] = total[1]/acc;
      m_centers[i*3 + 2] = total[2]/acc;
    } else {
      m_centers[i*3 + 0] = m_ave_point[0];
      m_centers[i*3 + 1] = m_ave_point[1];
      m_centers[i*3 + 2] = m_ave_point[2];
    }
  }

  account++;
  qDebug() << "[K-MEANS] iteration -" << account;

  // check if account to given iteration
  if (m_ifStep) {
    if (account == m_num_iter) {
      emit sendResult(11, 22);
      ttime->stop();
      return;
    }
  }
  // check if centers are not move
  if (m_centers == centers_ref) {
    emit sendResult(11, 22);
//    qDebug() << "[INFO] K-Means Algorithm finished.";
    ttime->stop();
    return;
  }

  centers_ref = m_centers;

  update();
}


