#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialog.h"
#include <QLabel>
#include <QMessageBox>
#include <iostream>

#include <QGuiApplication>
#include <QApplication>
#include <iostream>
#undef UNICODE
#undef _UNICODE
#include <dcmtk/config/osconfig.h>
#include <dcmtk/dcmdata/dctk.h>
#include <dcmtk/dcmimgle/dcmimage.h>
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

    for(int i=0 ; i<4; i++){
        creation[i]=0;
        contrast[i]=0;
        windowSerieNb[i]=-1;
    }

}

//********************************************************************************//
//---------------------ALL PUBLIC FUNCTION FOLLOWING -----------------------------//
//********************************************************************************//

MainWindow::~MainWindow()
{
    for(int i=0; i< Images.size() ; i++){
        if(Images[i]!=NULL)
            delete Images[i];
    }

    for(int i=0; i< Images2.size() ; i++){
        if(Images2[i]!=NULL)
            delete Images2[i];
    }

    for(int i=0; i< Images3.size() ; i++){
        if(Images3[i]!=NULL)
            delete Images3[i];
    }

    for(int i=0; i< myPixelsZ.size() ; i++){
        if(myPixelsZ[i]!=NULL)
            delete myPixelsZ[i];
    }

    for(int i=0; i< myPixelsX.size() ; i++){
        if(myPixelsX[i]!=NULL)
            delete myPixelsX[i];
    }

    for(int i=0; i< myPixelsY.size() ; i++){
        if(myPixelsY[i]!=NULL)
            delete myPixelsY[i];
    }


    delete ui;
}


void MainWindow::constructWindow(QString path){
    //call function to create button architecture
    createButtons();

    //processing Qtring path, creating string element to process DicomFile reading
    QByteArray ba = path.toLatin1();
    const char *dicomdirPath = ba.data();

    int size = strlen(dicomdirPath);
    char *filepath=new char[130];

    for(int i=0; i<size-8; i++){
        filepath[i]=dicomdirPath[i];
    }
    filepath[size-8]='\0';

    //call function to open DICOMDIR and store images paths
    processDicom(dicomdirPath,filepath);



    //display images for the first serie found
    // displayImages();

    display_one_window(); // by default show only one window
    createDefaultPlan();



    // constructPlans();


    delete filepath;
}

