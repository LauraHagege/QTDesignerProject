#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialog.h"
#include <QLabel>
#include <QMessageBox>
#include <iostream>

#include <QGuiApplication>
#include <QApplication>
#include <iostream>
//#undef UNICODE
//#undef _UNICODE
//#include <dcmtk/config/osconfig.h>
//#include <dcmtk/dcmdata/dctk.h>
//#include <dcmtk/dcmimgle/dcmimage.h>
#include <QPixmap>
#include <QLabel>
#include <QVBoxLayout>
#include <QGraphicsItem>
#include <QMap>
#include <QRgb>
#include <QColor>
#include <stdlib.h>
#include <QButtonGroup>
#include <QTransform>




using namespace std;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    buttonGroup = new QButtonGroup(this);
    connect(buttonGroup, SIGNAL(buttonClicked(QAbstractButton*)), SLOT(buttonInGroupClicked(QAbstractButton*)));

    report = new ReportWindow();
    Access=false;

    //by default one serie is displayed
    nbDisplayedSerie=1;

    for(int i=0 ; i<4; i++){
        contrast[i]=Default;
        windowSerieNb[i]=-1;
        windowCurrentPlane[i]=Unknown;
        windowZoom[i]=0;
        windowFlag[i]=false;
        windowRef[i]=false;
    }

}

//********************************************************************************//
//---------------------ALL PUBLIC FUNCTION FOLLOWING -----------------------------//
//********************************************************************************//

MainWindow::~MainWindow()
{
    delete ui;
}


//--------------------------------------------------------------------------//
//----------------------------------  CHECK PIN  -----------------=---------//
//--------------------------------------------------------------------------//
void MainWindow::checkPin(char *studyPath, int studyNb, char *studyname, char *dicomdirpath){
    windowCreation=true;

    strcpy(dicomdirPath,dicomdirpath);

    strcpy(studyName,studyname);

    //processing Qtring path, creating string element to process DicomFile reading
//    QByteArray ba = studyPath.toLatin1();
//    const char *dicomdirPath = ba.data();

    int size =(int) strlen(dicomdirPath);
    //char *filepath=new char[130];

    studyNumber=studyNb;

    for(int i=0; i<size-8; i++){
        absolutefilepath[i]=dicomdirPath[i];
    }
    absolutefilepath[size-8]='\0';


    int study=0;

    DcmDicomDir myDir(dicomdirPath);

    //Declaring var for following file of the folder
    DcmDirectoryRecord *   root = &(myDir.getRootRecord());
    DcmDirectoryRecord *   PatientRecord = NULL;
    DcmDirectoryRecord *   StudyRecord = NULL;
    OFString            tmpString;


    if(root != NULL){
        //First depth give goes in the DICOMDAT folder
        //Allows to get information about the patient
        //While loop  if there are more than one subfile
        while (((PatientRecord = root->nextSub(PatientRecord)) != NULL)){
            while (((StudyRecord = PatientRecord->nextSub(StudyRecord)) != NULL) && study<studyNumber){
                study +=1;
                char *birthdate=new char[11];
                if(studyNumber==study){
                    if (StudyRecord->findAndGetOFString(DCM_PatientBirthDate, tmpString).good()) {
                        int count=0;
                        for(int i=0 ; i< 8; i++){
                            if(i==4 || i ==6 ){
                                birthdate[count]= '\\';
                                count ++;
                            }
                            birthdate[count]=tmpString[i];

                            count ++;
                        }
                        birthdate[count]='\0';
                    }else{
                        strcpy(birthdate,string("unknown").c_str());
                    }

                    cout << "pin "<< birthdate << endl;
                    strcpy(pinAccess,birthdate);

                    //checking pin access before displaying datas
                    if(strcmp(birthdate,string("unknown").c_str())){
                        //pinDialog = new DialogPinAccess ;
                        //pinDialog.setPassword(birthdate);
                        pinDialog.setModal(true);

                        connect(&pinDialog,SIGNAL(checkPassword(char *)),this,SLOT(validPassword(char*)));

                         pinDialog.exec();
                    }
                    else{
                        //if no birthdate found, process data without pin access
                        processData();
                    }
                }

            }
        }

    }

}

//--------------------------------------------------------------------------//
//---------------------------------- GET ACCESS  ---------------------------//
//----------------------If no access granted window won't show-------------//
//-------------------------------------------------------------------------//
bool MainWindow::getAccess(){
    return Access;
}


//--------------------------------------------------------------------------//
//-----------------------------------  PROCESS DATA ------------------------//
//------------ Calling all related function to create the main window-------//
//-------------------------------------------------------------------------//
void MainWindow::processData(){
    cout << "processData" << endl;
    processDicom();

    display_one_window(); // by default show only one window
    createDefaultPlane();

    //call function to create button architecture
    createButtons();

    //call for report window creation
    report->render_report(absolutefilepath,studyName);
}




//-------------------------------------------------------------------------------//
//------------------------------  PROCESS DICOM FUNCTION ------------------------//
//----- Going down DICOMDIR tree structure to get all required informations -----//
//-------------------------------------------------------------------------------//

