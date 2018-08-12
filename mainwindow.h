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

private:
    //Declaring private attribute
    Ui::MainWindow *ui;

    //The four scene that will contains the Images
    QGraphicsScene *myScene;
    QGraphicsScene *myScene2;
    QGraphicsScene *myScene3;
    QGraphicsScene *myScene4;

    //Vector storing the serie related to the selected DICOM File
    //Serie class: created to store image information about a given serie
    vector<Serie *> Series;

    //ReportWindow class: created to handle the displaying of a report
    ReportWindow *report;

    //button to choose the displayed serie
    QButtonGroup *buttonGroup;

    //Boolean to specifie if a function has been called during window creation or not
    bool windowCreation;

    //Store the number of the current selected graphic window
    //Usefull to know where to display a serie and for WheelEvent
    int selectedWindow;

    //Boolean variable to sepcify if grayscale should be inverted on Images
    bool invertGrayScale ;

    //Store the number of series found in the directory
    int series;

    //Store the number of the serie displayed in the selectedWindow
    int currentSerieNumber;
    //Store the associated name of the serie
    string currentSerie;
    //Storing the number of images of the currentSerie
    //If the number if to low, plan creation button are disabled
    int currentNbImages;
    //Storing the default plan of the serie displayed in the selectedWindow
    bool defaultPlan;
    //Storing the current plan of the serie displayed in the selectedWindow
    Plan currentPlan;


    //Storing respectively, Serie number, Default and CurrentPlan, Nb of Images for each of the four graphicsView of the window
    //Variable used to update the above "current" attributes
    int windowSerieNb[4];
    Plan windowDefaultPlan[4];
    Plan windowCurrentPlan[4];
    int windowNbImg[4];
    int windowRotation[4]; // four step rotation, 1,2,3,4 adding 45 degree each time

    //storing connection window for each window
    //windowConnection[i][i] is always true;
    //if windowConnection[0][1] is true, window1 and two are connected
    //windows are connected if they contains the same plan of the same serie
    bool windowConnection[4][4];


    //Declaring QAction as Main Window property for action that will have special treatment outside of the createButton() function
    QAction *AxialAction;
    QAction *CoronalAction;
    QAction *SagittalAction;
    QAction *Scroll;
    QAction *Link;

    int contrast[4];

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    // Function to construct the Main Window architecture of the interface
    // This include, adding the Images and dinamicall created button
    // This function will call in order the necessary subfunction
    void constructWindow(QString path);

    //This function will in store all relevant informations found in the DICOMDIR
    //In particular call for the creation of Serie element once a serie is found
    void processDicom(const char *dicomdirPath,char *filepath);

    //Creation of the widget to display patient information
    void setPatientInfo(char *studydesc, char * date, char* patientName, char* birthdate);

    //Dinamic creation of button to access different series
    void addSerieButton(Serie* serie);

    //Return the default plan calculated from the given orientation
    Plan findSeriePlan(const char * orientation);

    //Treat DICOM given information to render a number
    double getPixelSpacingNb(const char* pixelArray);

    //Process to the creation of Images from the "processDicom" information
    //Will call for "bujildViews" and "displayInScene" function
    void createDefaultPlan();

    //Function only called once, during the creation of the window
    //Initializing QGraphicsScene and graphicsView for the window
    void buildViews();

    //Function to display the image given as a parameter in the "selectedWindow"
    void displayInScene(QPixmap img);

    void displayInScene(QPixmap img, int scene);


    //Calling for Serie "getCurrentImg(Plan plan)", to display the relevant image in the selectedWindow
    void displayAxialPlan();
    void displayCoronalPlan();
    void displaySagittalPlan();

    //Function to set up the button design of the window
    //Also binding button with their actions
    void createButtons();

    //Update all "current" variable to their value according to the selectedWindow and the serie number
    void updateWindowInfo();

    //Function called if "linkedViews" button is activated
    //Function to set up information before drawing onthe screen
    void paintLinkedLines();

    //Function to draw the line on the related plan/images while scrolling in an other one
    void paintOnScene(QPixmap &pixmap, int sceneNb, int beginX,int beginY,int endX, int endY );

    //Active Image rotation depending on the rotationIndice
    void rotate(int rotationIndice);

    void updateContrast();

    void updateWindowConnection();




private slots:
    //Private slots are action call on button pressed

    //Change the selected window on mousePress
    void mousePressEvent(QMouseEvent *e);

    //Scrolling images of the serie if Scroll button is activated
    void wheelEvent(QWheelEvent *event);

    //Click event for series button
    //Will display the serie according to his corresponding number
    void buttonInGroupClicked(QAbstractButton *);

    //Show/hide advanced settings
    void show_advanced();
    void hide_advanced();

    //Set invertGrayScale to true
    void invert_grayscale();

    //Show, hide graphicsView depending on the corresponding number
    void display_one_window();
    void display_two_window();
    void display_four_window();

    //Zoom in/out on the image
    void zoom_plus();
    void zoom_minus();

    //call for left/right image rotation
    void rotate_left();
    void rotate_right();

    //set contrst to the corresponding value
    void default_contrast();
    void minmax_contrast();
    void histo_contrast();

    //Open the report window
    void on_showReport_clicked();

    //Calling for corresponding displayPlan function
    void callAxial();
    void callCoronal();
    void callSagittal();


    void callTest();

    void link_views();




};


#endif // MAINWINDOW_H
