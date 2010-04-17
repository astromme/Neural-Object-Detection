
#include "gngviewer.h"
#include "libgng/gng.h"
#include "libgng/imagegenerator.h"
#include "libgng/node.h"

#include <boost/program_options.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <QApplication>
#include <QDebug>

namespace po=boost::program_options;
using std::string;
    
void setUpdateInterval(int steps); /**< Emit signal updated() once per this number of steps */
    
typedef struct s_popts {
  int delay;
  int updateInterval;
  string imagePath;
  float winnerLearnRate;
  float neighborLearnRate;
  int maxEdgeAge;
  int nodeInsertionDelay;
  float targetError;
  float errorReduction;
  float insertErrorReduction;
  int totalIterations;
} ProgOpts;

bool parse_args(int argc, char* argv[], ProgOpts& popts);

int main(int argc, char* argv[]) {
  // get command-line arguments
	ProgOpts popts;
	if(!parse_args(argc, argv, popts))
	  exit(1);

  QString imagePath = QString::fromStdString(popts.imagePath);

  // Create our QApplication object. Needed for the gui and for threading
  QApplication app(argc, argv);
  // Create the GNG object with bounds of -1 and 1.
  GrowingNeuralGas gng(5);

  // set command-line parameters
  gng.setDelay(popts.delay);
  gng.setWinnerLearnRate(popts.winnerLearnRate);
  gng.setNeighborLearnRate(popts.neighborLearnRate);
  gng.setMaxEdgeAge(popts.maxEdgeAge);
  gng.setNodeInsertionDelay(popts.nodeInsertionDelay);
  gng.setTargetError(popts.targetError);
  gng.setErrorReduction(popts.errorReduction);
  gng.setInsertErrorReduction(popts.insertErrorReduction);
  gng.setUpdateInterval(popts.updateInterval);

  // The ImageGenerator provides the source points for the gng (similar to the distribution)
  QImage tempimg(imagePath);
  ImageGenerator generator(tempimg);
  // The GngViewer provides the window in which we can see the results of the GNG/source image
  GngViewer view(generator.width(), generator.height());

  // Give the view a gng to visualize.
  view.setGng(&gng);
  view.show();
  
  // Tell the view to update each time the GNG emits updated(). Useful for async mode
  QObject::connect(&gng, SIGNAL(updated()), &view, SLOT(update()));
  
  // Give the visualizer the background. comment out these lines to have a blank background
  QPixmap img;
  img.load(imagePath);
  view.setSource(img);
  
  // Give the GNG its way of generating points
  gng.setPointGenerator(&generator);

  // Run the GNG asyncronously (in a separate thread) for 10,000 cycles
  gng.run(popts.totalIterations);
  
  // Run the GNG synchronously for 10,000 cycles. 
//   gng.synchronousRun(popts.totalIterations);
// 
//   // get subgraphs // TODO better comment
//   QList<Subgraph> subgraphs = gng.subgraphs();
//   gng.printSubgraphs(subgraphs);
// 
//   Subgraph best_match = gng.matchingSubgraph(subgraphs[subgraphs.size()-1], subgraphs);
//   /*qDebug() << "Printing Best Match!!";
//   foreach(Node* node, best_match){
//     qDebug() << node->toString();
//   }*/

  // Execute the Qt mainloop. Needed for widgets to update themselves/for events to happen
  app.exec();
  // This area is never reached by the program
}

bool parse_args(int argc, char* argv[], ProgOpts& popts){
   string configFile;
   po::options_description desc("Allowed options");
   desc.add_options()
     ("help,h", "Show this message")
     ("config,c", po::value<string>(&configFile), "Config file to read options from")
     ("imagePath,p", po::value<string>(&popts.imagePath), "Path to image")
     ("delay,d", po::value<int>(&popts.delay)->default_value(1), "Add a n millisecond delay to each step.")
     ("updateInterval,u", po::value<int>(&popts.updateInterval)->default_value(50), "Emit signal updated() once per this number of steps")
     ("winnerLearnrate,w", po::value<float>(&popts.winnerLearnRate)->default_value(0.1), "Used to adjust closest unit towards input point")
     ("neighborLearnRate,n", po::value<float>(&popts.neighborLearnRate)->default_value(0.01), "Used to adjust other neighbors towards input point")
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
   if (vm.count("help") || !vm.count("imagePath")){
     std::cout << desc;
	   return false;
   }
   return true;
}