void MainWindow::processDicom(){
    cout << "processDicom" << endl;
    //counter for the number of study
    int study=0;
    //counter for the number of series found in the file
    series=0;
    //information for the first serie to be displayed on the main window
    currentSerie = "Series2";
    currentSerieNumber=2;

    //cout <<"path " << filepath << endl;

    //Open DICOMDIR with given path
    //DcmDicomDir class to deal with DicomDir element
    DcmDicomDir myDir(dicomdirPath);

    //Declaring var for following file of the folder
    DcmDirectoryRecord *   root = &(myDir.getRootRecord());
    DcmDirectoryRecord *   PatientRecord = NULL;
    DcmDirectoryRecord *   StudyRecord = NULL;
    DcmDirectoryRecord *   SeriesRecord = NULL;
    DcmDirectoryRecord *   FileRecord = NULL;
    OFString            tmpString;



    //Local variable declaration
    int nbFrame=0;
    int WC1,WW1;
    double rescale =1;
    Plane seriePlane = Unknown;

    //Going down DICOMDIR tree structur to get the path to all images stored in the given Directory
    if(root != NULL){
        //First depth give goes in the DICOMDAT folder
        //Allows to get information about the patient
        //While loop  if there are more than one subfile
        while (((PatientRecord = root->nextSub(PatientRecord)) != NULL)){
            while (((StudyRecord = PatientRecord->nextSub(StudyRecord)) != NULL) && study<studyNumber){
                cout << "NEW STUDY " << endl;
                study +=1;

                //local var to store patient information
                char *date=new char[11];
                char *studydesc=new char[30];
                char *patientName=new char[50];
                char *birthdate=new char[11];


                //dealing only with the asked study
                if(studyNumber==study){

                    //Get Patient Name
                    if (StudyRecord->findAndGetOFString(DCM_PatientName, tmpString).good()) {
                      cout << "PatientName : " << tmpString.c_str() << endl ;
                        strcpy(patientName,tmpString.c_str());
                        int count=0;
                        while(patientName[count] != '\0'){
                            if(patientName[count] == '^')
                                patientName[count] = ' ';
                            count ++;
                        }
                    }
                    else{
                        string noname= "No patient name found";
                        strcpy(patientName,noname.c_str());
                    }

                    //Patient Birthdate information
                    if (StudyRecord->findAndGetOFString(DCM_PatientBirthDate, tmpString).good()) {
    //                    cout << "birthdate 1 : " << tmpString.c_str() << endl ;
                        int count=0;
                        for(int i=0 ; i< 8; i++){
                            if(i==4 || i ==6 ){
                                birthdate[count]= '\\';
                                count ++;
                            }
                            birthdate[count]=tmpString[i];

                            count ++;
                        }
                        birthdate[count]='\0';
                    }else{
                        strcpy(birthdate,string("unknown").c_str());
                    }

                    //Study Description
                    if (StudyRecord->findAndGetOFString(DCM_StudyDescription, tmpString).good()) {
                        strcpy(studydesc,tmpString.c_str());
                    }else{
                        string nodesc= "No description found";
                        strcpy(studydesc,nodesc.c_str());
                    }

                    //Study Date
                    if (StudyRecord->findAndGetOFString(DCM_StudyDate, tmpString).good()){
                        int count=0;
                        for(int i=0 ; i< 8; i++){
                            if(i==4 || i ==6 ){
                                date[count]= '\\';
                                count ++;
                            }
                            date[count]=tmpString[i];
                            count ++;
                        }
                        date[count]='\0';
                    }

                    //Set Patient information to be displayed on the screen
                    setPatientInfo(studydesc,date,patientName,birthdate);

                    //Going down the tree to find the series
                    while (((SeriesRecord = StudyRecord->nextSub(SeriesRecord)) != NULL)){
                        //local variable, vector to store paths to files for each serie
                        vector<string> paths;

                        // counter for images within a serie
                        int count =0;

                        //serie reference is the SRS* number
                        //usefull to access the flagged data
                        char serieRef[9];

                        //storing serie description
                        char *desc=new char[100];

                        //going down the tree to find images from each serie
                        while((FileRecord = SeriesRecord->nextSub(FileRecord)) != NULL){
                            //store full path to each image
                            string fullpath;

                            //increment image counter, storing the number of images in the serie (multi frame or not)
                            count +=1;
                            //Dicom images File are File containing Image information = ImageRecord
                            //This is the same depth

                            //DCM_ReferencedFileID give the path to the Image from the current DICOM directory
                            if(FileRecord->findAndGetOFStringArray(DCM_ReferencedFileID,tmpString).good()){
                                //add image inner path to file path to get the full path

                                fullpath = absolutefilepath + string(tmpString.c_str());
                            }



                            //Check file status, if not EIS_Normal the file is not an Image
                            //If not normal status the "parent" is not a serie

                            if(count ==1){
                                // If the file is not an image, don't store it, ie don't create a serie
                                if (DicomImage(fullpath.c_str()).getStatus() != EIS_Normal){
                                    count =0;
                                }

                                //get serie description
                                if (SeriesRecord->findAndGetOFString(DCM_SeriesDescription, tmpString).good()){
                                    strcpy(desc,tmpString.c_str());
                                    cout << tmpString.c_str() << " " << endl;
                                    if(!strcmp(desc,string("Patient Protocol").c_str()))
                                        count=0;
                                }
                            }

                            // add image path to the local vector
                            paths.push_back(fullpath);

                            //storing basic information common to all images of the serie
                            //count <= 3 is a security in case the first image is just a "reference plan"
                            if(count <=3 && count >0){
                                if(FileRecord->findAndGetOFString(DCM_WindowCenter,tmpString).good()){
                                    WC1=atoi(tmpString.c_str());
                                }

                                if(FileRecord->findAndGetOFString(DCM_WindowWidth,tmpString).good()){
                                    WW1= atoi(tmpString.c_str());

                                }

                                if(FileRecord->findAndGetOFStringArray(DCM_NumberOfFrames,tmpString).good()){
                                     //cout << "Nb of Frames " << tmpString.c_str() << endl;
                                     nbFrame = atoi(tmpString.c_str());
                                }

                                if(FileRecord->findAndGetOFStringArray(DCM_ImageOrientationPatient,tmpString).good()){
                                    seriePlane = findSeriePlane(tmpString.c_str());
                                }
                                if(FileRecord->findAndGetOFString(DCM_SliceThickness,tmpString).good()){
                                    rescale = atoi(tmpString.c_str()) ;
                                }
                                if(FileRecord->findAndGetOFStringArray(DCM_PixelSpacing,tmpString).good()){
                                    //use pixel spacing and slice thickness to get the final rescale factor
                                    //rescale factor used for other plan contruction
                                    double pixelspacing = getPixelSpacingNb(tmpString.c_str());
                                    rescale = rescale*pixelspacing;
                                }
                                //processing to get the number of the serie
                                if(count ==1){
                                    int counter =0;
                                    int size = strlen(fullpath.c_str());
                                    for(int i=size-17; i<size-9 ; i++){
                                        serieRef[counter]=fullpath.at(i);
                                        counter ++;
                                    }
                                    serieRef[counter]='\0';
                                    cout <<"serie Ref" << serieRef << endl;
                                }
                            }
                        }

                        //count !=0 means the folder contain DICOM images so this is a serie
                        if(count !=0){
                            series +=1;
                            //cout << "series nb " <<series << "with "<< count <<" images" << endl;


                            //create a new serie of images
                            Serie *serie = new Serie(series, studyName,serieRef, seriePlane,absolutefilepath,paths, nbFrame,rescale, desc, WW1, WC1);


                            //create a button for the new serie
                            addSerieButton(serie);

                            //add the serie to the current window
                            Series.push_back(serie);


                        }
                    }

                }
            }
        }
    }
}


//----------------------------------------------------------//
//----- CREATION OF THE WIDGET TO DISPLAY PATIENT INFO -----//
//----------------------------------------------------------//
void MainWindow::setPatientInfo(char *studydesc, char *date, char *patientName, char *birthdate){
    string str = string("<b>Personal Information:</b>") + string("<br>") + string(studydesc) + string("<br>") +  string(date) +  string("<br>") +  string(patientName) +  string("<br>") +  string("Birthdate: ") +  string(birthdate);

    QLabel *perso =new  QLabel( str.c_str()); //+ studydesc );//+ "\n" + date + "\n" + patientName + "\n" + "Birthdate: " + birthdate);
    perso->setMaximumHeight(120);
    ui->PersonalInfo->addWidget(perso);
}


