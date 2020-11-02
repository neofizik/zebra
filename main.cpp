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

struct Point
{
public:
    double x;
    double y;
};

int W = 480;
int H = 320;

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

double angle(Point pt1, Point pt2)
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

Point lineIntersect (Line L, Line R)
{
    // Для нахождения пересечения будем решать систему уравнений методом Крамера
    Point intersect;
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

void process(Image im)
{
    int x = W;
    int y = H;
    int bw_width = 110;
    Pixel green;
    green.Set(0,255,0,0);
    DrawingStyle standard;
    standard.drawingMode = DrawingMode::HighQuality;
    standard.opacity = 1.0f;
    standard.thickness = 1.0f;
    standard.filled = false;
    standard.pointSize = 1.0f;

    // Выделяем белый цвет

    Image mask;
    ThresholdImage(im,NIL,160.0f,NIL,0.0f,mask);
    SaveImage(mask,NIL,"mask.jpg",0);

    // Размываем изображение
    Image erode;
    Region a;
    int ErodeSize = int(y/40);
    ErodeImage(mask, NIL, NIL, NIL, KernelShape::Box, 1, 1, erode, a);
    SaveImage(erode,NIL,"erode.jpg",0);

    // Находим контуры
    ThresholdToRegion(erode,NIL,128.0f,NIL,0.0f,a);
    Array<Path> b;
    queue <double> bxRight;
    queue <double> bxLeft;
    queue <double> byRight;
    queue <double> byLeft;
    RegionContours(a,RegionContourMode::PixelCenters,RegionConnectivity::EightDirections,b);
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
        }
    }
    SaveImage(im,NIL,"im.jpg",0);
}

int main()
{
    Image im;
    LoadImage("../iii.jpg",0,im);
    process(im);
    return 0;
}
