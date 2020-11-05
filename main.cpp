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
    int radius = 250;

    Pixel green; // настройки рисования
    Pixel cyan;
    Pixel yellow;
    Pixel blue;
    green.Set(0,255,0,0);
    cyan.Set(0,250,250,0);
    yellow.Set(250,250,0,0);
    blue.Set(0,0,255,0);
    DrawingStyle standard;
    standard.drawingMode = DrawingMode::HighQuality;
    standard.opacity = 1.0f;
    standard.thickness = 1.0f;
    standard.filled = false;
    standard.pointSize = 1.0f;

    Array<Path> b; // массив контуров на изображении
    queue <double> bxRight;
    queue <double> bxLeft;
    queue <double> byRight;
    queue <double> byLeft;
    Array<Point2D> bxbyLeftArray; // точки слева и справа
    Array<Point2D> bxbyRightArray;
    Array<Point2D> boundedLeft; // точки слева и справа, лежащие в пределах окружности
    Array<Point2D> boundedRight;
    Point2D median_L; // медианные точки
    Point2D median_R;

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

    // Анализ гистограммы

    Histogram hist;
    float threshold;
    ImageHistogram(im,NIL,0,1.0f,0.0f,255.0f,hist);
    HistogramDataMedian(hist,threshold);
    threshold+=20;

    // Выделяем белый цвет

    Image mask;
    ThresholdImage(im,NIL,threshold,NIL,0.0f,mask);
    SaveImage(mask,NIL,"mask.jpg",0);

    // Размываем изображение
    Image erode;
    Region a;
    int ErodeSize = int(y/60);
    ErodeImage(mask, NIL, NIL, NIL, KernelShape::Box, ErodeSize, 1, erode, a);
    SaveImage(erode,NIL,"erode.jpg",0);

    // Находим контуры различных объектов на изображении

    DetectEdges_AsPaths(erode,NIL,EdgeFilter::Canny,2.0f,NIL,15.0f,5.0f,NIL,30.0f,0.0f,NIL,0.0f,b);

    ThresholdToRegion(erode,NIL,128.0f,NIL,0.0f,a);
   // RegionContours(a,RegionContourMode::PixelCenters,RegionConnectivity::EightDirections,b);

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
            DrawCircle(im, PointCircle,NIL,cyan,standard);
            CreateCircle(line.point2,Anchor2D::MiddleCenter,5,PointCircle);
            DrawCircle(im, PointCircle,NIL,yellow,standard);
            num++;
        }
    }

    // Находим медианную точку для левого и правого массива

    PointsMedian(bxbyLeftArray,NIL,10,median_L);
    PointsMedian(bxbyRightArray,NIL,10,median_R);

    SaveImage(im,NIL,"im.jpg",0);

    // Медианная точка - центр окружности с радиусом r. Выбираем только те точки, что лежат в пределах окружности

    for(auto point : bxbyLeftArray)
    {
        if(pow((median_L.x - point.x),2) + pow((median_L.y - point.y),2) < pow(radius,2))
        {
            boundedLeft.PushBack(point);
        }
    }

    for(auto point : bxbyRightArray)
    {
        if(pow((median_R.x - point.x),2) + pow((median_R.y - point.y),2) < pow(radius,2))
        {
            boundedRight.PushBack(point);
        }
    }

    // RANSAC

    // Conditional<Line2D> lineLeft;
    // Conditional<Line2D> lineRight;

    Line2D lineLeft;
    Line2D lineRight;

    FitLineToPoints_LTE(boundedLeft,NIL,3,NIL,lineLeft,NIL,NIL,num);
    FitLineToPoints_LTE(boundedRight,NIL,3,NIL,lineRight,NIL,NIL,num);


    // FitLineToPoints_RANSAC(boundedLeft,NIL,0,10.0f,42,lineLeft);
    // FitLineToPoints_RANSAC(boundedRight,NIL,0,10.0f,42,lineRight);
    DrawLine(im,lineLeft,NIL,blue,standard);
    DrawLine(im,lineRight,NIL,blue,standard);
    SaveImage(im,NIL,"im.jpg",0);
    int i = 0;
    while(1)
    {
        Conditional<Point2D> point;
        LineLineIntersection(lineLeft,lineRight,point);
        float distleft;
        float distright;
        DrawingStyle draw;
        draw.thickness = 3.0f;
        PointToLineDistance(bxbyLeftArray[i],lineLeft,1.0f,distleft,NIL,NIL);
        PointToLineDistance(bxbyRightArray[i],lineRight,1.0f,distright,NIL,NIL);
        if((bxbyLeftArray[i].y > point->y && distleft < 15) || (bxbyRightArray[i].y > point->y && distright < 15))
        {
            Line2D lin;
            LineThroughPoints(bxbyLeftArray[i],bxbyRightArray[i],lin);
            DrawLine(im,lin,NIL,cyan,draw);
            break;
        }
        i++;
    }
    SaveImage(im,NIL,"im.jpg",0);

}

int main()
{
    Image im;
    LoadImage("../im6.jpg",0,im);
    process(im);
    return 0;
}
