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



using namespace std;

enum Plan {Axial, Coronal, Sagittal};

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
    void addSerieButton(int serieNumber, char *serieDescription, char *date, int imgNb);

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
    void default_contrast();
    void minmax_contrast();
    void histo_contrast();
    void on_showReport_clicked();

    void callAxial();
    void callCoronal();
    void callSagittal();

private:
    int selectedWindow; // store the number of the current selected graphic window --> for mouse events
    bool invertGrayScale ; // boolean variable to sepcify if Grayscaled is inverted
    int *Index; // index array to store the number of the current displayed image according to each graphic window
    int series; //number of series in the directory
    string currentSerie; //name of the current selected serie

    map<string, vector <string>> allPath; // store all path for all series to reach them when selected;
    Ui::MainWindow *ui;


    vector<QImage*> Images; //store QImages for the current selected serie
    vector<QImage*> Images2;
    vector<QImage*> Images3;
    vector<QImage*> Images4;



    vector<uint8_t *> myPixelsZ;
    vector<uint8_t *> myPixelsX;
    vector<uint8_t *> myPixelsY;

    QButtonGroup *buttonGroup; //button to choose the displayed serie


    QGraphicsScene *myScene;
    QGraphicsScene *myScene2;
    QGraphicsScene *myScene3;
    QGraphicsScene *myScene4;

    int WW;
    int WC;

    int width;
    int height;
    int depth;

    int creation[4];
    int contrast[4];

    int nbOfFrame;
    unsigned long  indexFrame;

    ReportWindow *report;

    Plan currentPlan;
    map <string, Plan> seriesPlan; // store the "inherant"/obvious plan coresponding to the serie number
    bool PlanContruction ; // set to true if possible to recreate other plan from the current images, sest to false othewise

};






#endif // MAINWINDOW_H
