#ifndef SERIE_H
#define SERIE_H

#include <iostream>
#include <vector>
#include <QImage>


using namespace std;

enum Plan {Axial, Coronal, Sagittal, Unknown};

class Serie
{
private:
    char serieName[30];
    char serieDesc[100];
    Plan defaultPlan;

    vector<string> imgPaths;

    int serieId;
    int nbImages;
    int nbFrames;

    double rescaleXFactor;
    double rescaleYFactor;
    double rescaleZFactor;

    int WW;
    int WC;

    int imgXIndex;
    int imgYIndex;
    int imgZIndex;

    vector<uint8_t *> myPixelsZ;
    vector<uint8_t *> myPixelsX;
    vector<uint8_t *> myPixelsY;

    int pixelXdepth;
    int pixelYdepth;
    int pixelZdepth;

public:
    Serie(int id, Plan plan, vector <string> &Path, int nbframes, double rescale, char * description, int ww, int wc);
    QImage getCurrentImg(Plan currentPlan);
    string getPath(int pathNb);

    char* getName();
    char *getDescription();
    Plan getdefaultPlan();

    int getId();
    int getNbImages();
    int getNbFrames();
    int getWW();
    int getWC();


    int getXIndex();
    int getYIndex();
    int getZIndex();


    int getXdepth();
    int getYdepth();
    int getZdepth();

    void setDepths(int width, int height);
    void setXdepth(int depth);
    void setYdepth(int depth);
    void setZdepth(int depth);

    void storePixel(uint8_t *pixelData);

    void setNextIndex(Plan plan);
    void setPreviousIndex(Plan plan);

    void constructAxialPlan();
    void constructCoronalPlan();
    void constructSagittalPlan();

};

#endif // SERIE_H
