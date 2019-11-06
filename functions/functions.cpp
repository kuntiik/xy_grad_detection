#include "functions.hpp"

using namespace cv;

extern const int HEIGHT= 480;
extern const int WIDTH = 848;

int16_t* calculate_grad(Mat img, const int *kernel){
  int norm = 0;
  for(int i = 0; i < 49; i++){
    if(kernel[i] > 0){
    norm+= kernel[i];
    }
  }
  int16_t *grad = (int16_t*)calloc(HEIGHT*WIDTH, sizeof(int16_t));
  Coord c_tup[49];
  int n = 0;
  for(int i = -3; i < 4; i++){
    for(int j = -3; j <4; j++){
      c_tup[n].x = j;
      c_tup[n].y = i;
      n++;
  }
 }
 int tmp = 0;
 int k = 0;
 Coord t,f;
 for(int i = 0; i < HEIGHT; i++){
   for(int j = 0; j < WIDTH; j++){
      for(int l = 0; l < 49; l++){
        t = c_tup[l];
        f.x = t.x + j;
        f.y = t.y + i;
        if(in_picture(f)){
            k = kernel[(t.y+4)*7+t.x+3];
            tmp += k*img.at<uint16_t>(f.y,f.x);
            }
      }
      tmp = tmp / norm;
      if(tmp >= 32000){tmp = 32000;}
      else if(tmp <= -32000){tmp = 32000;}
      grad[i*WIDTH+j] = tmp;
      tmp = 0;
   }
 }

 return grad;
 }
  

bool in_picture(Coord t){
  if(t.x < 0 || t.x >= WIDTH){return false;}
  else if(t.y < 0 || t.y >= HEIGHT){return false;}
  else{return true;}
}