//-------------------------------------------------------------//
//----- ADD A BUTTON FOR EACH SERIE ON THE CURRENT WINDOW -----//
//-------------------------------------------------------------//
void MainWindow::addSerieButton(Serie *serie){
    //creating/adding button for each serie in the serieWidget
    QPushButton *button = new QPushButton(this);
    button->setCheckable(true);

    if(serie->getId() ==  currentSerieNumber)
        button->setChecked(true);

//     display first Image of the serie near the button
//    DicomImage *serieImg;
//    int nbFrame = serie->getNbFrames();

//    if(nbFrame ==0)
//        serieImg = new DicomImage(serie->getPath(0).c_str());
//    else {
//        serieImg = new DicomImage(serie->getPath(0).c_str(),0,0,1);
//    }

//    serieImg->setWindow(serie->getWC(),serie->getWW());
//    uint8_t * pixel = (uint8_t *)serieImg->getOutputData(8);

//    QImage *img = new QImage(pixel, serieImg->getWidth(), serieImg->getHeight(),QImage::Format_Indexed8);
//    QPixmap pixmap = QPixmap::fromImage( QImage(img->scaled(QSize(50,50), Qt::IgnoreAspectRatio,Qt::SmoothTransformation)) );
//    QIcon ButtonIcon(pixmap);

//    button->setIcon(ButtonIcon);
//    button->setIconSize(pixmap.rect().size());


    button->setText(tr(serie->getName()));

    button->setProperty("Id",serie->getId());
    button->setVisible(true);
    buttonGroup->addButton(button);

    QLabel *SerieDesc= new QLabel(this);

    SerieDesc->setMaximumHeight(50);
    SerieDesc->setText(serie->getDescription()) ;

    QFrame *SerieFrame = new QFrame(this);
    SerieFrame->setFrameShape(QFrame::HLine);
    SerieFrame->setFrameShadow(QFrame::Sunken);
    SerieFrame->setLineWidth(1);

    ui->SeriesLayout->addWidget(button);
    ui->SeriesLayout->addWidget(SerieDesc);
    ui->SeriesLayout->addWidget(SerieFrame);

}

//---------------------------------------------------------------------------------//
//----- DETERMINE THE DEFAULT PLAN FOR A GIVEN SERIE ------------------------------//
//----- DEFAULT PLAN meaning the one created without "treatment on DICOM files ----//
//---------------------------------------------------------------------------------//
Plane MainWindow::findSeriePlane(const char *orientation){

    int count=0;
    int str=0;
    int coordinate=0;
    int axial=2;
    int coronal=2;
    int sagittal=2;

    double ref=pow(10, -10);

    double number=0;
    char *nb= new char[30];

    while(orientation[str] != '\0'){
        if(orientation[str] == '\\' || orientation[str+1]=='\0' ){
            if(orientation[str+1]=='\0'){
                nb[count]=orientation[str];
                count ++;
            }

            nb[count]='\0';
            number=stod(nb);
            count =0;

            if(abs(number)<ref){
                if(coordinate==2 || coordinate==5){
                    axial-=1;
                }
                else if(coordinate==0 || coordinate==3){
                    coronal-=1;
                }
                else if(coordinate==1 || coordinate==4){
                    sagittal-=1;
                }
            }
            coordinate+=1;
        }else{
                nb[count]=orientation[str];
                count ++;
        }
        str +=1;
    }


    if(axial==0){
        cout << "axial" << endl;
        return Axial;
    }else if(coronal==0){
        cout << "coronal" << endl;
        return Coronal;
    }else if(sagittal==0){
        cout << "sagittal" << endl;
        return Sagittal;
    }else {
        cout << "other" << endl;
        return Unknown;
    }
}

//----------------------------------------------------//
//- Treat DICOM given information to render a number -//
//----------------------------------------------------//
double MainWindow::getPixelSpacingNb(const char* pixelArray){
    char *nb= new char[30];
    double number=0;

    int count=0;

    while(pixelArray[count] != '\\'){
        nb[count]=pixelArray[count];
        count ++;
    }
    nb[count]='\0';
    number=stod(nb);

    delete nb;
    return number;
}


//----------------------------------------------------------------------------------//
//----------------------------CREATE DEFAULT PLAN ----------------------------------//
//------------------------ Render Images according to the default plan -------------//
//----------------------------------------------------------------------------------//
void MainWindow::createDefaultPlane(){
    Serie *serie=Series[currentSerieNumber-1];
    //CurrentSerieNumber contains the number/id of the serie displayed in the current selected window

    // currentPlan stores the serie plan displayed relatively to the selected window
    // by default this is the default plan of the serie
    currentPlane = serie->getdefaultPlane();

    //currentNbImages is the number of current displyed serie
    currentNbImages = serie->getNbImages();

    //local var to treat DICOM Images
    //Information relative to the images will be stored add Pixel
    vector<DicomImage*> DicomImages;

    //local variable containing the number of Frames
    int nbFrame =serie->getNbFrames();

    //Creating DICOM Images depending on the type:
    //Single frame Images are stored separately
    //Multiframe are stored in one DicomImage element


    for(int i=0; i< currentNbImages ; i++){
        if(nbFrame ==0)
            DicomImages.push_back(new DicomImage(serie->getPath(i).c_str()));
        else {
            cout << "path " << serie->getPath(i).c_str() << endl;
             for(int j=0; j<nbFrame ; j++)
                DicomImages.push_back(new DicomImage(serie->getPath(i).c_str(),0,j,1));
             break;
        }


    }


    serie->clearPixels();

    // height and with are the default argument defined for the default plan of the Image
    int width = DicomImages[0]->getWidth();
    int height =  DicomImages[0]->getHeight();

    //cout << "width " << width <<" height " << height << endl;

    // Checking status to know if all DICOM Images have been created without any trouble
    int check=true;
    for(int i=0; i<DicomImages.size(); i++){
        if(DicomImages[i] == NULL)
            check=false;
        else if(!(DicomImages[i]->getStatus() == EIS_Normal))
            check=false;
    }


    //Process images to render pixels
    if (check){

        for(int i=0; i<DicomImages.size(); i++){
            //Contrast display depends on the selected one for each window
            if(!serie->isBuilt()){
                serie->setDepths(width,height);
                serie->setcontrast(Default);

                DicomImages[i]->setWindow(serie->getWC(),serie->getWW());
                // DicomImages[i] ->setMinMaxWindow();

            }
            else{
                switch(serie->getcontrast()){
                case Default:
                case DefaultInverted:
                    DicomImages[i]->setWindow(serie->getWC(),serie->getWW());
                    break;
                case MinMax:
                case MinMaxInverted:
                    DicomImages[i] ->setMinMaxWindow();
                    break;
                case Histo:
                case HistoInverted:
                    DicomImages[i] ->setHistogramWindow();
                    break;
                }

            }


            //Getting pixel information from the DicomImage
            // 8 is the number of pixel per sample

            uint8_t* pixelData = (uint8_t *)(DicomImages[i]->getOutputData(8));

            //storing the pixel for the current serie
            //according to the default plan of the serie pixel will be stored in a different vector
            serie->storePixel((uint8_t *)pixelData);
       }
   }
   else
      cerr << "Error: cannot load DICOM image (" << DicomImage::getString(DicomImages[0]->getStatus()) << ")" << endl;


    //Call for window scene creation
    if(windowCreation){
        buildViews();
        windowCreation=false;
    }

    displayInScene(serie->getCurrentImg(currentPlane));

    //If the serie contains multiple images, the other corresponding plan can be created
    //The function is called by default and stored
    //As asked plan will only need to be rendered
   if(currentNbImages>50 && serie->getdefaultPlane()!=Unknown)
        Series[currentSerieNumber-1]->constructPlanes();

}

