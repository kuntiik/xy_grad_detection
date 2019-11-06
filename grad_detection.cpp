#include <stdlib.h>
#include "opencv2/opencv.hpp"
#include "functions.hpp"
using namespace cv;
using namespace std;

extern const int HEIGHT;
extern const int WIDTH;

const int X_GRAD_KERNEL_3[49] = { 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0,
                                  0, 0,-1, 0, 1, 0, 0,
                                  0, 0,-2, 0, 2, 0, 0,
                                  0, 0,-1, 0, 1, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0};

const int Y_GRAD_KERNEL_3[49] = { 0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0,
                                  0, 0,-1,-2,-1, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 1, 2, 1, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0};

const int X_GRAD_KERNEL_5[49] = { 0, 0, 0, 0, 0, 0, 0,
                                  0,-2,-1, 0, 1, 2, 0,
                                  0,-2,-1, 0, 1, 2, 0,
                                  0,-4,-2, 0, 2, 4, 0,
                                  0,-2,-1, 0, 1, 2, 0,
                                  0,-2,-1, 0, 1, 2, 0,
                                  0, 0, 0, 0, 0, 0, 0};

const int X_GRAD_KERNEL_5MY[49]= { 0, 0, 0, 0, 0, 0, 0,
                                  0,-0,-1, 0, 1, 0, 0,
                                  0,-2,-4, 0, 4, 2, 0,
                                 -2,-4,-6, 0, 6, 4, 2,
                                  0,-2,-4, 0, 4, 2, 0,
                                  0,-0,-1, 0, 1, 0, 0,
                                  0, 0, 0, 0, 0, 0, 0};

const int Y_GRAD_KERNEL_5MY[49]={ 0, 0, 0,-2, 0, 0, 0,
                                  0,-0,-2,-4,-2,-0, 0,
                                  0,-1,-4,-6,-4,-1, 0,
                                  0, 0, 0, 0, 0, 0, 0,
                                  0, 1, 4, 6, 4, 1, 0,
                                  0, 0, 2, 4, 2, 0, 0,
                                  0, 0, 0, 2, 0, 0, 0};
class Stack{
  public:
    int size;
    int *storage;
    int tail;
  Stack(int s){
    size = s;
    tail = 0;
    storage = (int*)malloc(2*s*sizeof(int));
    for(int i = 0; i < s; i++){
      storage[i] = 0;
    }
  }
  ~Stack(){
    free(storage);
  }
  void push(int x, int y){
   if(tail >= size - 3){
     size = 2*size;
     storage = (int*)realloc(storage, sizeof(int)*size);
     for(int i = size/2; i < size; i++){storage[i] = 0;}
   }
   tail++;
   storage[tail] = x;
   tail++;
   storage[tail] = y;
  }
  Coord pop(){
    struct Coord ret;
    ret.y = storage[tail];
    tail--;
    ret.x = storage[tail];
    tail--;
    return ret;
  }
  bool is_empty(){
    if(tail <= 0){ return true; }
    else{return false;}
  }
};

void find_zero(int *regions, Coord *first_out, bool *found_out);
void expand(int16_t *x_grad, int16_t *y_grad, int *regions, int diff,int region_value_x, int region_value_y, int region_number, int *not_sorted, Coord *first_out, bool *found_out, Stack *stack);

int16_t *x_grad, *y_grad;
void my_mouse_callback2(int event, int x, int y, int flags, void *param){
  int16_t *image = (int16_t*)param;
  switch( event ){
    case EVENT_LBUTTONDOWN: {
      cout <<"x_grad is :" <<  x_grad[y*WIDTH + x] << " : ";
      cout <<"y_grad is :" <<  y_grad[y*WIDTH + x] << endl;
      break;
    }
  }
}
void my_mouse_callback(int event, int x, int y, int flags, void *param){
  int16_t *image = (int16_t*)param;
  switch( event ){
    case EVENT_LBUTTONDOWN: {
      cout << image[y*WIDTH + x] << endl;
      break;
    }
  }
}
int *make_regions(int16_t *x_grad, int16_t *y_grad, int diff){
  int *regions = (int*)calloc(WIDTH*HEIGHT, sizeof(int));
  int region_number = 1;
  int region_value_x, region_value_y;
  int not_sorted = HEIGHT*WIDTH;
  Coord first_out = {.x=200, .y=100};
  bool found_out = true;
  Stack stack(10000);

  while(not_sorted != 0){
    if(found_out){found_out = false;}
    else{find_zero(regions, &first_out, &found_out);}
    regions[first_out.y*WIDTH+first_out.x] = region_number;
    region_value_x = x_grad[first_out.y*WIDTH + first_out.x];
    region_value_y = y_grad[first_out.y*WIDTH + first_out.x];
    //cout << "region x " << region_value_x << "region y " << region_value_y <<endl;
    stack.push(first_out.x, first_out.y);
    not_sorted--;
    //cout << not_sorted << " ";
    while(!stack.is_empty()){
      expand(x_grad, y_grad, regions, diff, region_value_x, region_value_y, region_number, &not_sorted, &first_out, &found_out, &stack);
    }
      region_number++;

  }
  cout << "number of regions is: " << region_number << endl;
  return regions;
}

