#ifndef MAINWINDOW_H
#define MAINWINDOW_H
//#include "/Users/laurah/Desktop/malon/dcmtk-3.6.3-install/usr/local/include/dcmtk/dcmdata/dctk.h"
//#include "/Users/laurah/Desktop/malon/dcmtk-3.6.3-install/usr/local/include/dcmtk/dcmimgle/dcmimage.h"
//#include <dcmtk/config/osconfig.h>
//#include <dcmtk/dcmimgle/dcmimage.h>
//#include "/Users/laurah/Desktop/malon/dcmtk-3.6.3/dcmimgle/libsrc/dcmimage.cc"
#include <QString>

#include <QMainWindow>
#include <QPushButton>

#include "qimage.h"
#include "serie.h"

//#include <dcmtk/config/cfunix.h>
//#include <dcmtk/config/osconfig.h>
//#include <dcmtk/ofstd/ofstring.h>
//#include <dcmtk/ofstd/ofconsol.h>
//#include <dcmtk/dcmdata/dcdeftag.h>
//#include <dcmtk/dcmdata/dcfilefo.h>
//#include <dcmtk/dcmimgle/dcmimage.h>
//#include "dcmtk/dcmimage/diregist.h"
#include <QImage>
#include <QGraphicsScene>
#include <QLabel>
#include <QStackedWidget>
#include <QScrollBar>
#include <vector>
#include <map>
#include <string>
#include <QWheelEvent>
#include "reportwindow.h"
#include <QPixmap>



using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    //call all relevant function in order to build the window architecture and display images
    void constructWindow(QString path);

    void processDicom(const char *dicomdirPath,char *filepath);
    void addSerieButton(Serie* serie);
    void setPatientInfo(char *studydesc, char * date, char* patientName, char* birthdate);


   // void displayImages();

    //construct other plan according to the given one
    //if coronal --> sagittal and axial
    //if axial --> sagittal and coronal
    // if sagittal --> coronal and axial
    //void constructPlans();

    void createDefaultPlan();

    void constructAxialPlan();
    void constructCoronalPlan();
    void constructSagittalPlan();


    void setSelectedWindow(int nb);

    void createScene();

    Plan findSeriePlan(const char * orientation);
    double getPixelNb(const char* pixelArray);
    void rotate(int rotation);

    void updateWindowInfo();

    //void clearImagesVector();


private slots:
    void mousePressEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *event);
    void buttonInGroupClicked(QAbstractButton *);
    void createButtons();
    void hide_advanced();
    void invert_grayscale();
    void show_advanced();
    void display_one_window();
    void display_two_window();
    void display_four_window();
    void zoom_plus();
    void zoom_minus();
    void rotate_left();
    void rotate_right();
    void default_contrast();
    void minmax_contrast();
    void histo_contrast();
    void on_showReport_clicked();

    void callAxial();
    void callCoronal();
    void callSagittal();

    void callTest();

    void link_views();
private:
    int selectedWindow; // store the number of the current selected graphic window --> for mouse events
    bool invertGrayScale ; // boolean variable to sepcify if Grayscaled is inverted
    int *Index; // index array to store the number of the current displayed image according to each graphic window
    int series; //number of series in the directory

    int currentSerieNumber; // asociated in for the current displayed serie
    string currentSerie; //name of the current selected serie

   // vector<int> serieNbImages;
    int currentNbImages;

    //map<string, vector <string>> allPath; // store all path for all series to reach them when selected;
    Ui::MainWindow *ui;

    //Store from wich serie comes the current displayed plan
    int windowSerieNb[4];
    Plan windowDefaultPlan[4];
    Plan windowCurrentPlan[4];
    int windowNbImg[4];

//    vector<QImage*> Images; //store QImages for the current selected serie
//    vector<QImage*> Images2;
//    vector<QImage*> Images3;
//    vector<QImage*> Images4;


//    vector<uint8_t *> myPixelsZ;
//    vector<uint8_t *> myPixelsX;
//    vector<uint8_t *> myPixelsY;


    QButtonGroup *buttonGroup; //button to choose the displayed serie


    QGraphicsScene *myScene;
    QGraphicsScene *myScene2;
    QGraphicsScene *myScene3;
    QGraphicsScene *myScene4;

//    vector<int> WW;
//    vector<int> WC;

    // height and with are the default argument defined for the default plan of the Image
    int width;
    int height;

    // Those three value are used to recreate the views
//    int pixelXdepth;
//    int pixelYdepth;
//    int pixelZdepth;

//    double rescaleFactor;
//    vector <double> seriesRescaleFactor;

    int creation[4];
    int contrast[4];

//    map <string, int >nbOfFrame;
//    unsigned long  indexFrame;

    ReportWindow *report;

    Plan currentPlan;
   // vector< Plan> seriesPlan; // store the "inherant"/obvious plan for eaxh serie

    bool PlanContruction ; // set to true if possible to recreate other plan from the current images, sest to false othewise

    bool viewConnected;

    bool defaultPlan; // var to know if the plan of the current selected window is the default one
    // usefull to enable, disable plan construction button


    //QAction
    QAction *AxialAction;
    QAction *CoronalAction;
    QAction *SagittalAction;



    vector<Serie *> Series;
};






#endif // MAINWINDOW_H
