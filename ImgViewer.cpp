#include "ImgViewer.hpp"

#include <iostream>
#include <cmath>
#include <algorithm>
#include <cfloat>
#include <iostream>
#include <fstream>
#include <ctime>
#include <cfloat>

#include <QtGui>
#include <QFileDialog>
#include <QMutexLocker>
#include <QFileDialog>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <png++/png.hpp>


using namespace std;

ImgViewer::ImgViewer(const vector<string> &filenames)
//    : QMainWindow(parent)
{
  // fill filename list
  files = filenames;
  currFileIdx = 0;
  scaleFactor = 1.0;

#ifdef BOOST_FILESYSTEM_FILESYSTEM_HPP
#ifdef BOOST_RE_REGEX_HPP
  vector<string> recImages;
  namespace fs = boost::filesystem;
  const boost::regex scanfilter( ".*\\.png" ); // "\\" is transformed into "\" at compile-time
  boost::smatch what; // match result
  for (vector<string>::const_iterator fi = filenames.begin(); fi != filenames.end(); ++fi) {
    fs::path file(*fi);
    if (fs::exists(file)) {
      if (fs::is_directory(file)) {
        vector<string> imageNames;
        fs::directory_iterator end;
        for (fs::directory_iterator iter(file); iter != end; ++iter) {
          fs::path currFile = *iter;
          if (!is_regular_file(currFile)) continue; // Skip if not a file //i->status()
          if (is_directory(currFile)) continue; // Skip if it is a directory // i->status()
          if (!boost::regex_match( currFile.filename().string(), what, scanfilter)) continue; // Skip if no match
          imageNames.push_back(currFile.string()); // File matches, store it
        }
        sort(imageNames.begin(), imageNames.end()); // sort alphabetically
        recImages.insert(recImages.end(), imageNames.begin(), imageNames.end());
      } else {
        recImages.push_back(*fi);
      }
    } // file exists
  } // for each file
  cout << endl << "found " << recImages.size() << " images" << flush;
  files = recImages;
#endif
#endif

  // create GUI elements
  lImage = new QLabel;
  lImage->setBackgroundRole(QPalette::Base);
  lImage->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  lImage->setScaledContents(true);

  saImage = new QScrollArea;
  saImage->setBackgroundRole(QPalette::Dark);
  saImage->setWidget(lImage);
  setCentralWidget(saImage);

  tPlayer = new QTimer();
  tPlayer->setSingleShot(false);
  tPlayer->setInterval(100);
  connect(tPlayer, SIGNAL(timeout()), this, SLOT(next()));

  aNext = new QAction(tr("Next"), this);
  aNext->setShortcut(QKeySequence::MoveToNextWord);
  aNext->setEnabled(files.size() > 1);
  connect(aNext, SIGNAL(triggered()), this, SLOT(next()));

  aPrev = new QAction(tr("Previous"), this);
  aPrev->setShortcut(QKeySequence::MoveToPreviousWord);
  aPrev->setEnabled(files.size() > 1);
  connect(aPrev, SIGNAL(triggered()), this, SLOT(prev()));

  aPlay = new QAction(tr("Play"), this);
  aPlay->setShortcut(tr("Space"));
  aPlay->setEnabled(files.size() > 1);
  aPlay->setCheckable(true);
  connect(aPlay, SIGNAL(triggered()), this, SLOT(play()));

  aZoomIn = new QAction(tr("Zoom &In (25%)"), this);
  aZoomIn->setShortcut(QKeySequence::ZoomIn);
  aZoomIn->setEnabled(false);
  connect(aZoomIn, SIGNAL(triggered()), this, SLOT(zoomIn()));

  aZoomOut = new QAction(tr("Zoom &Out (25%)"), this);
  aZoomOut->setShortcut(QKeySequence::ZoomOut);
  aZoomOut->setEnabled(false);
  connect(aZoomOut, SIGNAL(triggered()), this, SLOT(zoomOut()));

  aOrigSize = new QAction(tr("&Normal Size"), this);
  aOrigSize->setShortcut(tr("Ctrl+S"));
  aOrigSize->setEnabled(false);
  connect(aOrigSize, SIGNAL(triggered()), this, SLOT(zoomOrigSize()));

  aFitWindow = new QAction(tr("Fit Window to &Image"), this);
  aFitWindow->setShortcut(tr("Ctrl+I"));
  aFitWindow->setEnabled(false);
  aFitWindow->setCheckable(true);
  aFitWindow->setChecked(true);
  connect(aFitWindow, SIGNAL(triggered()), this, SLOT(adjustMainWindowSize()));

  this->addAction(aNext);
  this->addAction(aPrev);
  this->addAction(aPlay);
  this->addAction(aZoomIn);
  this->addAction(aZoomOut);
  this->addAction(aOrigSize);
  this->addAction(aFitWindow);
  this->setContextMenuPolicy(Qt::ActionsContextMenu);
  
  setWindowTitle(tr("PNG Distance Image Viewer"));
  resize(700, 500);

  while ((!loadImg()) && (currFileIdx < files.size()-1)) {
    ++currFileIdx;
  }
  aOrigSize->setEnabled(true);
  aZoomIn->setEnabled(true);
  aZoomOut->setEnabled(true);
  aFitWindow->setEnabled(true);
  lImage->adjustSize();
  adjustMainWindowSize();
}