//-----------------------------------------------------------------------//
//--------------------------- BUILD VIEWS -------------------------------//
//----- Initializing QGraphicsScene and graphicsView for the window -----//
//-----------------------------------------------------------------------//
void MainWindow::buildViews(){

    myScene= new QGraphicsScene(this);
    myScene2= new QGraphicsScene(this);
    myScene3= new QGraphicsScene(this);
    myScene4= new QGraphicsScene(this);

    //while building for the first time the selected window is the first and only one displayed
    ui->graphicsView->setFrameStyle(3);

    ui->graphicsView->setStyleSheet("color:orange");
    ui->graphicsView_2->setStyleSheet("color:black");
    ui->graphicsView_3->setStyleSheet("color:black");
    ui->graphicsView_4->setStyleSheet("color:black");

    ui->graphicsView->setBackgroundBrush(QBrush(Qt::black));
    ui->graphicsView_2->setBackgroundBrush(QBrush(Qt::black));
    ui->graphicsView_3->setBackgroundBrush(QBrush(Qt::black));
    ui->graphicsView_4->setBackgroundBrush(QBrush(Qt::black));

    ui->graphicsView->setScene(myScene);
    ui->graphicsView_2->setScene(myScene2);
    ui->graphicsView_3->setScene(myScene3);
    ui->graphicsView_4->setScene(myScene4);
}

//----------------------------------------------------------------------------------//
//----------------------------- DISPLAY IN SCENE -----------------------------------//
//--- Function to display the image given as a parameter in the "selectedWindow" ---//
//----------------------------------------------------------------------------------//
void MainWindow::displayInScene(QPixmap img){
    displayInScene(img,selectedWindow);

}

void MainWindow::displayInScene(QPixmap img, int window){
    Serie *serie = Series[currentSerieNumber-1];
    //Set the information if the window wasnt containing any Serie
    if(windowSerieNb[selectedWindow-1]==-1 || windowSerieNb[selectedWindow-1] != currentSerieNumber ){
        windowSerieNb[selectedWindow-1]= currentSerieNumber;
        windowDefaultPlane[selectedWindow-1]= serie->getdefaultPlane();
        windowCurrentPlane[selectedWindow-1]= serie->getdefaultPlane();
        windowNbImg[selectedWindow-1]=  serie->getNbImages();
    }

    //This is a "casual" update
    //All serie must store information to know in which plan is displayed in which window
    //Variable used to link views of the same serie together
    serie->setPlaneWindows(windowSerieNb, windowCurrentPlane);


    //If views are marked linked
    if(serie->getViewLinked())
        paintLinkedLines();

    switch(window){
    case 1:
        myScene= new QGraphicsScene(this);
        myScene->addPixmap(img);
        ui->graphicsView->setScene(myScene);
        break;
    case 2:
        myScene2= new QGraphicsScene(this);
        myScene2->addPixmap(img);
        ui->graphicsView_2->setScene(myScene2);
        break;
    case 3:
        myScene3= new QGraphicsScene(this);
        myScene3->addPixmap(img);
        ui->graphicsView_3->setScene(myScene3);
        break;
    case 4:
        myScene4= new QGraphicsScene(this);
        myScene4->addPixmap(img);
        ui->graphicsView_4->setScene(myScene4);
        break;
    }

    updateWindowConnection();
}


//----------------------------------------------------------------------//
//--------------------------- DISPPLAY SAGITTAL PLAN  ------------------//
//----------------------------------------------------------------------//
void MainWindow::displaySagittalPlane(){
    windowCurrentPlane[selectedWindow-1]=Sagittal;
   displayInScene(Series[currentSerieNumber-1]->getCurrentImg(windowCurrentPlane[selectedWindow-1]));

   updateWindowInfo();
}

//------------------------------------------------------------------//
//--------------------------- DISPLAY AXIAL PLAN  ------------------//
//------------------------------------------------------------------//
void MainWindow::displayAxialPlane(){
   windowCurrentPlane[selectedWindow-1]=Axial;

    displayInScene(Series[currentSerieNumber-1]->getCurrentImg(windowCurrentPlane[selectedWindow-1]));
    updateWindowInfo();

}


//--------------------------------------------------------------------//
//--------------------------- DISPLAY CORONAL PLAN  ------------------//
//--------------------------------------------------------------------//
void MainWindow::displayCoronalPlane(){

  //  cout <<"current plan " << windowCurrentPlane[selectedWindow-1] << "selected window "<< selectedWindow<<  endl;

    windowCurrentPlane[selectedWindow-1]=Coronal;
    displayInScene(Series[currentSerieNumber-1]->getCurrentImg(windowCurrentPlane[selectedWindow-1]));

    updateWindowInfo();
}

