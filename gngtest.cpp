
#include "gngviewer.h"
#include "libgng/gng.h"
#include "libgng/imagegenerator.h"

#include <QApplication>

int main(int argc, char* argv[]) {
  GrowingNeuralGas gng = GrowingNeuralGas(5);

  ImageGenerator generator = ImageGenerator("../images/rgb/rgb1.png");
  gng.run(100000, &generator);
  
  QApplication app(argc, argv);
  GngViewer view(generator.width(), generator.height());
  view.setGng(&gng);
  view.show();
  app.exec();
}