void MainWindow::processDicom(const char *dicomdirPath, char *filepath){
    //counter for the number of series found in the file
    series=0;

    cout <<"path " << filepath << endl;

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

    DcmDirectoryRecord *   ImageRecord = NULL;

    currentSerie = "Series2";
    currentSerieNumber =2;
    int nbFrame=0;
    Plan seriePlan = Unknown;

    int WC1,WW1;

    double rescale =1;

    string firstpath;


    if(root != NULL)
            {
                //Find next folder
                while (((PatientRecord = root->nextSub(PatientRecord)) != NULL))
                {
                    while (((StudyRecord = PatientRecord->nextSub(StudyRecord)) != NULL))
                    {
                        char *date=new char[11];
                        char *studydesc=new char[30];
                        char *patientName=new char[50];
                        char *birthdate=new char[11];

                        if (StudyRecord->findAndGetOFString(DCM_PatientName, tmpString).good()) {
                            cout << "PatientName 1: " << tmpString.c_str() << endl ;
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

                        if (StudyRecord->findAndGetOFString(DCM_PatientBirthDate, tmpString).good()) {
                            cout << "birthdate 1 : " << tmpString.c_str() << endl ;
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

                        }
                        if (StudyRecord->findAndGetOFString(DCM_StudyDescription, tmpString).good()) {
                            cout <<"Study Description: " << tmpString.c_str() << endl ;
                            strcpy(studydesc,tmpString.c_str());
                        }else{
                            string nodesc= "No description found";
                            strcpy(studydesc,nodesc.c_str());
                        }

                        if (StudyRecord->findAndGetOFString(DCM_StudyDate, tmpString).good()){
                            cout << "date " << tmpString.c_str() << endl ;
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

                            cout << "date " << date <<  endl;
                            //strcpy(date,tmpString.c_str());
                        }

                        setPatientInfo(studydesc,date,patientName,birthdate);





                        while (((SeriesRecord = StudyRecord->nextSub(SeriesRecord)) != NULL))
                        {
                            //local variable, vector to store paths to files for each serie
                            vector<string> paths;

                            // counter for images within a serie
                            int count =0;

                            //going down the tree to find images
                            while((FileRecord = SeriesRecord->nextSub(FileRecord)) != NULL){
                                //full path to each image
                                string fullpath;

                                //increment image counter
                                count +=1;

                                if((ImageRecord = FileRecord->nextSub(ImageRecord)) != NULL)
                                    cout <<"tree keep going" << endl;

                                //DCM_ReferencedFileID give the name of the DICOM Image file
                                if(FileRecord->findAndGetOFStringArray(DCM_ReferencedFileID,tmpString).good()){
                                    //add image name to file path to get the full path
                                    fullpath = filepath + string(tmpString.c_str());
                                    if(count ==1)
                                        firstpath =  filepath + string(tmpString.c_str());
                                }

                                //Check file status, if not EIS_Normal the file is not an Image
                                //If not normal status the "parent" is not a serie

                                if(count ==1){
                                    if (DicomImage(fullpath.c_str()).getStatus() != EIS_Normal){
                                        count =0;
                                        cout << "statu not normal" << endl;
                                    }
                                }

                                // add path to the local vector
                                 paths.push_back(fullpath);


                                 if(count <=3){
                                    if(FileRecord->findAndGetOFString(DCM_WindowCenter,tmpString).good()){
                                        //cout << "window center " << tmpString.c_str() << endl;
                                        WC1=atoi(tmpString.c_str());

                                    }
                                    if(FileRecord->findAndGetOFString(DCM_WindowWidth,tmpString).good()){
                                       // cout << "window width " << tmpString.c_str() << endl;
                                        WW1= atoi(tmpString.c_str());
                                    }
                                 }


                                 if(count<=2 ){
                                     if(FileRecord->findAndGetOFStringArray(DCM_NumberOfFrames,tmpString).good()){
                                          cout << "Nb of Frames " << tmpString.c_str() << endl;
                                          nbFrame = atoi(tmpString.c_str());
                                     }

                                     if(FileRecord->findAndGetOFStringArray(DCM_ImageOrientationPatient,tmpString).good()){
                                         cout << "image orientation patient " << tmpString.c_str() << endl;
                                         seriePlan = findSeriePlan(tmpString.c_str());
                                     }
                                     if(FileRecord->findAndGetOFString(DCM_SliceThickness,tmpString).good()){
                                         //cout << "image orientation patient " << tmpString.c_str() << endl;
                                         rescale = atoi(tmpString.c_str()) ;
                                         //cout << "RESCALE FACTOR " << rescaleFactor << endl;
                                     }
                                     if(FileRecord->findAndGetOFStringArray(DCM_PixelSpacing,tmpString).good()){
                                         double pixelspacing = getPixelNb(tmpString.c_str());
                                         rescale = rescale*pixelspacing;
                                         cout << "pixel spacing " << pixelspacing << endl;
                                         cout << "RESCALE FACTOR " << rescale << endl;
                                     }

                                 }


                            }

                            //count !=0 means the folder contain DICOM images so this is a serie
                            if(count !=0){
                                series +=1;

                                cout << "series nb " <<series << "with "<< count <<" images" << endl;

                                char *desc=new char[100];


                                if (SeriesRecord->findAndGetOFString(DCM_SeriesDescription, tmpString).good()){
                                    strcpy(desc,tmpString.c_str());
                                    cout << tmpString.c_str() << " " << endl;
                                }


                                WC.push_back(WC1);
                                WW.push_back(WW1);

                                addSerieButton(series,desc, count, firstpath);

                                cout << "Serie " << series << " plan " << seriePlan << endl;


                                //SUPPOSSE HERE I HAVE A WAY TO DET THE SERIE CURRENT PLAN
                                // FOR NOW BY DEFAULT I PU AXIAL FOR ALL SERIES
                                seriesPlan.push_back(seriePlan);
                                seriesRescaleFactor.push_back(rescale);
                                serieNbImages.push_back(paths.size());


                                //storing paths for the current serie
                                allPath.insert(pair<string,vector<string>>("Series"+to_string(series),paths));

                                nbOfFrame.insert(pair<string,int>("Series"+to_string(series), nbFrame));


                            }
                        }
                    }
                }
            }

}

void MainWindow::setPatientInfo(char *studydesc, char *date, char *patientName, char *birthdate){
    //Personnal Information
    //QWidget *personalInfo = new QWidget(this);
    string str = string("<b>Personal Information:</b>") + string("<br>") + string(studydesc) + string("<br>") +  string(date) +  string("<br>") +  string(patientName) +  string("<br>") +  string("Birthdate: ") +  string(birthdate);

    QLabel *perso =new  QLabel( str.c_str()); //+ studydesc );//+ "\n" + date + "\n" + patientName + "\n" + "Birthdate: " + birthdate);

    perso->setMaximumHeight(120);

    ui->PersonalInfo->addWidget(perso);

}


void MainWindow::addSerieButton(int serieNumber, char *serieDescription, int imgNb, string firstPath){
    //adding button for each serie
    char serieName[30]="Series";
    strcat(serieName,to_string(serieNumber).c_str());


    // display first Image of the serie as seriebutton
//    DicomImage *serieImg;
//    int nbFrame = nbOfFrame[serieName];

//    if(nbFrame ==0)
//        serieImg = new DicomImage(firstPath.c_str());
//    else {
//        serieImg = new DicomImage(firstPath.c_str(),0,0,1);
//    }


//    serieImg->setWindow(WC[serieNumber-1],WW[serieNumber-1]);


//    uint8_t * pixel = (uint8_t *)serieImg->getOutputData(8);


//    QImage *img = new QImage(pixel, serieImg->getWidth(), serieImg->getHeight(),QImage::Format_Indexed8);
//    QPixmap pixmap = QPixmap::fromImage( QImage(img->scaled(QSize(50,50), Qt::IgnoreAspectRatio,Qt::SmoothTransformation)) );
//    QIcon ButtonIcon(pixmap);


    QPushButton *button = new QPushButton(this);


//    button->setIcon(ButtonIcon);
//    button->setIconSize(pixmap.rect().size());


    button->setText(tr(serieName));

    button->setProperty("Id",serieNumber);
    button->setVisible(true);
    buttonGroup->addButton(button);

    QLabel *SerieDesc= new QLabel(this);
   // QLabel *SerieImgNb = new QLabel(this);


    char serieDesc[100]="";
    strcpy(serieDesc, serieDescription);
    strcat(serieDesc, string("<br>").c_str());
    strcat(serieDesc, string ("Images: ").c_str());
    strcat(serieDesc,  to_string(imgNb).c_str());

    SerieDesc->setMaximumHeight(50);



//    char Images[15]= "Images: ";
//    strcat(Images, to_string(imgNb).c_str());


    SerieDesc->setText(serieDesc) ;

    //SerieImgNb->setText(Images);

    QFrame *SerieFrame = new QFrame(this);
    SerieFrame->setFrameShape(QFrame::HLine);
    SerieFrame->setFrameShadow(QFrame::Sunken);
    SerieFrame->setLineWidth(1);

    ui->SeriesLayout->addWidget(button);
    ui->SeriesLayout->addWidget(SerieDesc);
    //ui->SeriesLayout->addWidget(SerieImgNb);
    ui->SeriesLayout->addWidget(SerieFrame);

}

Plan MainWindow::findSeriePlan(const char *orientation){
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

double MainWindow::getPixelNb(const char* pixelArray){
    char *nb= new char[30];
    double number=0;

    int count=0;

    while(pixelArray[count] != '\\'){
        nb[count]=pixelArray[count];
        count ++;
    }
    nb[count]='\0';
    number=stod(nb);

   // cout << "test " << nb < " " << number << endl;

    delete nb;
    return number;
}




void MainWindow::createDefaultPlan(){
    //initializing index for images
    currentPlan = seriesPlan[currentSerieNumber-1];
    rescaleFactor=seriesRescaleFactor[currentSerieNumber-1];
    currentNbImages=serieNbImages[currentSerieNumber-1];

    Index = new int[4];
    for(int i=0; i<4; i++){
        Index[i]=0;
    }

    vector<DicomImage*> DicomImages;
    myPixelsZ.clear();
    myPixelsX.clear();
    myPixelsY.clear();

    int nbFrame = nbOfFrame[currentSerie];
    int nbImage = (int)allPath[currentSerie].size();

    //currentPlan=Axial;

    for(int i=0; i< nbImage ; i++){
       // DicomImage * img =
        if(nbFrame ==0)
            DicomImages.push_back(new DicomImage(allPath[currentSerie][i].c_str()));
        else {
             for(int j=0; j<nbFrame ; j++)
                DicomImages.push_back(new DicomImage(allPath[currentSerie][i].c_str(),0,j,1));
        }


    }

    //**********DEALING WITH ALL IMAGES OF THE SERIE**********//
    width = DicomImages[0]->getWidth(); // height and with are the default argument defined for the default plan of the Image
    height =  DicomImages[0]->getHeight();

    switch(currentPlan){
    case Axial:
        pixelXdepth = DicomImages[0]->getWidth();
        pixelYdepth =  DicomImages[0]->getHeight();
        //if currentplan is Axial, while creating other plan, pixelZdepth will need to be rescales to rescaleFatcor*pixelZdepth
        pixelZdepth = nbImage; //  nbImage is not properly the depth, but this is the value that will be used to recreate the other plans
        break;
    case Coronal:
        pixelXdepth =  DicomImages[0]->getWidth();
        //if currentplan is Coronal, while creating other plan, pixelYdepth will need to be rescales to rescaleFatcor*pixelYdepth
        pixelYdepth = nbImage;
        pixelZdepth = DicomImages[0]->getHeight();

        break;
    case Sagittal:
        //if currentplan is Sagittal, while creating other plan, pixelXdepth will need to be rescales to rescaleFatcor*pixelXdepth
        pixelXdepth = nbImage;
        pixelYdepth =  DicomImages[0]->getWidth();
        pixelZdepth = DicomImages[0]->getHeight();

        break;
    default:
        break;

    }

    cout << "width " << width <<" height " << height << endl;

    int check=true;
    for(int i=0; i<DicomImages.size(); i++){
        if(DicomImages[i] == NULL)
            check=false;
        else if(!(DicomImages[i]->getStatus() == EIS_Normal))
            check=false;
    }


    if (check){
        //if (DicomImages[0]->getStatus() == EIS_Normal && DicomImages[1]->getStatus() == EIS_Normal)
        //cout << "width " << DicomImages[0]->getWidth() << endl;
        //cout << "height "<<  DicomImages[0]->getHeight() << endl;
        for(int i=0; i<DicomImages.size(); i++){

            if (contrast[0]==0)
                DicomImages[i]->setWindow(WC[currentSerieNumber-1],WW[currentSerieNumber-1]);
            else if (contrast[0] ==1)
                DicomImages[i] ->setMinMaxWindow();
            else
                DicomImages[i] ->setHistogramWindow();

            Uint8* pixelData = (Uint8 *)(DicomImages[i]->getOutputData(8 )); // bits per sample

            switch(currentPlan){
            case Axial:
                myPixelsZ.push_back((uint8_t *)pixelData);
                break;
            case Sagittal:
                myPixelsX.push_back((uint8_t *)pixelData);
                break;
            case Coronal:
                myPixelsY.push_back((uint8_t *)pixelData);
                break;

            }

            if (pixelData != NULL){

                // do something useful with the pixel data
                QImage *img=new QImage (pixelData,width,height, QImage::Format_Indexed8);

                switch(selectedWindow){
                case 1:
                    Images.push_back(img);
                    break;
                case 2:
                    Images2.push_back(img);
                    break;
                case 3:
                    Images3.push_back(img);
                    break;
                case 4:
                    Images4.push_back(img);
                    break;
                }



            }
       }
   }
   else
      cerr << "Error: cannot load DICOM image (" << DicomImage::getString(DicomImages[0]->getStatus()) << ")" << endl;


//    cout << "nb of images " << Images.size() << endl;

  //  Index[0]= (int)Images.size()/2;

    //creating scene

    createScene();

}

void MainWindow::createScene(){
//    QPixmap pixmap;
//     pixmap = QPixmap::fromImage( *Images[Index[0]] );
//    QPainter p(&pixmap);

    if(windowSerieNb[selectedWindow-1]==-1 || windowSerieNb[selectedWindow-1] != currentSerieNumber ){
        windowSerieNb[selectedWindow-1]= currentSerieNumber;
        windowDefaultPlan[selectedWindow-1]= seriesPlan[currentSerieNumber-1];
        windowCurrentPlan[selectedWindow-1]=seriesPlan[currentSerieNumber-1];
        windowNbImg[selectedWindow-1]=serieNbImages[currentSerieNumber-1];
    }


    switch(selectedWindow){
    case 1:
//        windowSerieNb[0]= currentSerieNumber;
//        windowDefaultPlan[0]= seriesPlan[currentSerieNumber-1];
//        windowCurrentPlan[0]=seriesPlan[currentSerieNumber-1];
//        windowNbImg[0]=serieNbImages[currentSerieNumber-1];


        if(creation[0]==0){
             creation[0] =1;

             myScene= new QGraphicsScene(this);

             myScene->addPixmap( QPixmap::fromImage( *Images[Index[0]] ) );

             ui->graphicsView->setAlignment(Qt::AlignCenter);
             ui->graphicsView->setBackgroundBrush(QBrush(Qt::black));
             ui->graphicsView->setScene(myScene);


             ui->graphicsView->fitInView(myScene->sceneRect(),Qt::KeepAspectRatioByExpanding);

             ui->graphicsView->fitInView(QRectF(0,0,ui->graphicsView->width(), ui->graphicsView->height()),Qt::KeepAspectRatio);

             ui->graphicsView->setFrameRect(QRect(0,0,ui->graphicsView->width(), ui->graphicsView->height()));
             ui->graphicsView->setFrameStyle(3);

             ui->graphicsView->setStyleSheet("color:orange");
             ui->graphicsView_2->setStyleSheet("color:black");
             ui->graphicsView_3->setStyleSheet("color:black");
             ui->graphicsView_4->setStyleSheet("color:black");


             myScene2= new QGraphicsScene(this);
             myScene3= new QGraphicsScene(this);
             myScene4= new QGraphicsScene(this);

             ui->graphicsView_2->setBackgroundBrush(QBrush(Qt::black));
             ui->graphicsView_3->setBackgroundBrush(QBrush(Qt::black));
             ui->graphicsView_4->setBackgroundBrush(QBrush(Qt::black));

             ui->graphicsView_2->setScene(myScene2);
             ui->graphicsView_3->setScene(myScene3);
             ui->graphicsView_4->setScene(myScene4);
        }else {

            cout << "there" << endl;
            myScene->clear();
            myScene->addPixmap( QPixmap::fromImage( *Images[Index[0]] ) );

            ui->graphicsView->setBackgroundBrush(QBrush(Qt::black));

            ui->graphicsView->setScene(myScene);

        }

        break;
    case 2:
//        windowSerieNb[1]= currentSerieNumber;
//        windowDefaultPlan[1]= seriesPlan[currentSerieNumber-1];
//        windowCurrentPlan[1]=seriesPlan[currentSerieNumber-1];
//        windowNbImg[1]=serieNbImages[currentSerieNumber-1];

        myScene2->clear();
        myScene2->addPixmap( QPixmap::fromImage( *Images2[Index[1]] ) );

        ui->graphicsView_2->setScene(myScene2);

        if(creation[1]==0){
             creation[1] =1;

             ui->graphicsView_2->fitInView(myScene2->sceneRect(),Qt::KeepAspectRatioByExpanding);

             ui->graphicsView_2->fitInView(QRectF(0,0,ui->graphicsView_2->width(), ui->graphicsView_2->height()),Qt::KeepAspectRatio);

             ui->graphicsView_2->setFrameRect(QRect(0,0,ui->graphicsView_2->width(), ui->graphicsView_2->height()));
        }
        break;
    case 3:

        myScene3->clear();
        myScene3->addPixmap( QPixmap::fromImage( *Images3[Index[2]] ) );

        ui->graphicsView_3->setScene(myScene3);

        if(creation[2]==0){
             creation[2] =1;
             ui->graphicsView_3->fitInView(myScene3->sceneRect(),Qt::KeepAspectRatioByExpanding);

             ui->graphicsView_3->fitInView(QRectF(0,0,ui->graphicsView_3->width(), ui->graphicsView_3->height()),Qt::KeepAspectRatio);

             ui->graphicsView_3->setFrameRect(QRect(0,0,ui->graphicsView_3->width(), ui->graphicsView_3->height()));
        }
        break;
    case 4:
//        windowSerieNb[3]= currentSerieNumber;
//        windowDefaultPlan[3]= seriesPlan[currentSerieNumber-1];
//        windowCurrentPlan[3]=seriesPlan[currentSerieNumber-1];
//        windowNbImg[3]=serieNbImages[currentSerieNumber-1];

        myScene4->clear();
        myScene4->addPixmap( QPixmap::fromImage( *Images4[Index[3]] ) );

         ui->graphicsView_4->setScene(myScene3);

         if(creation[3]==0){
             creation[3] =1;
             ui->graphicsView_4->fitInView(myScene4->sceneRect(),Qt::KeepAspectRatioByExpanding);

             ui->graphicsView_4->fitInView(QRectF(0,0,ui->graphicsView_4->width(), ui->graphicsView_4->height()),Qt::KeepAspectRatio);

             ui->graphicsView_4->setFrameRect(QRect(0,0,ui->graphicsView_4->width(), ui->graphicsView_4->height()));
         }
        break;

    }

    updateWindowInfo();
}

void MainWindow::constructSagittalPlan(){
    cout << "construct sagittal" << endl;
    //construct Sagittal from Axial view
    if(currentPlan == Axial){

        //X fixed <-> width
        for (int x=0; x<pixelXdepth; x++){
           uint8_t *mypixel= new uint8_t[pixelYdepth*pixelZdepth];
           int countX=0;
            for(int z=0; z<pixelZdepth; z++){
                for(int y=x; y<pixelXdepth*pixelYdepth ; y+=pixelXdepth){
                    mypixel[countX]=myPixelsZ[z][y];
                    countX +=1;
                }
            }

            myPixelsX.push_back(mypixel);
            QImage *img= new QImage(mypixel,pixelYdepth, pixelZdepth, QImage::Format_Indexed8);

            //CHECK FOR THE RIGTH SCALING

            QImage *copy =  new QImage(img->scaled(QSize(pixelYdepth,rescaleFactor*pixelZdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
            delete img;

            switch(selectedWindow){
            case 1:
                Images.push_back(copy);
                break;
            case 2:
                Images2.push_back(copy);
                break;
            case 3:
                Images3.push_back(copy);
                break;
            case 4:
                Images4.push_back(copy);
            }
        }
    }else if(currentPlan == Coronal){
        cout << "sagittal from coronal" << endl;
        for (int x=0; x<pixelXdepth; x++){
           uint8_t *mypixel= new uint8_t[pixelYdepth*pixelZdepth];
           int countX=0;
            for(int z=x; z<pixelXdepth*pixelZdepth; z+=pixelXdepth){
                for(int y=0; y<pixelYdepth ; y++){
                    mypixel[countX]=myPixelsY[y][z];
                    countX +=1;
                }
            }
                myPixelsX.push_back(mypixel);
                QImage *img = new QImage (mypixel,pixelYdepth, pixelZdepth, QImage::Format_Indexed8);
                QImage *copy =  new QImage(img->scaled(QSize(rescaleFactor*pixelYdepth,pixelZdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
                delete img;



                switch(selectedWindow){
                case 1:
                    Images.push_back(copy);
                    break;
                case 2:
                    Images2.push_back(copy);
                    break;
                case 3:
                    Images3.push_back(copy);
                    break;
                case 4:
                    Images4.push_back(copy);
                }

        }

    }

    switch(selectedWindow){
    case 1:
        Index[0]=(int)Images.size()/2;
        break;
    case 2:
        Index[1]=(int)Images2.size()/2;
        break;
    case 3:
        Index[2]=(int)Images3.size()/2;
        break;
    case 4:
        Index[3]=(int)Images4.size()/2;
    }

    windowCurrentPlan[selectedWindow-1]=Sagittal;
    createScene();

}



void MainWindow::constructAxialPlan(){
    if(currentPlan == Sagittal){
        for (int z=0; z<pixelZdepth; z++){
           uint8_t *mypixel= new uint8_t[pixelYdepth*pixelXdepth];
           int countZ=0;
            for(int y=z*pixelYdepth; y<(z+1)*pixelYdepth; y++){
                for(int x=0; x<pixelXdepth ; x++){
                    mypixel[countZ]=myPixelsX[x][y];
                    countZ +=1;
                }
            }

            myPixelsZ.push_back(mypixel);
            QImage *img= new QImage(mypixel,pixelXdepth, pixelYdepth, QImage::Format_Indexed8);

            QImage *copy =  new QImage(img->scaled(QSize(rescaleFactor*pixelXdepth,pixelYdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
            delete img;

            switch(selectedWindow){
            case 1:
                Images.push_back(copy);
                break;
            case 2:
                Images2.push_back(copy);
                break;
            case 3:
                Images3.push_back(copy);
                break;
            case 4:
                Images4.push_back(copy);
            }
        }
    }else if(currentPlan == Coronal){
        cout << "axial from coronal" << endl;
        for (int z=0; z<pixelZdepth; z++){
           uint8_t *mypixel= new uint8_t[pixelYdepth*pixelXdepth];
           int countZ=0;
            for(int y=0; y<pixelYdepth; y++){
                for(int x=z*pixelXdepth; x<(z+1)*pixelXdepth ; x++){
                    mypixel[countZ]=myPixelsY[y][x];
                    countZ +=1;
                }
            }

            myPixelsZ.push_back(mypixel);
            QImage *img= new QImage(mypixel,pixelXdepth, pixelYdepth, QImage::Format_Indexed8);


            QImage *copy =  new QImage(img->scaled(QSize(pixelXdepth,rescaleFactor*pixelYdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
            delete img;

            switch(selectedWindow){
            case 1:
                Images.push_back(copy);
                break;
            case 2:
                Images2.push_back(copy);
                break;
            case 3:
                Images3.push_back(copy);
                break;
            case 4:
                Images4.push_back(copy);
            }
        }

    }
    switch(selectedWindow){
    case 1:
        Index[0]=(int)Images.size()/2;
        break;
    case 2:
        Index[1]=(int)Images2.size()/2;
        break;
    case 3:
        Index[2]=(int)Images3.size()/2;
        break;
    case 4:
        Index[3]=(int)Images4.size()/2;
        break;
    }

    windowCurrentPlan[selectedWindow-1]=Axial;
    createScene();


}

void MainWindow::constructCoronalPlan(){
    if(currentPlan == Axial){
        //Y fixed <-> width
        for (int y=0; y<pixelYdepth; y++){
           uint8_t *mypixel= new uint8_t[pixelXdepth*pixelZdepth];
           int countY=0;
            for(int z=0; z<pixelZdepth; z++){

                for(int x=y*pixelXdepth; x<(y+1)*pixelXdepth ; x++){
                    mypixel[countY]=myPixelsZ[z][x];
                    countY +=1;
                }
            }
                myPixelsY.push_back(mypixel);
                QImage *img = new QImage (mypixel,pixelXdepth, pixelZdepth, QImage::Format_Indexed8);
                QImage *copy =  new QImage(img->scaled(QSize(pixelXdepth,rescaleFactor*pixelZdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
                delete img;

                switch(selectedWindow){
                case 1:
                    Images.push_back(copy);
                    break;
                case 2:
                    Images2.push_back(copy);
                    break;
                case 3:
                    Images3.push_back(copy);
                    break;
                case 4:
                    Images4.push_back(copy);
                }

        }
    }else if(currentPlan == Sagittal){
        cout << "coronal from sagittal" << endl;
        for (int y=0; y<pixelYdepth; y++){
           uint8_t *mypixel= new uint8_t[pixelXdepth*pixelZdepth];
           int countY=0;
            for(int z=y; z<pixelYdepth*pixelZdepth; z+=pixelYdepth){
                for(int x=0; x<pixelXdepth ; x++){
                    mypixel[countY]=myPixelsX[x][z];
                    countY +=1;
                }
            }
                myPixelsY.push_back(mypixel);
                QImage *img = new QImage (mypixel,pixelXdepth, pixelZdepth, QImage::Format_Indexed8);
                QImage *copy =  new QImage(img->scaled(QSize(rescaleFactor*pixelXdepth,pixelZdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
                delete img;


                switch(selectedWindow){
                case 1:
                    Images.push_back(copy);
                    break;
                case 2:
                    Images2.push_back(copy);
                    break;
                case 3:
                    Images3.push_back(copy);
                    break;
                case 4:
                    Images4.push_back(copy);
                }

        }

    }
    switch(selectedWindow){
    case 1:
        Index[0]=(int)Images.size()/2;
        break;
    case 2:
        Index[1]=(int)Images2.size()/2;
        break;
    case 3:
        Index[2]=(int)Images3.size()/2;
        break;
    case 4:
        Index[3]=(int)Images4.size()/2;
        break;
    }

    windowCurrentPlan[selectedWindow-1]=Coronal;
    cout <<"current plan " << windowCurrentPlan[selectedWindow-1] << "selected window "<< selectedWindow<<  endl;
    createScene();

}



void MainWindow::createButtons(){
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

//    QAction *RotateRight= new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/rotateright.png"),"Rotate image to right",this);
//    ui->mainToolBar->addAction(RotateRight);

//    connect(RotateRight, SIGNAL(triggered(bool)), SLOT(rotate_right()));

//    QAction *RotateLeft= new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/rotateleft.png"),"Rotate image to left",this);
//    ui->mainToolBar->addAction(RotateLeft);

//    connect(RotateLeft, SIGNAL(triggered(bool)), SLOT(rotate_left()));

//    ui->mainToolBar->addSeparator();


    QAction *Flag = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/flag.png"),"Get me to the relevant image", this);
    ui->mainToolBar->addAction(Flag);
    connect(Flag, SIGNAL(triggered(bool)), SLOT(callTest()));

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

    if(currentNbImages <10){
        AxialAction->setEnabled(false);
        SagittalAction->setEnabled(false);
        CoronalAction->setEnabled(false);
    }else {
        switch(currentPlan){
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
            AxialAction->setEnabled(true);
            SagittalAction->setEnabled(true);
            CoronalAction->setEnabled(true);
            break;
        }

    }



    ui->mainToolBar->addSeparator();

    QAction *Label = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/label.png"),"Show labels", this);
    ui->mainToolBar->addAction(Label);

    ui->mainToolBar->addSeparator();

    QAction *Scroll = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/scroll.png"),"Scroll Images", this);
    ui->mainToolBar->addAction(Scroll);

    ui->mainToolBar->addSeparator();

    QAction *Examples = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/examples.png"),"Show me normal images", this);
    ui->mainToolBar->addAction(Examples);

    ui->mainToolBar->addSeparator();

    QAction *Reset = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/reset.png"),"Reset", this);
    ui->mainToolBar->addAction(Reset);

    ui->mainToolBar->addSeparator();

    QAction *Advanced = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/add.png"),"Advanced settings", this);
    ui->mainToolBar->addAction(Advanced);
    connect(Advanced,SIGNAL(triggered(bool)),this,SLOT(show_advanced()) );

    ui->mainToolBar->addSeparator();


    //creating advanced setting tool bar
    ui->AdvancedSettings->setVisible(false);

    QAction *Invert = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/grayscale.png"),"Invert Grayscale", this);
    ui->AdvancedSettings->addAction(Invert);

    //setting invert var to zero = normal grayscale displayed
    invertGrayScale=0;
    connect(Invert,SIGNAL(triggered(bool)),this,SLOT(invert_grayscale()) );

    QAction *Default = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/defaultcontrast.png"),"Set default contrast", this);
    ui->AdvancedSettings->addAction(Default);
    connect(Default,SIGNAL(triggered(bool)),this,SLOT(default_contrast()) );

    QAction *Darker = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/minmaxcontrast.png"),"Set darker contrast", this);
    ui->AdvancedSettings->addAction(Darker);
    connect(Darker,SIGNAL(triggered(bool)),this,SLOT(minmax_contrast()) );

    QAction *Brigther = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/histogramcontrast.png"),"Set brighter contrast", this);
    ui->AdvancedSettings->addAction(Brigther);
    connect(Brigther,SIGNAL(triggered(bool)),this,SLOT(histo_contrast()) );

    QAction *AddStudy = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/study.png"),"Compare my other study", this);
    ui->AdvancedSettings->addAction(AddStudy);

    QAction *Link = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/link.png"),"Link views scrolling", this);
    ui->AdvancedSettings->addAction(Link);
    connect(Link,SIGNAL(triggered(bool)),this,SLOT(link_views()) );

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


void MainWindow::updateWindowInfo(){
    currentNbImages=serieNbImages[currentSerieNumber-1];
    currentPlan=windowCurrentPlan[selectedWindow-1];

//    cout <<"update" << endl;
//    cout << "default " << windowDefaultPlan[selectedWindow-1]<< "current " <<windowCurrentPlan[selectedWindow-1] << endl;


    if(currentPlan != windowDefaultPlan[selectedWindow-1] || currentNbImages <10){
        AxialAction->setEnabled(false);
        SagittalAction->setEnabled(false);
        CoronalAction->setEnabled(false);
    }else {
        AxialAction->setEnabled(true);
        SagittalAction->setEnabled(true);
        CoronalAction->setEnabled(true);

        switch(currentPlan){
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
            AxialAction->setEnabled(true);
            SagittalAction->setEnabled(true);
            CoronalAction->setEnabled(true);
            break;
        }
    }




}



//********************************************************************************//
//---------------------ALL PRIVATE SLOTS FOLLOWING -----------------------------//
//********************************************************************************//
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
    updateWindowInfo();


}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    if(event->delta() >0){

    if(selectedWindow==1){
        Index[0]+=1;
        if(Index[0]+1 > Images.size())
            Index[0]=0;

        myScene->addPixmap( QPixmap::fromImage( *Images[Index[0]] ) );
        ui->graphicsView->setScene(myScene);
    }else if(selectedWindow==2){
        Index[1]+=1;
        if(Index[1]+1 > Images2.size())
            Index[1]=0;

        myScene2->addPixmap( QPixmap::fromImage( *Images2[Index[1]] ) );
        ui->graphicsView_2->setScene(myScene2);
    }
    else if(selectedWindow==3){
        Index[2]+=1;
        if(Index[2]+1 > Images3.size())
            Index[2]=0;

        myScene3->addPixmap( QPixmap::fromImage( *Images3[Index[2]] ) );
        ui->graphicsView_3->setScene(myScene3);
    }

    }
    else if(event->delta() < 0){
        if(selectedWindow==1){
            Index[0]-=1;
            if(Index[0]<0)
                Index[0]= (int)Images.size()-1;

            myScene->addPixmap( QPixmap::fromImage( *Images[Index[0]] ) );
            ui->graphicsView->setScene(myScene);
        }else if(selectedWindow==2){
            Index[1]-=1;
            if(Index[1]<0)
                Index[1]=(int)Images2.size()-1;;

            myScene2->addPixmap( QPixmap::fromImage( *Images2[Index[1]] ) );
            ui->graphicsView_2->setScene(myScene2);
        }
        else if(selectedWindow==3){
            Index[2]+=1;
            if(Index[2]<0)
                Index[2]=(int)Images3.size()-1;;

            myScene3->addPixmap( QPixmap::fromImage( *Images3[Index[2]] ) );
            ui->graphicsView_3->setScene(myScene3);
        }


    }
}


void MainWindow::clearImagesVector(){
    switch(selectedWindow){
    case 1:
        Images.clear();
        break;
    case 2:
        Images2.clear();
        break;
    case 3:
        Images3.clear();
        break;
    case 4:
        Images4.clear();
        break;
    default:
        break;
    }

}

void MainWindow::buttonInGroupClicked(QAbstractButton *b){
    currentSerieNumber = b->property("Id").toInt();
    currentPlan = seriesPlan[currentSerieNumber-1];
    cout << "property number  " << currentSerieNumber<<endl;
    string buttonName = b->text().toLocal8Bit().constData();
    cout << "button " << buttonName << " clicked, associated serie size: "<< this->allPath[buttonName].size() << endl;
    currentSerie=buttonName;

    if(windowSerieNb[selectedWindow-1] == currentSerieNumber)
        windowSerieNb[selectedWindow-1]=-1;


    clearImagesVector();
    createDefaultPlan();

    updateWindowInfo();


}



void MainWindow::hide_advanced()
{
    ui->AdvancedSettings->setVisible(false);
}

void MainWindow::invert_grayscale()
{
    if(invertGrayScale ==0){
        for(int j=0; j<Images.size(); j++){
//            for( int i = 0; i < 256; ++i )
//                Images[j]->setColor(255-i, qRgb(i,i,i));
            Images[j]->invertPixels();
    }
    invertGrayScale=1;
    }else {
        for(int j=0; j<Images.size(); j++){
//            for( int i = 0; i < 256; ++i )
//                Images[j]->setColor(i, qRgb(i,i,i));
            Images[j]->invertPixels();
        }
        invertGrayScale=0;
    }

    myScene->addPixmap( QPixmap::fromImage( *Images[Index[0]] ) );
    ui->graphicsView->setScene(myScene);
}

void MainWindow::show_advanced()
{
      ui->AdvancedSettings->setVisible(true);
}

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

void MainWindow::zoom_plus(){
    cout << "zoom clicked" << endl;
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

void MainWindow::rotate(int rotation){
    QTransform rotating;
    rotating.rotate(rotation);



    switch(selectedWindow){
    case 1:
        for(int i =0; i< Images.size(); i ++)
            *Images[i] = Images[i]->transformed(rotating);
        myScene->addPixmap( QPixmap::fromImage( *Images[Index[0]] ) );
        break;
    case 2:
        for(int i =0; i< Images2.size(); i ++)
            *Images2[i] = Images2[i]->transformed(rotating);
        myScene2->addPixmap( QPixmap::fromImage( *Images2[Index[1]] ) );
        break;
    case 3:
        for(int i =0; i< Images3.size(); i ++)
            *Images3[i] = Images3[i]->transformed(rotating);
        myScene3->addPixmap( QPixmap::fromImage( *Images3[Index[2]] ) );
        break;
    case 4:
        for(int i =0; i< Images4.size(); i ++)
            *Images4[i] = Images4[i]->transformed(rotating);
        myScene4->addPixmap( QPixmap::fromImage( *Images4[Index[3]] ) );
        break;
    default:
        break;



    }


}


void MainWindow::rotate_left(){
    rotate(-90);
}

void MainWindow::rotate_right(){
    rotate(90);

}


void MainWindow::default_contrast(){
    Images.clear();
    if(contrast[0] !=0){
        contrast[0]=0;
        createDefaultPlan();
    }
}


void MainWindow::minmax_contrast(){
    Images.clear();
    if(contrast[0] !=1){
        contrast[0]=1;
        createDefaultPlan();
    }
}

void MainWindow::histo_contrast(){
    Images.clear();
    if(contrast[0] !=2){
        contrast[0]=2;
        createDefaultPlan();
    }
}

void MainWindow::on_showReport_clicked()
{
    report->show();
}

void MainWindow::callAxial(){
    clearImagesVector();
    constructAxialPlan();
}

void MainWindow::callCoronal(){
    clearImagesVector();
    constructCoronalPlan();
}

void MainWindow::callSagittal(){
    clearImagesVector();
    constructSagittalPlan();
}

void MainWindow::callTest(){
    cout << "callTest"<< endl;
    QPixmap pixmap = QPixmap::fromImage( *Images[Index[0]] );
    QPainter p(&pixmap);


    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(Qt::red, 2));
    p.drawLine(0, 0,0, height);
    p.end(); // Don't forget this line!

    myScene->addPixmap( pixmap );


}

void MainWindow::link_views(){
    viewConnected= true;




}