//--------------------------------------------------------------------//
//---------------------------- CREATE BUTTONS  -----------------------//
//--------------------------------------------------------------------//
void MainWindow::createButtons(){
    Serie * serie= Series[currentSerieNumber-1];

    ui->mainToolBar->setIconSize(QSize(33,33));
    ui->AdvancedSettings->setIconSize(QSize(33,33));


    //Adding button to the toolbar
    QAction *ZoomPlus= new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/zoom-plus.png"),"Zoom",this);
    ui->mainToolBar->addAction(ZoomPlus);

    connect(ZoomPlus, SIGNAL(triggered(bool)), SLOT(zoom_plus()));

    QAction *ZoomMinus= new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/zoom-minus.png"),"Dezoom",this);
    ui->mainToolBar->addAction(ZoomMinus);

    connect(ZoomMinus, SIGNAL(triggered(bool)), SLOT(zoom_minus()));

    ui->mainToolBar->addSeparator();

    Flag = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/flag.png"),"Get me to the relevant image", this);
    ui->mainToolBar->addAction(Flag);
    connect(Flag, SIGNAL(triggered(bool)), SLOT(showFlagged()));
    Flag->setCheckable(true);

    bool flag=serie->hasFlag();
    if(!flag)
        Flag->setEnabled(false);

    ui->mainToolBar->addSeparator();

    QAction *Display1Window = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/w1.png"),"Show only one window", this);
    ui->mainToolBar->addAction(Display1Window);
    connect(Display1Window,SIGNAL(triggered(bool)),this,SLOT(display_one_window()) );

    QAction *Display2Window = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/w2.png"),"Show only one window", this);
    ui->mainToolBar->addAction(Display2Window);
    connect(Display2Window,SIGNAL(triggered(bool)),this,SLOT(display_two_window()) );

    QAction *Display4Window = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/w4.png"),"Show only one window", this);
    ui->mainToolBar->addAction(Display4Window);
    connect(Display4Window,SIGNAL(triggered(bool)),this,SLOT(display_four_window()) );

    ui->mainToolBar->addSeparator();

    AxialAction = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/axial.png"),"Show top/bottom view (Axial)", this);
    ui->mainToolBar->addAction(AxialAction);
    connect(AxialAction, SIGNAL(triggered(bool)), SLOT(callAxial()));

    CoronalAction = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/coronal.png"),"Show front/back view (Coronal)", this);
    ui->mainToolBar->addAction(CoronalAction);
    connect(CoronalAction, SIGNAL(triggered(bool)), SLOT(callCoronal()));

    SagittalAction = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/sagittal.png"),"Show left/right view (Sagittal)", this);
    ui->mainToolBar->addAction(SagittalAction);
    connect(SagittalAction, SIGNAL(triggered(bool)), SLOT(callSagittal()));


    if(currentNbImages <50 || !serie->getMultiplane()){
        AxialAction->setEnabled(false);
        SagittalAction->setEnabled(false);
        CoronalAction->setEnabled(false);
    }else {
        switch(currentPlane){
        case Axial:
            AxialAction->setEnabled(false);
            break;
        case Coronal:
            CoronalAction->setEnabled(false);
            break;
        case Sagittal:
            SagittalAction->setEnabled(false);
            break;
        default:
            AxialAction->setEnabled(false);
            SagittalAction->setEnabled(false);
            CoronalAction->setEnabled(false);
            break;
        }

    }

    ui->mainToolBar->addSeparator();

    Link = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/link.png"),"Link views scrolling", this);
    ui->mainToolBar->addAction(Link);
    connect(Link,SIGNAL(triggered(bool)),this,SLOT(link_views()) );
    Link->setCheckable(true);

    ui->mainToolBar->addSeparator();

    QAction *Label = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/label.png"),"Show labels", this);
    ui->mainToolBar->addAction(Label);

    ui->mainToolBar->addSeparator();

    Scroll = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/scroll.png"),"Scroll Images", this);
    ui->mainToolBar->addAction(Scroll);
    Scroll->setCheckable(true);
    Scroll->setChecked(true);

    ui->mainToolBar->addSeparator();

    Ref = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/examples.png"),"Show me normal images", this);
    ui->mainToolBar->addAction(Ref);
    connect(Ref, SIGNAL(triggered(bool)), SLOT(showRef()));

    Ref->setCheckable(true);

    bool ref=serie->hasRef();
    if(!flag)
        Ref->setEnabled(false);

    ui->mainToolBar->addSeparator();

    QAction *Reset = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/reset.png"),"Reset settings in the active window", this);
    ui->mainToolBar->addAction(Reset);
    connect(Reset,SIGNAL(triggered(bool)),this,SLOT(reset_settings()) );


    ui->mainToolBar->addSeparator();

    QAction *Advanced = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/add.png"),"Advanced settings", this);
    ui->mainToolBar->addAction(Advanced);
    connect(Advanced,SIGNAL(triggered(bool)),this,SLOT(show_advanced()) );

    ui->mainToolBar->addSeparator();


    //creating advanced setting tool bar
    ui->AdvancedSettings->setVisible(false);

    InvertContrast = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/grayscale.png"),"Invert Grayscale", this);
    ui->AdvancedSettings->addAction(InvertContrast);
    connect(InvertContrast,SIGNAL(triggered(bool)),this,SLOT(invert_grayscale()) );
    InvertContrast->setCheckable(true);
    InvertContrast->setChecked(false);

    //setting invert var to zero = normal grayscale displayed
    //invertGrayScale=0;

    DefaultContrast = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/defaultcontrast.png"),"Set default contrast", this);
    ui->AdvancedSettings->addAction(DefaultContrast);
    connect(DefaultContrast,SIGNAL(triggered(bool)),this,SLOT(default_contrast()) );
    DefaultContrast->setCheckable(true);
    DefaultContrast->setChecked(true);

    MinMaxContrast = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/minmaxcontrast.png"),"Set darker contrast", this);
    ui->AdvancedSettings->addAction(MinMaxContrast);
    connect(MinMaxContrast,SIGNAL(triggered(bool)),this,SLOT(minmax_contrast()) );
    MinMaxContrast->setCheckable(true);
    MinMaxContrast->setChecked(false);


    HistoContrast = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/histogramcontrast.png"),"Set brighter contrast", this);
    ui->AdvancedSettings->addAction(HistoContrast);
    connect(HistoContrast,SIGNAL(triggered(bool)),this,SLOT(histo_contrast()) );
    HistoContrast->setCheckable(true);
    HistoContrast->setChecked(false);

    QAction *AddStudy = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/study.png"),"Compare my other study", this);
    ui->AdvancedSettings->addAction(AddStudy);

    QAction *Measure = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/measure.png"),"Measure", this);
    ui->AdvancedSettings->addAction(Measure);

    QAction *MeasureAngles = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/measureangle.png"),"Measure Angles", this);
    ui->AdvancedSettings->addAction(MeasureAngles);

    QAction *Anonymize = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/user.png"),"Anonymize my study", this);
    ui->AdvancedSettings->addAction(Anonymize);

    QAction *Share = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/share.png"),"Share my study", this);
    ui->AdvancedSettings->addAction(Share);

    QAction *Hide = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/remove.png"),"Hide Settings", this);
    ui->AdvancedSettings->addAction(Hide);
    connect(Hide,SIGNAL(triggered(bool)),this,SLOT(hide_advanced()) );


    ui->Informations->setLayout(ui->SeriesLayout);



}

//---------------------------------------------------------------//
//------------------------ UPDATE WINDOW INFO  ------------------//
//---------------------------------------------------------------//
void MainWindow::updateWindowInfo(){
   cout << "updateWindowinfo" << endl;
    if(currentSerieNumber != -1)
        currentNbImages=Series[currentSerieNumber-1]->getNbImages();
    else
        currentNbImages=0;

    currentPlane=windowCurrentPlane[selectedWindow-1];

    updatePlaneButton();

    cout << "1" << endl;

    //update the number of different series displayed;
//    int nb=4;
//    for(int i=0; i<4; i++){
//        if(windowSerieNb[i]==-1)
//            nb-=1;
//        else {
//            for(int j=i+1; j<4; j++){
//                if(windowSerieNb[i] == windowSerieNb[j])
//                    nb-=1;
//            }
//        }
//    }
//    nbDisplayedSerie = nb;


//    for(int i=0; i<4 ; i++){
//        cout << "serie nb " << windowSerieNb[i] << endl;
//        cout << "plan " << windowCurrentPlan[i] << endl;
//    }



    updateContrastButton();
    cout << "2" << endl;

    updateWindowConnection();


    updateFlagButton();
    cout << "4" << endl;

    updateRefButton();

    cout << "2" << endl;
    //cout << "there are currently " << nbDisplayedSerie << " different series displayed on screen" << endl;

}

