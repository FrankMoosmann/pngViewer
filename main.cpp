/*!
   \brief Program to display distance-png-images

   \author Frank Moosmann (<frank.moosmann@kit.edu>) 
**/

#include <iostream>
#include <vector>

#include <QApplication>

#include "ImgViewer.hpp"

 
using namespace std;

int main( int argc, char** argv )
{
  // ------------------------------------------------
  // -------------- parse arguments -----------------
  // ------------------------------------------------
  if ((argc == 1) || (string(argv[1]) == "-h") || (string(argv[1]) == "--help")) {
    cerr << "Usage: " << argv[0] << " <list of files/directories>" << endl;
    return 1;
  }
  vector<string> files;
  for (int i=1; i<argc; ++i) {
    // TODO: if isdir(argv[i]) scanDir(argV[i])
    files.push_back(string(argv[i]));
  }

  // -------------------------------------------
  // ---------------   run GUI   ---------------
  // -------------------------------------------
  QApplication app(argc, argv);
  ImgViewer imageViewer(files);
  imageViewer.show();
  int r = app.exec();
  cout << endl;
  return r;
}





