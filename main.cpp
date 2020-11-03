#include <iostream>
#include <cmath>
#include "AVL.h"
#include <queue>

using namespace std;
using namespace avl;
using namespace atl;
using namespace avs;

struct Line
{
public:
    double m;
    double b;
};

// Нахожднние коэффициентов m и b уравнения прямой y = mx +

Line lineCalc(double vx, double vy, double x0, double y0)
{
    Line output;
    int scale = 10;
    double x1 = x0 + scale*vx;
    double y1 = y0 + scale*vy;
    output.m = (y1-y0)/(x1-x0);
    output.b = y1 - output.m * x1;
    return output;
}

// Нахождение угла между двумя прямыми, заданными уравнениями ax + by + c = 0

double angle(Point2D pt1, Point2D pt2)
{
    double x1 = pt1.x;
    double y1 = pt1.y;
    double x2 = pt2.x;
    double y2 = pt2.y;
    double inner_product = x1*x2 + y1*y2;
    double len1 = hypot(x1,y1);
    double len2 = hypot(x2,y2);
    cout << len1 << endl;
    cout << len2 << endl;
    double a = acos(inner_product/(len1*len2));
    return a*180/M_PI;
}

// Нахождение точки пересечения двух прямых, заданных уравнениями y = mx + b

Point2D lineIntersect (Line L, Line R)
{
    // Для нахождения пересечения будем решать систему уравнений методом Крамера
    Point2D intersect;
    double a1 = -L.m;
    double b1 = 1;
    double c1 = L.b;

    double a2 = -R.m;
    double b2 = 1;
    double c2 = R.b;

    double d = a1*b2 - a2*b1; // Детерминант
    double dx = c1*b2 - c2*b1;
    double dy = a1*c2 - a2*c1;

    intersect.x = dx/d;
    intersect.y = dy/d;
    return intersect;
}

double* QueueToArray(queue <double> q, int num)
{
    double *arr = new double[num];
    for(int i = 0; i < num; i++)
    {
        arr[i] = q.front();
        q.pop();
    }
    return arr;
}

void process(Image im)
{
    int bw_width = 170; // минимальная длина полосы перехода в px

    Pixel green; // настройки рисования
    Pixel blue;
    Pixel yellow;
    green.Set(0,255,0,0);
    blue.Set(0,250,250,0);
    yellow.Set(250,250,0,0);
    DrawingStyle standard;
    standard.drawingMode = DrawingMode::HighQuality;
    standard.opacity = 1.0f;
    standard.thickness = 1.0f;
    standard.filled = false;
    standard.pointSize = 1.0f;

    Array<Path> b;
    queue <double> bxRight;
    queue <double> bxLeft;
    queue <double> byRight;
    queue <double> byLeft;
    Array<Point2D> bxbyLeftArray;
    Array<Point2D> bxbyRightArray;

    // Рассчитываем размеры изображения

    Size imSize;
    ImageToSize(im,imSize);
    int x = imSize.width;
    int y = imSize.height;

    // Определяем новое расширение и переводим в него изображение

    double ratio = double(y)/double(x);
    x = 800;
    y = int(x * ratio);
    Image toResize = im;
    ResizeImage(toResize,x,y,ResizeMethod::NearestNeighbour,im);

    // Выделяем белый цвет

    Image mask;
    ThresholdImage(im,NIL,160.0f,NIL,0.0f,mask);
    SaveImage(mask,NIL,"mask.jpg",0);

    // Размываем изображение
    Image erode;
    Region a;
    // int ErodeSize = int(y/60);
    ErodeImage(mask, NIL, NIL, NIL, KernelShape::Box, 1, 1, erode, a);
    SaveImage(erode,NIL,"erode.jpg",0);

    // Находим контуры различных объектов на изображении

    ThresholdToRegion(erode,NIL,128.0f,NIL,0.0f,a);
    RegionContours(a,RegionContourMode::PixelCenters,RegionConnectivity::EightDirections,b);

    // Отсекаем мелкие объекты, рисуем зеленые линии вдоль линий пехеходного перехода

    int num = 0;

    for (auto path : b)
    {
        Rectangle2D rect;
        PathBoundingRectangle(path,BoundingRectangleFeature::MinimalArea,0.0f,RectangleOrientation::Horizontal,rect);
        if(rect.width > bw_width)
        {
            Segment2D line;
            line.point1.x = rect.origin.x;
            line.point1.y = rect.origin.y;
            line.point2.x = rect.origin.x + rect.width;
            line.point2.y = rect.origin.y;
            DrawSegment(im,line,NIL,green,standard,MarkerType::Circle,5.0f);
            bxRight.push(line.point2.x);
            byRight.push(line.point2.y);
            bxLeft.push(line.point1.x);
            byLeft.push(line.point1.y);
            bxbyLeftArray.PushBack(line.point1);
            bxbyRightArray.PushBack(line.point2);
            Circle2D PointCircle;
            CreateCircle(line.point1,Anchor2D::MiddleCenter,5,PointCircle);
            DrawCircle(im, PointCircle,NIL,blue,standard);
            CreateCircle(line.point2,Anchor2D::MiddleCenter,5,PointCircle);
            DrawCircle(im, PointCircle,NIL,yellow,standard);
            num++;
        }
    }

    // Находим среднее значение для левого и правого массива

    Point2D median_L;
    Point2D median_R;
    PointsMedian(bxbyLeftArray,NIL,10,median_L);
    PointsMedian(bxbyRightArray,NIL,10,median_R);

    SaveImage(im,NIL,"im.jpg",0);
}

int main()
{
    Image im;
    LoadImage("../im2.jpg",0,im);
    process(im);
    return 0;
}