void MainWindow::updatePlaneButton(){
    if(currentSerieNumber ==-1){
        AxialAction->setEnabled(false);
        SagittalAction->setEnabled(false);
        CoronalAction->setEnabled(false);
    }
    else if(currentNbImages <50 || currentPlane ==FlagImg || currentPlane== RefImg){
        AxialAction->setEnabled(false);
        SagittalAction->setEnabled(false);
        CoronalAction->setEnabled(false);
    }else {
        AxialAction->setEnabled(true);
        SagittalAction->setEnabled(true);
        CoronalAction->setEnabled(true);

        switch(currentPlane){
        case Axial:
            AxialAction->setEnabled(false);
            break;
        case Coronal:
            CoronalAction->setEnabled(false);
            break;
        case Sagittal:
            SagittalAction->setEnabled(false);
            break;
        default:
            AxialAction->setEnabled(false);
            SagittalAction->setEnabled(false);
            CoronalAction->setEnabled(false);
            break;
        }
    }

}

void MainWindow::updateFlagButton(){
    if(currentSerieNumber != -1){
        Serie *serie = Series[currentSerieNumber-1];

        bool flag=serie->hasFlag();
        if(flag){
            Flag->setEnabled(true);
            if(windowFlag[selectedWindow-1])
                Flag->setChecked(true);
            else
                Flag->setChecked(false);
        }
        else
           Flag->setEnabled(false);
    }

}

void MainWindow::updateRefButton(){
    if(currentSerieNumber != -1){
        Serie *serie = Series[currentSerieNumber-1];

        bool ref=serie->hasRef();

        if(ref){
            Ref->setEnabled(true);
            if(windowRef[selectedWindow-1])
                Ref->setChecked(true);
            else
                Ref->setChecked(false);
        }
        else
            Ref->setEnabled(false);
    }

}

void MainWindow::updateContrastButton(){
    InvertContrast->setChecked(false);
    DefaultContrast->setChecked(false);
    MinMaxContrast->setChecked(false);
    HistoContrast->setChecked(false);

    if(currentSerieNumber != -1){
        Serie *serie = Series[currentSerieNumber-1];
        switch(serie->getcontrast()){
        case Default:
            DefaultContrast->setChecked(true);
            break;
        case MinMax:
            MinMaxContrast->setChecked(true);
            break;
        case Histo:
            HistoContrast->setChecked(true);
            break;
        case DefaultInverted:
            DefaultContrast->setChecked(true);
            InvertContrast->setChecked(true);
            break;
        case MinMaxInverted:
            MinMaxContrast->setChecked(true);
            InvertContrast->setChecked(true);
            break;
        case HistoInverted:
            HistoContrast->setChecked(true);
            InvertContrast->setChecked(true);
            break;
        }

    }
}


//---------------------------------------------------------------//
//------------------------ PAINT LINKED LINES  ------------------//
//---------------------------------------------------------------//
void MainWindow::paintLinkedLines(){
    //cout << "paint on linked views" << endl;
    Serie *serie = Series[currentSerieNumber-1];

    vector<int> axialWindow=serie->getAxialWindow();
    vector<int> coronalWindow=serie->getCoronalWindow();
    vector<int> sagittalWindow=serie->getSagittalWindow();


    QPixmap Axialmap, Sagittalmap,Coronalmap;
    int bx,by,ex,ey;

    switch(currentPlane){
    case Axial:
        bx=0;
        by = ey = serie->getZIndex();
        if(coronalWindow[0] != -1){
            Coronalmap =serie->getCurrentImg(Coronal);
            ex=Coronalmap.width();
            for(int i=0; i<coronalWindow.size(); i++)
                paintOnScene(Coronalmap,coronalWindow[i],bx,by,ex,ey);
        }
        if(sagittalWindow[0] != -1){
            Sagittalmap = serie->getCurrentImg(Sagittal);
            ex=Sagittalmap.width();
            for(int i=0; i<sagittalWindow.size(); i++)
                paintOnScene(Sagittalmap,sagittalWindow[i],bx,by,ex,ey);
        }
        break;
    case Coronal:
        if(axialWindow[0] != -1){
            Axialmap =serie->getCurrentImg(Axial);
            bx=0;
            by = ey = serie->getYIndex();
            ex=Axialmap.width();
            for(int i=0; i<axialWindow.size(); i++)
                paintOnScene(Axialmap,axialWindow[i],bx,by,ex,ey);
        }
        if(sagittalWindow[0] != -1){
            Sagittalmap = serie->getCurrentImg(Sagittal);
            bx= ex = serie->getYIndex();
            by=0;
            ey=Sagittalmap.height();
            for(int i=0; i<sagittalWindow.size(); i++)
                paintOnScene(Sagittalmap,sagittalWindow[i],bx,by,ex,ey);
        }
        break;
    case Sagittal:
        bx = ex = serie->getXIndex();
        by=0;
        if(axialWindow[0] != -1){
            Axialmap =serie->getCurrentImg(Axial);
            ey=Axialmap.height();
            for(int i=0; i<axialWindow.size(); i++)
                paintOnScene(Axialmap,axialWindow[i],bx,by,ex,ey);
        }
        if(coronalWindow[0] != -1){
            Coronalmap = serie->getCurrentImg(Coronal);
            ey=Coronalmap.height();
            for(int i=0; i<coronalWindow.size(); i++)
                paintOnScene(Coronalmap,coronalWindow[i],bx,by,ex,ey);
        }
        break;
    default:
        break;

    }
}


//---------------------------------------------------------------//
//-------------------------- PAINT ON SCENE  --------------------//
//---------------------------------------------------------------//
void MainWindow::paintOnScene(QPixmap &pixmap, int sceneNb,int beginX,int beginY,int endX, int endY ){
    QPainter p(&pixmap);

    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Qt::red, 2));
    p.drawLine(beginX,beginY,endX, endY);
    p.end(); // Don't forget this line!

    //cout << "window nb " << sceneNb << endl;
    //cout << "bx " << beginX << " by " << beginY << " ex " << endX << " ey " << endY << endl;

    switch(sceneNb){
    case 1:
        myScene->addPixmap(pixmap);
        break;
    case 2:
        myScene2->addPixmap(pixmap);
        break;
    case 3:
        myScene3->addPixmap(pixmap);
        break;
    case 4:
        myScene4->addPixmap(pixmap);
        break;
    default:
        break;
    }

}


void MainWindow::updateWindowConnection(){
    //cout << "update window connection" << endl;
    for(int i=0; i<4; i++){
        for(int j=0; j<4; j++){
            windowConnection[i][j]=false;
            if(windowSerieNb[i] !=-1 && windowSerieNb[j] !=-1 && windowSerieNb[i]==windowSerieNb[j]){
               //cout << "from window " << i << " to window " << j << endl;
                //  cout << "plan 1 " << windowCurrentPlan[i] << " plan 2 " << windowCurrentPlan[j]  << endl;
                if(windowCurrentPlane[i] != Unknown && windowCurrentPlane[j] != Unknown && windowCurrentPlane[i]== windowCurrentPlane[j])
                    windowConnection[i][j]=true;
            }

        }
    }

    //update connection for displayed series
    for(int i =0; i<4; i++){
        if(windowSerieNb[i] != -1)
            Series[windowSerieNb[i]-1]->setPlaneWindows(windowSerieNb,windowCurrentPlane);
    }
}


