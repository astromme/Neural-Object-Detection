
#include "gngviewer.h"
#include "libgng/gng.h"
#include "libgng/imagegenerator.h"
#include "libgng/node.h"

#include <QApplication>
#include <QDebug>


int main(int argc, char* argv[]) {
  if (argc < 2) {
    qDebug() << "Usage:" << argv[0] << "[image]";
    return 1;
  }
  
  QString imagePath = argv[1];

  // Create our QApplication object. Needed for the gui and for threading
  QApplication app(argc, argv);
  // Create the GNG object with bounds of -1 and 1.
  GrowingNeuralGas gng(5, -1, 1, 5000);
  // The ImageGenerator provides the source points for the gng (similar to the distribution)
  ImageGenerator generator(imagePath);
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
  //gng.run(10000);
  
  // Run the GNG synchronously for 10,000 cycles. 
  gng.synchronousRun(10000);

  // get subgraphs // TODO better comment
  QList<NodeList> subgraphs = gng.getSubgraphs();
  gng.printSubgraphs(subgraphs);

  // Execute the Qt mainloop. Needed for widgets to update themselves/for events to happen
  app.exec();
  // This area is never reached by the program
}
