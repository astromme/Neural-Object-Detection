
#include "gngviewer.h"
#include "libgng/gng.h"
#include "libgng/imagegenerator.h"

#include <QApplication>
#include <QDebug>


int main(int argc, char* argv[]) {
  if (argc < 2) {
    qDebug() << "Usage: " << argv[0] << " [frame]";
    return 1;
  }
  
  //QString imagePath = argv[1];
  QApplication app(argc, argv);
  
    
  QString templateImagePath = "../images/rgb/rgb%1.png";

  GrowingNeuralGas gng(5);
  GngViewer view(400, 400);
  QList<ImageGenerator> generators;
  QList<QString> imagePaths;
  
  for(int i=1; i<=11; i++) {
    QString imagePath = templateImagePath.arg(i);
    generators.append(ImageGenerator(imagePath));
    imagePaths.append(imagePath);
  }
  
  view.setGng(&gng);
  view.show();
  
  QObject::connect(&gng, SIGNAL(updated()), &view, SLOT(update()));
  
  int frame = QString(argv[1]).toInt();  
  
  QPixmap img;
  img.load(imagePaths[frame]);
  view.setSource(img);
  gng.setPointGenerator(&generators[frame]);
  
  gng.run(100000);
  app.exec();
}
