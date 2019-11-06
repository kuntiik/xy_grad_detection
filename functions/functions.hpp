#ifndef FUNCTIONS
#define FUNCTIONS
#include <opencv2/opencv.hpp>

using namespace cv;

struct Coord{
  int x;
  int y;
};


bool in_picture(Coord t);
int16_t* calculate_grad(Mat img, const int *kernel);

#endif
