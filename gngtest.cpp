
#include "gngviewer.h"
#include "libgng/gng.h"
#include "libgng/imagegenerator.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    qDebug() << "Usage: " << argv[0] << " [imagefile]";
    return 1;
  }
  
  QString imagePath = argv[1];
  
  GrowingNeuralGas gng = GrowingNeuralGas(5);
  ImageGenerator generator = ImageGenerator(imagePath);
  //gng.run(100000, &generator);
  
  QApplication app(argc, argv);
  GngViewer view(generator.width(), generator.height());
  view.setGng(&gng);
  QPixmap img;
  img.load(imagePath);
  view.setSource(img);
  view.show();
  app.exec();
}