//********************************************************************************//
//---------------------ALL PRIVATE SLOTS FOLLOWING -----------------------------//
//********************************************************************************//
//Change the selected window on mousePress
void MainWindow::mousePressEvent(QMouseEvent* e){
    if(ui->graphicsView->underMouse()){
        selectedWindow=1;
        ui->graphicsView->setFrameStyle(3);
        ui->graphicsView_2->setFrameStyle(1);
        ui->graphicsView_3->setFrameStyle(1);
        ui->graphicsView_4->setFrameStyle(1);

        ui->graphicsView->setStyleSheet("color:orange");
        ui->graphicsView_2->setStyleSheet("color:black");
        ui->graphicsView_3->setStyleSheet("color:black");
        ui->graphicsView_4->setStyleSheet("color:black");

    }
    else if(ui->graphicsView_2->underMouse()){
        selectedWindow=2;
        ui->graphicsView->setFrameStyle(1);
        ui->graphicsView_2->setFrameStyle(3);
        ui->graphicsView_3->setFrameStyle(1);
        ui->graphicsView_4->setFrameStyle(1);

        ui->graphicsView->setStyleSheet("color:black");
        ui->graphicsView_2->setStyleSheet("color:orange");
        ui->graphicsView_3->setStyleSheet("color:black");
        ui->graphicsView_4->setStyleSheet("color:black");

    }
    else if(ui->graphicsView_3->underMouse()){
        selectedWindow=3;
        ui->graphicsView->setFrameStyle(1);
        ui->graphicsView_2->setFrameStyle(1);
        ui->graphicsView_3->setFrameStyle(3);
        ui->graphicsView_4->setFrameStyle(1);

        ui->graphicsView->setStyleSheet("color:black");
        ui->graphicsView_2->setStyleSheet("color:black");
        ui->graphicsView_3->setStyleSheet("color:orange");
        ui->graphicsView_4->setStyleSheet("color:black");
    }
    else if(ui->graphicsView_4->underMouse()){
        selectedWindow=4;
        ui->graphicsView->setFrameStyle(1);
        ui->graphicsView_2->setFrameStyle(1);
        ui->graphicsView_3->setFrameStyle(1);
        ui->graphicsView_4->setFrameStyle(3);


        ui->graphicsView->setStyleSheet("color:black");
        ui->graphicsView_2->setStyleSheet("color:orange");
        ui->graphicsView_3->setStyleSheet("color:black");
        ui->graphicsView_4->setStyleSheet("color:orange");
    }

    currentSerieNumber=windowSerieNb[selectedWindow-1];


    //Updating Link button and views
    Link->setEnabled(false);

    if(windowSerieNb[selectedWindow-1]!= -1){
        Serie * serie = Series[currentSerieNumber-1];
        Link->setChecked(false);

        if(serie->getNbImages()>10){
            Link->setEnabled(true);
            if(serie->getViewLinked()){
                displayInScene(serie->getCurrentImg(windowCurrentPlane[selectedWindow-1]));
                Link->setChecked(true);
            }
        }
    }
    updateWindowInfo();
}

//Scrolling images of the serie if Scroll button is activated
void MainWindow::wheelEvent(QWheelEvent *event){
   // cout << "JPPPP " << windowSerieNb[selectedWindow-1] << endl;
    updateWindowConnection();
    if(windowSerieNb[selectedWindow-1] != -1){
        Serie *serie = Series[currentSerieNumber-1];
        vector<int> connection;
        for(int i=0;i<4;i++){
            if(i!=selectedWindow-1 && windowConnection[selectedWindow-1][i])
                connection.push_back(i);
        }


        if(Scroll->isChecked()){
            if(event->delta() >0){
                serie->setNextIndex(currentPlane);
            }
            else{
                serie->setPreviousIndex(currentPlane);
            }

            QPixmap img;
            if(Flag->isChecked())
                img=serie->getFlags();
            else
                img = serie->getCurrentImg(windowCurrentPlane[selectedWindow-1]);

            if(serie->getViewLinked())
                paintLinkedLines();


            displayInScene(img);

            for(int i=0; i<connection.size(); i++){
                   displayInScene(img,connection[i]+1);
            }
        }
    }
}

//Click event for series button
//Will display the serie according to his corresponding number
void MainWindow::buttonInGroupClicked(QAbstractButton *b){
    currentSerieNumber = b->property("Id").toInt();

    b->setChecked(true);

    cout <<"button checked " << buttonGroup->checkedId() << endl;

    //uncheck the button that was previously selected
    buttonGroup->button(buttonGroup->checkedId())->setChecked(false);

    Serie *serie= Series[currentSerieNumber-1];
    currentPlane = serie->getdefaultPlane();

    //cout << "property number  " << currentSerieNumber<<endl;
    string buttonName = b->text().toLocal8Bit().constData();
    //cout << "button " << buttonName << " clicked, associated serie size: "<< this->allPath[buttonName].size() << endl;
    currentSerie=buttonName;

    //if(windowSerieNb[selectedWindow-1] == currentSerieNumber)
      //  windowSerieNb[selectedWindow-1]=-1;

    if(serie->isBuilt())
        displayInScene(serie->getCurrentImg(serie->getdefaultPlane()));
    else
        createDefaultPlane();

    windowSerieNb[selectedWindow-1]=currentSerieNumber;
    windowDefaultPlane[selectedWindow-1]=currentPlane;
    windowCurrentPlane[selectedWindow-1]=currentPlane;
    windowNbImg[selectedWindow-1]=serie->getNbImages();

    if(!serie->hasFlag())
        Flag->setEnabled(false);

    if(!serie->hasRef())
        Ref->setEnabled(false);

    updateWindowInfo();
}

//Show/hide advanced settings
void MainWindow::show_advanced()
{
      ui->AdvancedSettings->setVisible(true);
}

void MainWindow::hide_advanced()
{
    ui->AdvancedSettings->setVisible(false);
}

void MainWindow::invert_grayscale(){
    Serie *serie = Series[currentSerieNumber-1];

    if(InvertContrast->isChecked()){
        switch(serie->getcontrast()){
        case Default:
            serie->setcontrast(DefaultInverted);
            break;
        case MinMax:
            serie->setcontrast(MinMaxInverted);
            break;
        case Histo:
            serie->setcontrast(HistoInverted);
            break;
        }
    }else{
        switch(serie->getcontrast()){
        case DefaultInverted:
            serie->setcontrast(Default);
            break;
        case MinMaxInverted:
            serie->setcontrast(MinMax);
            break;
        case Histo:
            serie->setcontrast(HistoInverted);
            break;
        }

    }

    displayInScene(serie->getCurrentImg(windowCurrentPlane[selectedWindow-1]));

//    switch(selectedWindow){
//    case 1:
//        myScene->addPixmap(img);
//        ui->graphicsView->setScene(myScene);
//        break;
//    case 2:
//        myScene2->addPixmap(img);
//        ui->graphicsView_2->setScene(myScene2);
//        break;
//    case 3:
//        myScene3->addPixmap(img);
//        ui->graphicsView_3->setScene(myScene3);
//        break;
//    case 4:
//        myScene4->addPixmap(img);
//        ui->graphicsView_4->setScene(myScene4);
//        break;
//    }

}


