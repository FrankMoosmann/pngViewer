/*!
    \author Frank Moosmann (<frank.moosmann@kit.edu>),
    \date   2011

    Copyright: Karlsruhe Institute of Technology (KIT)
               Institute of Measurement and Control Systems
               All rights reserved
               http://www.mrt.kit.edu
*/
#ifndef IMGVIEWER_H
#define IMGVIEWER_H

#include <vector>
#include <QtGui/QMainWindow>
//#include <QtGui/QLayout>
//#include <QtGui/QScrollArea>

class QLabel;
class QScrollArea;
class QScrollBar;


class ImgViewer : public QMainWindow
{
  Q_OBJECT

public:
  ImgViewer(const std::vector<std::string> &files);
  ~ImgViewer();

//protected:
//  void resizeEvent(QResizeEvent *event);

private:

  std::vector<std::string> files;
  unsigned int currFileIdx;
  
  double scaleFactor;
  QLabel *lImage;
  QScrollArea *saImage;
  QTimer *tPlayer;

  QAction *aPlay;
  QAction *aNext;
  QAction *aPrev;
  QAction *aZoomIn;
  QAction *aZoomOut;
  QAction *aOrigSize;
  QAction *aFitWindow;

  void scaleImage(double factor);
  void adjustScrollBar(QScrollBar *scrollBar, double factor);

private slots:
  bool loadImg(); //!< returns true on success
  void next();
  void prev();
  void play();
  void zoomOrigSize();
  void zoomIn();
  void zoomOut();
  void adjustMainWindowSize();
};

#endif // IMGVIEWER_H
