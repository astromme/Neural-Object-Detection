#include <cstdio>
#include <iostream>
#include <string>
#include <ctime>
#include "cv.h"
#include "highgui.h"

using namespace cv;
using std::cout;

int main(int, char**)
{
    CvCapture* cam = cvCreateCameraCapture(-1);
    if (!cam){
      printf("Camera not found!\n");
      return -1;
    }

    //Mat edges;
    Mat frame;
    IplImage *img;
    //namedWindow("edges",1);


    while(true)
    {
      //cap >> frame; // get a new frame from camera

      //cvtColor(frame, edges, CV_BGR);
      //cvtColor(frame, edges, CV_BGR2GRAY);
      //GaussianBlur(edges, edges, Size(7,7), 1.5, 1.5);
      //Canny(edges, edges, 0, 30, 3);
      //imshow("frame", frame);

      cvGrabFrame(cam);
      img = cvRetrieveFrame(cam);
      cvShowImage("img", img);
      if(waitKey(30) >= 0) 
        break;
        
    }
    int type = frame.type();
    //int val = frame.at<int>(0,0);
    //printf("%d\n",val);
    //cout << frame.at<float>(0,0);
    //
    CvScalar s;
    for (int r=0; r<img->height; r++){
      for (int c=0; c<img->width; c++){
        s=cvGet2D(img,r,c); // get the (i,j) pixel value 
        printf("[%d,%d]: R=%f, G=%f, B=%f\n",r,c,s.val[2],s.val[1],s.val[0]); 
        usleep(1);
      }
    }

    //s.val[0]=111; 
    //s.val[1]=111; 
    //s.val[2]=111; 
    //cvSet2D(img,i,j,s); // set the (i,j) pixel value 


    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
}
