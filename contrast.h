#ifndef CONTRAST_H
#define CONTRAST_H
#include <vector>
#include <string>
#include <QImage>

using namespace std;

class Contrast
{
public:
    Contrast(int a);
    void change(QImage *prev_img, int val);
};

#endif // CONTRAST_H
