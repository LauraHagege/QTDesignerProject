#ifndef SERIE_H
#define SERIE_H


#include <iostream>
#include <vector>
#include <QImage>
#include <QPixmap>
#include <QPainter>


using namespace std;

enum Plan {Axial, Coronal, Sagittal, Unknown, FlagImg};

//Contrast level relatives to the one that DICOM offers
enum Contrast{Default, DefaultInverted, MinMax, MinMaxInverted, Histo, HistoInverted};

//----------CLASS SERIE ----------//
//Element to store all information relative to a serie
class Serie
{
private:
    //Declaring private attribute
    //boolean attribute to indicate if the pixel vector have already been filled or not
    bool built;

    //contains the name of the serie, format: Serie1
    char serieName[30];
    //contains the description of the serie
    char serieDesc[100];
    //stores the default plan of the serie
    Plan defaultPlan;

    //Study number associated to the serie
    char studyName[10];

    //serie reference is the SRS* number
    char serieRef[9];

    //vector stroring all paths to the images of the serie
    vector<string> imgPaths;

    //storing serie Id
    int serieId;

    //Number of images of the serie
    //If multiple singleframe images in the serie nbFrame=0
    //If one multiframe image nbImages=nbFrames
    int nbImages;
    int nbFrames;

    //Factore to rescale the images for the plan that are created after pixel treatment
    double rescaleXFactor;
    double rescaleYFactor;
    double rescaleZFactor;

    //storing the window width and the window center, commin to all images of the serie
    //value given from DICOM tags, usefull for the default contrast
    int WW;
    int WC;

    //Index of the image to be displayed relatively to the pixel vector
    int imgXIndex;
    int imgYIndex;
    int imgZIndex;

    //Vector storing pixels for each images
    //pixelZ - storing axial plan
    vector<uint8_t *> myPixelsZ;
    //pixelX - storing sagittal plan
    vector<uint8_t *> myPixelsX;
    //pixelY - storing coronal plan
    vector<uint8_t *> myPixelsY;

    //storing the lenght of the corresponding vector
    int pixelXdepth;
    int pixelYdepth;
    int pixelZdepth;

    //boolean value to know if views of the serie has been ask to be linked
    bool viewLinked;
    //serie is ;ultiplan if it contains enough value in order to construct relaliv plans
    bool MultiPlan;

    //saving the contrast of the serie
    Contrast contrast;

    //storing in wich window is the plan displayed
    //if the plan is not in any window the vector contains -1
    vector<int> axialWindow;
    vector<int> coronalWindow;
    vector<int> sagittalWindow;

    bool hasFlagImages;
    vector <QPixmap*> FlagImages;
    int flagIndex;
    int nbFlag;


public:
    Serie(int id, char *studyname, char *serieref, Plan plan, char *absolutepath, vector <string> &Path, int nbframes, double rescale, char * description, int ww, int wc);

    //return the image corresponding to the given plan at the corresponding indexX,Y,Z
    QPixmap getCurrentImg(Plan currentPlan);

    //return the path saved at pathNb
    string getPath(int pathNb);

    //storing image pixel in the right pixel vector
    void storePixel(uint8_t *pixelData);
    //clearing pixels vector
    void clearPixels();

    //calling for the following function depending on the default plan
    void constructPlans();

    //construction of respective plan depending on the serie default plan
    void constructAxialPlan();
    void constructCoronalPlan();
    void constructSagittalPlan();

    //PROPERTIES SETTER FUNCTIONS
    void setDepths(int width, int height);
    void setXdepth(int depth);
    void setYdepth(int depth);
    void setZdepth(int depth);

    //Set next and previous index for scrolling trough images
    void setNextIndex(Plan plan);
    void setPreviousIndex(Plan plan);

    //set the boolean viewLinked to true or false
    void setViewLinked();

    //For multiplan series
    //Pair on plan with the window inwhich it is displayed
    void setPlanWindows(int windowSerieNb[], Plan windowCurrentPlan[]);

    void setcontrast(Contrast c);

    //Store the flagged images according to the path
    void setFlags(char * absolutePath);


    //PROPERTIES GETTER FUNCTIONS
    bool isBuilt();
    char* getName();
    int getId();
    char *getDescription();
    Plan getdefaultPlan();


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

    bool getViewLinked();

    vector<int> getAxialWindow();
    vector<int> getCoronalWindow();
    vector<int> getSagittalWindow();
    vector<int> getwindow(Plan plan);

    bool getMultiplan();

    Contrast getcontrast();

    QPixmap getFlags();
    bool hasFlag();

};

#endif // SERIE_H