void expand(int16_t *x_grad, int16_t *y_grad, int *regions, int diff,int region_value_x, int region_value_y, int region_number, int *not_sorted, Coord *first_out, bool *found_out, Stack *stack){

  Coord it_tuple[] = {
    { it_tuple[0].x = -1, it_tuple[0].y = 0},
    { it_tuple[1].x =  1, it_tuple[1].y = 0},
    { it_tuple[2].x =  0, it_tuple[2].y = 1},
    { it_tuple[3].x =  0, it_tuple[3].y = -1}};

Coord coord, t;
coord = stack->pop();
for(int i = 0; i < 4; i++){
  t.x = coord.x + it_tuple[i].x;
  t.y = coord.y + it_tuple[i].y;
  if(in_picture(t) && regions[t.y*WIDTH + t.x] == 0){

      //cout << region_value_x - diff << " : " << x_grad[t.y*WIDTH+t.x] << " : " << region_value_x + diff <<endl;  
      //cout << region_value_y - diff << " : " << y_grad[t.y*WIDTH+t.x] << " : " << region_value_y + diff <<endl;  
      //cout << "OUT" << endl;
    if(x_grad[t.y*WIDTH+t.x] >= region_value_x -diff && region_value_x  + diff >= x_grad[t.y*WIDTH+t.x] && y_grad[t.y*WIDTH+t.x] + diff >= region_value_y && region_value_y >= y_grad[t.y*WIDTH+t.x] - diff){
      //if(x_grad[t.y*WIDTH +t.x] + diff >= region_value_x && region_value_x >= x_grad[t.y*WIDTH + t.x] - diff){
      //cout << region_value_x - diff << " : " << x_grad[t.y*WIDTH+t.x] << " : " << region_value_x + diff <<endl;  
      //cout << region_value_y - diff << " : " << y_grad[t.y*WIDTH+t.x] << " : " << region_value_y + diff <<endl;  
      //cout << "IN" << endl;
      stack->push(t.x, t.y);
      regions[t.y*WIDTH + t.x] = region_number;
      (*not_sorted)--;
    }
    else{
      if(*found_out == false){
        *found_out = true;
        first_out->x = t.x;
        first_out->y = t.y;
      }
    }
  }
}
}
void find_zero(int *regions, Coord *first_out, bool *found_out){
  bool found = false;
  for(int i = 0; i < HEIGHT; i++){
    for(int j = 0; j < WIDTH; j++){
      if(regions[i*WIDTH+j] == 0){
        first_out->x = j;
        first_out->y = i;
        found = true;
        break;
      }
    }
    if(found){break;}
  }
}

  void color_regions(int *completed){
    //erase all regions that are small
    int count[500000] = {0};
    int reg;
    for(int i = 0; i < HEIGHT; i++){
      for(int j = 0; j < WIDTH; j++){
        reg = completed[i*WIDTH+j];
        count[reg]++;
      }
    }
    int tmp;
    uint8_t data[HEIGHT][WIDTH][3];
    for(int i = 0; i < HEIGHT; i++){
      for(int j = 0; j < WIDTH; j++){
        tmp = completed[i*WIDTH+j];
        if(count[tmp] <= 30){
          data[i][j][0] = 0;
          data[i][j][1] = 0;
          data[i][j][2] = 0;
        }
        else{
        tmp = tmp % 15;
        data[i][j][tmp % 3] = tmp%5 * 64 -1; 
        data[i][j][tmp+1 % 3] = (tmp+1)%5 * 64 -1; 
        data[i][j][tmp+2 % 3] = (tmp+3)%5 * 64 -1; 
        }
      }
    }
    Mat image_col(HEIGHT, WIDTH, CV_8UC3, data);
  //Rect Rec(270, 170, 30, 30);
  //rectangle(image_col,Rec,Scalar(255,0,0),1,8,0);
    imshow("Colors", image_col);
    imwrite( "../images/3x3kernel_.jpg", image_col );
    setMouseCallback("Colors", my_mouse_callback2, (void*)y_grad);
  }

int main(int argc, char **argv) {
  Mat image;
  FileStorage fs;
  if (argc < 2) {
    fs.open("prob0001.xml", FileStorage::READ);
  }
  else {
    fs.open(argv[1], FileStorage::READ);
  }
  fs["depth"] >> image;
  uint16_t tmp;
  uint8_t  im_array[480][848];
  for (int i = 0; i < 480; i++) {
    for (int j = 0; j < 848; j++) {
      tmp = image.at<uint16_t>(i, j) / 20;
      if (tmp >= 255) {
        tmp = 255;
      }
      im_array[i][j] = (uint8_t)tmp;
    }
  }
  Mat grey_image(HEIGHT, WIDTH, CV_8U, im_array);
  Mat edges;  
  Canny( grey_image, edges, 90, 20*3, 3 );
  //int16_t *x_grad = calculate_grad(image, X_GRAD_KERNEL_3);
  //int16_t *y_grad = calculate_grad(image, Y_GRAD_KERNEL_3);
  x_grad = calculate_grad(image, X_GRAD_KERNEL_3);
  y_grad = calculate_grad(image, Y_GRAD_KERNEL_3);
  cout << "gradients calculated" << endl;
  int limit = 12;
  int *regions = make_regions(x_grad, y_grad, limit);
  color_regions(regions);
  imshow("image depth", grey_image);
  //imshow("image edges", edges);
  int p = 170;
  int p2 = 270;
  //for(int i = p; i < 30+p;i++){
    //for(int j = p2; j < 30 + p2; j++){
      ////cout << x_grad[i*WIDTH + j] << " ";
      //printf("%4d ", x_grad[i*WIDTH+j]);
    //}
    //cout << endl;
  //}
  //for(int i = p; i < 30+p;i++){
    //for(int j = p2; j < 30 + p2; j++){
      ////cout << x_grad[i*WIDTH + j] << " ";
      //printf("%4d ", regions[i*WIDTH+j]);
    //}
    //cout << endl;
  //}
  //setMouseCallback("image depth", my_mouse_callback, (void*)x_grad);
  waitKey(0);
  free(x_grad);
  return 0;
}
