#include "contrast.h"
#include <QMap>
#include <QRgb>
#include <QColor>

using namespace std;

Contrast::Contrast(int a)
{
 //DO NOTHING
}

void Contrast::change(QImage *prev_img, double val){
    QMap<QRgb,long> histo; // histogram map
         QMap<QRgb,long>::iterator j;
         QRgb c = 0;
         int l, k;
         histo.clear();
         for(k = 0; k < prev_image->width(); ++k)
            for( l = 0; l < prev_image->height(); ++l) {
                  c = prev_image.pixel(k,l);

                if(!histo.contains(c))
                    histo.insert(c,0);
            }
         //computation of occurences
         for( k = 0; k < prev_image->width(); ++k)
            for( l = 0; l < prev_image->height(); ++l) {
                c = prev_image->pixel(k,l);
                histo[c] = histo[c]+1;
            }
                     //compute average value
                     long sum_1 = 0;
                     long sum_2 = 0;
                     for(j = histo.begin();j!=histo.end();j++)
                     {
                         sum_1+=j.value()*j.key();
                     }
                     for(j = histo.begin();j!=histo.end();j++)
                     {
                         sum_2+=j.value();
                     }
                     long av = sum_1/sum_2;
                     av_r = qRed(av);
                     av_g = qGreen(av);
                     av_b = qBlue(av);
                    //changing contrast of an image by factor getted from horizontal slider ui:
                    double factor = (double)( val/100) ;    // to change also   //got to be a value between 0 - 1
                    if(factor!=0.99)
                    for (int y = 0; y < prev_image->height(); ++y) {
                    for (int x = 0; x < prev_image->width(); ++x) {
                    c = QRgb(prev_image.pixel(x,y));
                    QColor col(c);
                    col.getRgb(&r,&g,&b);

                    if(factor!=1)
                   for (l = 0; l < working_image.height(); ++l) {
                   for (k = 0; k < working_image.width(); ++k) {
                   c = QRgb(working_image.pixel(k,l));
                   QColor col(c);
                   col.getRgb(&r,&g,&b);
                   double r_n = r / 255.0;
                   r_n -= 0.5;
                   r_n *= factor;
                   r_n += 0.5;
                   r_n *= 255;
                   double g_n = g / 255.0;
                   g_n -= 0.5;
                   g_n *= factor;
                   g_n += 0.5;
                   g_n *= 255;
                   double b_n = b / 255.0;
                   b_n -= 0.5;
                   b_n *= factor;
                   b_n += 0.5;
                   b_n *= 255;
                   if(r_n>255)
                   r_n=255;
                   if(r_n<0)
                   r_n=0;
                   if(g_n>255)
                   g_n=255;
                   if(g_n<0)
                   g_n=0;
                   if(b_n>255)
                   b=255;
                   if(b_n<0)
                   b_n=0;
                   prev_img->setPixel(k,l,qRgb((int)r_n,(int)g_n,(int)b_n));
                   }
                   }






}