ImgViewer::~ImgViewer()
{
}

void ImgViewer::scaleImage(double factor)
{
  Q_ASSERT(lImage->pixmap());
  scaleFactor *= factor;
  lImage->resize(scaleFactor * lImage->pixmap()->size());

  adjustScrollBar(saImage->horizontalScrollBar(), factor);
  adjustScrollBar(saImage->verticalScrollBar(), factor);

  aZoomIn->setEnabled(scaleFactor < 5.0);
  aZoomOut->setEnabled(scaleFactor > 0.2);
  adjustMainWindowSize();
}

void ImgViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
  scrollBar->setValue(int(factor * scrollBar->value() + ((factor - 1) * scrollBar->pageStep()/2)));
}

void ImgViewer::adjustMainWindowSize()
{
  if (aFitWindow->isChecked()) { // adjust window size
    saImage->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    saImage->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //cout << endl << "new size: " << lImage->size().width() << " x " << lImage->size().height() << flush;
    resize(lImage->size());
  } else {
    saImage->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    saImage->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  }
}


bool ImgViewer::loadImg()
{
  QString fn(files[currFileIdx].c_str());
  //cout << endl << "loading " << fn.toStdString() << "..." << flush;
  if (QFile(fn).exists()) {
    try {
      png::image<png::gray_pixel_16> png;
      png.read(files[currFileIdx]);
      short unsigned int maxVal = 0;
      short unsigned int minVal = USHRT_MAX;
      for (unsigned int row=0; row<png.get_height(); ++row) {
        for (unsigned int col=0; col<png.get_width(); ++col) {
          short unsigned int v = png[row][col];
          if ((v != 0) && (v != USHRT_MAX)) {
            maxVal = max(maxVal, v);
            minVal = min(minVal, v);
          }
        }
      }
      //cout << "values in range " << minVal << "..." << maxVal << flush;
      double minD = (double)minVal;
      double maxD = (double)maxVal;
      QImage image(png.get_width(),png.get_height(),QImage::Format_RGB16);
      for (unsigned int row=0; row<png.get_height(); ++row) {
        for (unsigned int col=0; col<png.get_width(); ++col) {
          short unsigned int v = png[row][col];
          if ((v < minVal) || (v > maxVal)) {
            image.setPixel(col, row, QColor::fromHsv(0, 0, 0).rgb() ); // H, S, V
          } else {
            double d = (double)max(minVal, min(maxVal,png[row][col]));// cut off value, so it is within [minVal...maxVal]
            d = 1.0 - (d - minD)/(maxD-minD); // scale to [0...1]
            image.setPixel(col, row, QColor::fromHsv(359.0 - d*d*d*359.0, 255.0, d*255.0).rgb() ); // H, S, V
          }
        }
      }
      //scaleFactor = 1.0;
      lImage->setPixmap(QPixmap::fromImage(image));
      setWindowTitle(fn);
      //lImage->adjustSize();
      adjustMainWindowSize();
    } catch (exception &e) {
      return false;
    }
    return true;
  }
  return false;
}

void ImgViewer::next()
{
  if (currFileIdx < files.size()-1) {
    ++currFileIdx;
    loadImg();
  } else {
    aPlay->setChecked(false);
    //tPlayer->stop();
  }
}

void ImgViewer::prev()
{
  if (currFileIdx > 0) {
    --currFileIdx;
    loadImg();
  }
}

void ImgViewer::play()
{
  if (aPlay->isChecked()) {
    tPlayer->start();
    next();
  } else {
    tPlayer->stop();
  }
}


void ImgViewer::zoomOrigSize()
{
  lImage->adjustSize();
  scaleFactor = 1.0;
  adjustMainWindowSize();
}

void ImgViewer::zoomIn()
{
  scaleImage(1.25);
}

void ImgViewer::zoomOut()
{
  scaleImage(0.8);
}



