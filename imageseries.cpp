
#include "gngviewer.h"
#include "gngapp.h"
#include "libgng/gng.h"
#include "libgng/imagesource.h"
#include "libgng/node.h"

#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <QDebug>
#include <QDir>

namespace po=boost::program_options;
using std::string;
using namespace GNG;
    
void setUpdateInterval(int steps); /**< Emit signal updated() once per this number of steps */
    
typedef struct s_popts {
  int delay;
  int updateInterval;
  string imagesDir;
  float winnerLearnRate;
  float neighborLearnRate;
  float maxEdgeColorDiff;
  int maxEdgeAge;
  int nodeInsertionDelay;
  float targetError;
  float errorReduction;
  float insertErrorReduction;
  int totalIterations;
} ProgOpts;

bool parse_args(int argc, char* argv[], ProgOpts& popts);

int main(int argc, char* argv[]) {
  // Create our QApplication object. Needed for the gui and for threading
  GngApp app(argc, argv);
  
  // get command-line arguments
  ProgOpts popts;
  if(!parse_args(argc, argv, popts))
    exit(1);

  // Create the GNG object with bounds of 0 and 1.
  GrowingNeuralGas gng(5);

  // set command-line parameters
  gng.setDelay(popts.delay);
  gng.setWinnerLearnRate(popts.winnerLearnRate);
  gng.setNeighborLearnRate(popts.neighborLearnRate);
  gng.setMaxEdgeAge(popts.maxEdgeAge);
  gng.setMaxEdgeColorDiff(popts.maxEdgeColorDiff);
  gng.setNodeInsertionDelay(popts.nodeInsertionDelay);
  gng.setTargetError(popts.targetError);
  gng.setErrorReduction(popts.errorReduction);
  gng.setInsertErrorReduction(popts.insertErrorReduction);
  gng.setUpdateInterval(popts.updateInterval);

 
  QDir imagesDir(QString::fromStdString(popts.imagesDir));
  foreach(QString imageFile, imagesDir.entryList(QDir::Files)) {
    qDebug() << "image: " << imageFile;
    app.addImage(QImage(imagesDir.absoluteFilePath(imageFile)));
  }
  
  
  // The GngViewer provides the window in which we can see the results of the GNG/source image
  GngViewer view;
    
  // The ImageGenerator provides the source points for the gng (similar to the distribution)
  QImage firstImg(imagesDir.absoluteFilePath(imagesDir.entryList(QDir::Files).first()));
  ImageSource generator(firstImg);
  QPixmap firstImgPixmap = QPixmap::fromImage(firstImg);
  view.setSource(firstImgPixmap);
  
  view.setSize(generator.width(), generator.height());
  
  app.setGenerator(&generator);
  app.setViewer(&view);

  // Give the view a gng to visualize.
  view.setGng(&gng);
  view.show();
  
  
  // Give the GNG its way of generating points
  gng.setPointGenerator(&generator);

  // Run the GNG during idle processing for 10,000 cycles
  gng.stopAt(popts.totalIterations);
  gng.start();

  // Execute the Qt mainloop. Needed for widgets to update themselves/for events to happen
  app.runMovie();
  // This area is never reached by the program
}

bool parse_args(int argc, char* argv[], ProgOpts& popts){
   string configFile;
   po::options_description desc("Allowed options");
   desc.add_options()
     ("help,h", "Show this message")
     ("config,c", po::value<string>(&configFile), "Config file to read options from")
     ("imagesDir,p", po::value<string>(&popts.imagesDir), "Path to a directory that has images in it")
     ("delay,d", po::value<int>(&popts.delay)->default_value(1), "Add a n millisecond delay to each step.")
     ("updateInterval,u", po::value<int>(&popts.updateInterval)->default_value(50), "Emit signal updated() once per this number of steps")
     ("winnerLearnRate,w", po::value<float>(&popts.winnerLearnRate)->default_value(0.1), "Used to adjust closest unit towards input point")
     ("neighborLearnRate,n", po::value<float>(&popts.neighborLearnRate)->default_value(0.01), "Used to adjust other neighbors towards input point")
     ("maxEdgeColorDiff,f", po::value<float>(&popts.maxEdgeColorDiff)->default_value(0.1), "Edges not created if color difference between nodes is above value")
     ("maxEdgeAge,m", po::value<int>(&popts.maxEdgeAge)->default_value(50), "Edges older than maxAge are removed")
     ("nodeInsertionDelay,i", po::value<int>(&popts.nodeInsertionDelay)->default_value(100), "Min steps before inserting a new node")
     ("targetError,e", po::value<float>(&popts.targetError)->default_value(0.001), "Continue inserting nodes until the average error has reached this threshold")
     ("errorReduction,r", po::value<float>(&popts.errorReduction)->default_value(0.1), "All errors are reduced by this amount each GNG step")
     ("insertErrorReduction,s", po::value<float>(&popts.insertErrorReduction)->default_value(0.5), "Reduce new unit's error by this much")
     ("totalIterations,t", po::value<int>(&popts.totalIterations)->default_value(100000), "Run this many iterations in total");
   po::variables_map vm;
   po::store(po::parse_command_line(argc, argv, desc), vm);
   po::notify(vm);
   if(vm.count("config")){
     std::ifstream ifs(vm["config"].as<string>().c_str());
     store(parse_config_file(ifs, desc), vm);
     notify(vm);
   }
   po::store(po::parse_command_line(argc, argv, desc), vm);
   po::notify(vm);
   if (vm.count("help") || !vm.count("imagesDir")){
     std::cout << desc;
	   return false;
   }
   return true;
}
