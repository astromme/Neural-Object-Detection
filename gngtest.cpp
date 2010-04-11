
#include "gngviewer.h"
#include "libgng/gng.h"
#include "libgng/imagegenerator.h"

#include <QApplication>
#include <QDebug>


int main(int argc, char* argv[]) {
  if (argc < 2) {
    qDebug() << "Usage:" << argv[0] << "[image]";
    return 1;
  }
  
  QString imagePath = argv[1];
  
  QApplication app(argc, argv);
  GrowingNeuralGas gng(5, -1, 1, 5000);
  ImageGenerator generator(imagePath);
  GngViewer view(generator.width(), generator.height());

  view.setGng(&gng);
  view.show();
  
  QObject::connect(&gng, SIGNAL(updated()), &view, SLOT(update()));
  
  QPixmap img;
  img.load(imagePath);
  view.setSource(img);
  gng.setPointGenerator(&generator);
  
  gng.run(100000);
  app.exec();
}