//Show, hide graphicsView depending on the corresponding number
void MainWindow::display_one_window()
{
    //set default selected window
    selectedWindow=1;

    //update frame for selected window
    ui->graphicsView->setFrameStyle(3);
    ui->graphicsView_2->setFrameStyle(1);
    ui->graphicsView_3->setFrameStyle(1);
    ui->graphicsView_4->setFrameStyle(1);

    ui->graphicsView_2->setVisible(false);
    ui->graphicsView_3->setVisible(false);
    ui->graphicsView_4->setVisible(false);
}

void MainWindow::display_two_window()
{
    if(selectedWindow==3 || selectedWindow==4){
        selectedWindow=1;
        ui->graphicsView->setFrameStyle(3);
        ui->graphicsView_2->setFrameStyle(1);
        ui->graphicsView_3->setFrameStyle(1);
        ui->graphicsView_4->setFrameStyle(1);
    }

     ui->graphicsView_2->setVisible(true);
     ui->graphicsView_3->setVisible(false);
     ui->graphicsView_4->setVisible(false);
}

void MainWindow::display_four_window()
{
    ui->graphicsView_2->setVisible(true);
    ui->graphicsView_3->setVisible(true);
    ui->graphicsView_4->setVisible(true);
}


//Zoom in the image
void MainWindow::zoom_plus(){
    windowZoom[selectedWindow-1] +=1;
    switch(selectedWindow){
    case 1:
        ui->graphicsView->scale(1.1,1.1);
        break;
    case 2:
        ui->graphicsView_2->scale(1.1,1.1);
        break;
    case 3:
        ui->graphicsView_3->scale(1.1,1.1);
        break;
    case 4:
        ui->graphicsView_4->scale(1.1,1.1);
        break;
    }

}

void MainWindow::zoom_minus(){
    windowZoom[selectedWindow-1] -=1;
    switch(selectedWindow){
    case 1:
        ui->graphicsView->scale(0.9,0.9);
        break;
    case 2:
        ui->graphicsView_2->scale(0.9,0.9);
        break;
    case 3:
        ui->graphicsView_3->scale(0.9,0.9);
        break;
    case 4:
        ui->graphicsView_4->scale(0.9,0.9);
        break;
    }
}


//set contrst to the corresponding value
void MainWindow::default_contrast(){
    Serie *serie = Series[currentSerieNumber-1];
    Contrast ct = contrast[selectedWindow-1];

    //disable all other contrast
    MinMaxContrast->setChecked(false);
    HistoContrast->setChecked(false);

    if(ct ==Default || ct==DefaultInverted){
       DefaultContrast->setChecked(true);
    }else if(!InvertContrast->isChecked() ){
        contrast[selectedWindow-1]=Default;
        serie->setcontrast(Default);
        createDefaultPlane();
    }else{
        contrast[selectedWindow-1]=DefaultInverted;
        serie->setcontrast(DefaultInverted);
        createDefaultPlane();
    }
}

void MainWindow::minmax_contrast(){
    Serie *serie = Series[currentSerieNumber-1];
    Contrast ct = contrast[selectedWindow-1];

    //disable all other contrasts
    DefaultContrast->setChecked(false);
    HistoContrast->setChecked(false);

    if(ct ==MinMax || ct==MinMaxInverted){
       MinMaxContrast->setChecked(true);
    }else if (!InvertContrast->isChecked()){
        contrast[selectedWindow-1]=MinMax;
        serie->setcontrast(MinMax);
        createDefaultPlane();
    }else {
        contrast[selectedWindow-1]=MinMaxInverted;
        serie->setcontrast(MinMaxInverted);
        createDefaultPlane();
    }
}

void MainWindow::histo_contrast(){
    Serie *serie = Series[currentSerieNumber-1];
    Contrast ct = contrast[selectedWindow-1];

    //disable all other contrasts
    DefaultContrast->setChecked(false);
    MinMaxContrast->setChecked(false);

    if(ct ==Histo || ct==HistoInverted){
       HistoContrast->setChecked(true);
    }else if (!InvertContrast->isChecked()){
        contrast[selectedWindow-1]=Histo;
        serie->setcontrast(Histo);
        createDefaultPlane();
    }else{
        contrast[selectedWindow-1]=HistoInverted;
        serie->setcontrast(HistoInverted);
        createDefaultPlane();
    }
}


//Open the report window
void MainWindow::on_showReport_clicked()
{
    report->showMaximized();
}


//Calling for corresponding displayPlan function
void MainWindow::callAxial(){
    displayAxialPlane();
}

void MainWindow::callCoronal(){
    displayCoronalPlane();
}

void MainWindow::callSagittal(){
    displaySagittalPlane();
}


void MainWindow::link_views(){
    Series[currentSerieNumber-1]->setViewLinked();
    if(Link->isChecked()){
        paintLinkedLines();

    }else{

    }
}


void MainWindow::showFlagged(){
    Serie *serie=Series[currentSerieNumber-1];

    if(Flag->isChecked()){
        windowFlag[selectedWindow-1]=true;
        currentPlane =FlagImg;
        QPixmap img = serie->getFlags();
        if(img.data_ptr())
            displayInScene(img);
    }
    else{
        windowFlag[selectedWindow-1]=false;
        currentPlane=serie->getdefaultPlane();
        displayInScene(serie->getCurrentImg(currentPlane));
    }

}

void MainWindow::showRef(){
    Serie *serie=Series[currentSerieNumber-1];

    if(Ref->isChecked()){
        windowRef[selectedWindow-1]=true;
        currentPlane =RefImg;
        QPixmap img = serie->getRef();
        if(img.data_ptr())
            displayInScene(img);
    }
    else{
        windowRef[selectedWindow-1]=false;
        currentPlane=serie->getdefaultPlane();
        displayInScene(serie->getCurrentImg(currentPlane));
    }

}

void MainWindow::validPassword(char * pw){
    if(!strcmp(pinAccess,pw)){
        cout << "correct password " << endl;
        Access=true;
        pinDialog.close();
        processData();
    }else{
        pinDialog.setWrongPin();
        cout << "wrong password" << endl;
    }

}


//reseting
void MainWindow::reset_settings(){
    //reseting contrast
    contrast[selectedWindow-1] = Default;
    default_contrast();

    //reseting zoom
    int zoom = windowZoom[selectedWindow-1] ;
    if(zoom > 0){
        for (int i=0; i<zoom ; i++){
            zoom_minus();
        }
    }else  if(zoom < 0){
        for (int i=0; i>zoom ; i--){
            zoom_plus();
        }
    }
    windowZoom[selectedWindow-1]=0;

    //by default Scrolling is activated
    Scroll->setChecked(true);

    //by default everything else is unchecked
    if(Flag->isChecked()){
        Flag->setChecked(false);
        windowFlag[selectedWindow-1]=false;
        showFlagged();
    }



}
