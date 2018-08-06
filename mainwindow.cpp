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
    nbOfFrame=0;

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

    int WC1,WW1;


    if(root != NULL)
            {
                //Find next folder
                while (((PatientRecord = root->nextSub(PatientRecord)) != NULL))
                {
                    while (((StudyRecord = PatientRecord->nextSub(StudyRecord)) != NULL))
                    {
                        if (StudyRecord->findAndGetOFString(DCM_PatientName, tmpString).good()) {
                            cout << "PatientName 1: " << tmpString.c_str() << endl ;
                        }
                        if (StudyRecord->findAndGetOFString(DCM_PatientBirthDate, tmpString).good()) {
                            cout << "birthdate 1 : " << tmpString.c_str() << endl ;
                        }
                        if (StudyRecord->findAndGetOFString(DCM_StudyDescription, tmpString).good()) {
                            cout <<"Study Description: " << tmpString.c_str() << endl ;
                        }
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


                                 if(count ==1){
                                    if(FileRecord->findAndGetOFString(DCM_WindowCenter,tmpString).good()){
                                        cout << "window center " << tmpString.c_str() << endl;
                                        WC1=atoi(tmpString.c_str());

                                    }
                                    if(FileRecord->findAndGetOFString(DCM_WindowWidth,tmpString).good()){
                                        cout << "window width " << tmpString.c_str() << endl;
                                        WW1= atoi(tmpString.c_str());
                                    }
                                 }


                                 if (count ==1 && (series ==1 || series ==2)){
                                    if(FileRecord->findAndGetOFStringArray(DCM_ImageOrientationPatient,tmpString).good()){
                                         cout << "image posiition patient " << tmpString.c_str() << endl;
                                    }
                                    if(FileRecord->findAndGetOFStringArray(DCM_NumberOfFrames,tmpString).good()){
                                         cout << "Nb of Frames " << tmpString.c_str() << endl;
                                         nbOfFrame= atoi(tmpString.c_str());
                                        // cout << "test " << nbOfFrame << endl;
                                    }


                                 }
                            }

                            //count !=0 means the folder contain DICOM images so this is a serie
                            if(count !=0){
                                series +=1;

                                cout << "series nb " <<series << "with "<< count <<" images" << endl;

                                char *desc=new char[100];
                                char *date=new char[10];

                                if (SeriesRecord->findAndGetOFString(DCM_SeriesDescription, tmpString).good())
                                    strcpy(desc,tmpString.c_str());


                                if (SeriesRecord->findAndGetOFString(DCM_Date, tmpString).good())
                                    strcpy(date,tmpString.c_str());


                                addSerieButton(series,desc,date, count);

                                WC.push_back(WC1);
                                WW.push_back(WW1);




                                //SUPPOSSE HERE I HAVE A WAY TO DET THE SERIE CURRENT PLAN
                                // FOR NOW BY DEFAULT I PU AXIAL FOR ALL SERIES
                                seriesPlan.push_back(Sagittal);


                                //storing paths for the current serie
                                allPath.insert(pair<string,vector<string>>("Series"+to_string(series),paths));

                            }
                        }
                    }
                }
            }

    currentPlan = seriesPlan[currentSerieNumber-1];

}

void MainWindow::addSerieButton(int serieNumber, char *serieDescription, char *date, int imgNb){
    //adding button for each serie
    char serieName[30]="Series";

    strcat(serieName,to_string(serieNumber).c_str());

    QPushButton *button = new QPushButton(this);

    button->setText(tr(serieName));
    button->setProperty("Id",serieNumber);
    button->setVisible(true);
    buttonGroup->addButton(button);

    QLabel *SerieDesc= new QLabel(this);
    QLabel *SerieDate = new QLabel(this);
    QLabel *SerieImgNb = new QLabel(this);

    SerieDesc->setText(serieDescription) ;
    SerieDate->setText(date) ;


    char Images[15]= "Images: ";
    strcat(Images, to_string(imgNb).c_str());
    SerieImgNb->setText(Images);

    QFrame *SerieFrame = new QFrame(this);
    SerieFrame->setFrameShape(QFrame::HLine);
    SerieFrame->setFrameShadow(QFrame::Sunken);
    SerieFrame->setLineWidth(1);

    ui->SeriesLayout->addWidget(button);
    ui->SeriesLayout->addWidget(SerieDesc);
    ui->SeriesLayout->addWidget(SerieDate);
    ui->SeriesLayout->addWidget(SerieImgNb);
    ui->SeriesLayout->addWidget(SerieFrame);

}




void MainWindow::createDefaultPlan(){
    //initializing index for images
    Index = new int[4];
    for(int i=0; i<4; i++){
        Index[i]=0;
    }

    vector<DicomImage*> DicomImages;
    myPixelsZ.clear();
    myPixelsX.clear();
    myPixelsY.clear();


    int nbImage = (int)allPath[currentSerie].size();


    for(int i=0; i< nbImage ; i++){
       // DicomImage * img =
        if(nbOfFrame ==0)
            DicomImages.push_back(new DicomImage(allPath[currentSerie][i].c_str()));
        else {
        //DiconImage img = new DicomImage(path);
             DicomImages.push_back(new DicomImage(allPath[currentSerie][i].c_str(),0,0,nbOfFrame));
            //indexFrame=DicomImages[i]->getFirstFrame() ;
            //cout << "nb frame " << nbOfFrame << endl;
            //cout << "first frame" << DicomImages[i]->getFirstFrame() << endl;


        }


    }

    //**********DEALING WITH ALL IMAGES OF THE SERIE**********//
    width = DicomImages[0]->getWidth(); // height and with are the default argument defined for the default plan of the Image
    height =  DicomImages[0]->getHeight();



    switch(currentPlan){
    case Axial:
        pixelXdepth = DicomImages[0]->getWidth();
        pixelYdepth =  DicomImages[0]->getHeight();
        pixelZdepth = nbImage; //  nbImage is not properly the depth, but this is the value that will be used to recreate the other plans
        break;
    case Coronal:
        pixelZdepth = DicomImages[0]->getHeight();
        pixelXdepth =  DicomImages[0]->getWidth();
        pixelYdepth = nbImage;

        break;
    case Sagittal:
        pixelZdepth = DicomImages[0]->getHeight();
        pixelYdepth =  DicomImages[0]->getWidth();
        pixelXdepth = nbImage;

        break;
    default:
        break;

    }


    cout << "width " << width <<" height " << height << endl;



    if(nbOfFrame ==0){

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

                Images.push_back(img);


            }
       }
   }
   else
      cerr << "Error: cannot load DICOM image (" << DicomImage::getString(DicomImages[0]->getStatus()) << ")" << endl;
}

    else {
        if(DicomImages[0]->getStatus() == EIS_Normal){
//            if (contrast==0)
//                DicomImages[0]->setWindow(WC,WW);
//            else if (contrast ==1)
//                DicomImages[0] ->setMinMaxWindow();
//            else
//                DicomImages[0] ->setHistogramWindow();


            Uint8* pixelData = (Uint8 *)(DicomImages[0]->getOutputData(8), nbOfFrame-1);
            if (pixelData != NULL)
                cout << "no problem"  << endl;
            else
                cout << "no pixel";

            //DicomImages[0]->processNextFrames();
            for(int i=0; i< nbOfFrame ; i ++){
               // cout << "processsed  " << DicomImages[0]->getFrameCount() <<  endl;
                //DicomImages[0]->processNextFrames(nbOfFrame-i);


                uint8_t* pixelFrameData = new uint8_t[width*height];
                int index=0;
                //cout << "there " << endl;
                for(int j =i* width*height; j <(i+1)* width*height ; j++ ){

                    pixelFrameData[index]= (uint8_t)pixelData[j];

                        //cout << "idk" << endl;
                    index+=1;
                }

               // cout << "there after " << endl;
                myPixelsZ.push_back((uint8_t *)pixelFrameData);

                if (pixelFrameData != NULL){
                    // do something useful with the pixel data
                    QImage *img=new QImage (pixelFrameData,width,height, QImage::Format_Indexed8);

                    Images.push_back(img);

                }


                //cout << "index frame " << indexFrame << endl;
            }


        }
        else
            cerr << "Error: cannot load DICOM image (" << DicomImage::getString(DicomImages[0]->getStatus()) << ")" << endl;


    }

    cout << "nb of images " << Images.size() << endl;

    Index[0]= Images.size()/2;
    cout << "RATIO "<< Images[Index[0]]->devicePixelRatio() << endl;;

    //creating scene

    createScene();

}

void MainWindow::createScene(){
    switch(selectedWindow){
    case 1:
        myScene= new QGraphicsScene(this);
        myScene->addPixmap( QPixmap::fromImage( *Images[Index[0]] ) );

        ui->graphicsView->setBackgroundBrush(QBrush(Qt::black));
        ui->graphicsView->setScene(myScene);

        if(creation[0]==0){
             creation[0] =1;
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
        }

        break;
    case 2:
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

            QImage *copy =  new QImage(img->scaled(QSize(pixelYdepth,1.8*pixelZdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
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
        // TO DOOOOOOO
        cout << "pixelXdepth "<< pixelXdepth << endl;
        cout << "pixelYdepth "<< pixelYdepth << endl;
        cout << "pixelZdepth "<< pixelZdepth << endl;


        for (int x=0; x<pixelXdepth; x++){
           uint8_t *mypixel= new uint8_t[pixelZdepth*pixelYdepth];
           int countX=0;
            for(int y=0; y<pixelYdepth; y++){
                for(int z=x*pixelZdepth; z<(x+1)*pixelZdepth ; z++){
                    mypixel[countX]=myPixelsY[y][z];
                    countX +=1;
                }
            }

            if(x==0)
                cout << "x0 pass" << endl;

            myPixelsX.push_back(mypixel);
            QImage *img= new QImage(mypixel,pixelYdepth, pixelZdepth, QImage::Format_Indexed8);

            //CHECK FOR THE RIGTH SCALING

            QImage *copy =  new QImage(img->scaled(QSize(pixelYdepth,1.8*pixelZdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
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
        Index[0]=Images.size()/2;
        break;
    case 2:
        Index[1]=Images2.size()/2;
        break;
    case 3:
        Index[2]=Images3.size()/2;
        break;
    case 4:
        Index[3]=Images4.size()/2;
    }
    createScene();

}



void MainWindow::constructAxialPlan(){
    if(currentPlan == Sagittal){

        //X fixed <-> width
        for (int z=0; z<pixelZdepth; z++){
           uint8_t *mypixel= new uint8_t[pixelYdepth*pixelXdepth];
           int countZ=0;
            for(int x=0; x<pixelXdepth; x++){
                for(int y=x; y<pixelXdepth*pixelYdepth ; y+=pixelXdepth){
                    mypixel[countZ]=myPixelsZ[z][y];
                    countZ +=1;
                }
            }

            myPixelsX.push_back(mypixel);
            QImage *img= new QImage(mypixel,pixelYdepth, pixelZdepth, QImage::Format_Indexed8);

            //CHECK FOR THE RIGTH SCALING

            QImage *copy =  new QImage(img->scaled(QSize(pixelYdepth,1.8*pixelZdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
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



    }

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
                QImage *copy =  new QImage(img->scaled(QSize(pixelXdepth,1.8*pixelZdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
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
        cout << "pixelXdepth "<< pixelXdepth << endl;
        cout << "pixelYdepth "<< pixelYdepth << endl;
        cout << "pixelZdepth "<< pixelZdepth << endl;

        for (int y=0; y<pixelYdepth; y++){
           uint8_t *mypixel= new uint8_t[pixelXdepth*pixelZdepth];
           int countY=0;
            for(int z=0; z<pixelZdepth; z++){
                for(int x=0; x<pixelXdepth ; x++){
                    mypixel[countY]=myPixelsX[x][z];
                    countY +=1;
                }
            }
                myPixelsY.push_back(mypixel);
                QImage *img = new QImage (mypixel,pixelXdepth, pixelZdepth, QImage::Format_Indexed8);
                QImage *copy =  new QImage(img->scaled(QSize(pixelXdepth,pixelZdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
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
        Index[0]=Images.size()/2;
        break;
    case 2:
        Index[1]=Images2.size()/2;
        break;
    case 3:
        Index[2]=Images3.size()/2;
        break;
    case 4:
        Index[3]=Images4.size()/2;
        break;
    }
    createScene();

}



void MainWindow::createButtons(){
    ui->mainToolBar->setIconSize(QSize(33,33));
    ui->AdvancedSettings->setIconSize(QSize(33,33));

    //Personnal Information
    QLabel *perso =new  QLabel("Personal Information");
    ui->PersonalInfo->addWidget(perso);

    //Adding button to the toolbar
    QAction *ZoomPlus= new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/zoom-plus.png"),"Zoom",this);
    ui->mainToolBar->addAction(ZoomPlus);

    connect(ZoomPlus, SIGNAL(triggered(bool)), SLOT(zoom_plus()));

    QAction *ZoomMinus= new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/zoom-minus.png"),"Zoom",this);
    ui->mainToolBar->addAction(ZoomMinus);

    connect(ZoomMinus, SIGNAL(triggered(bool)), SLOT(zoom_minus()));

    ui->mainToolBar->addSeparator();

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

    QAction *Axial = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/axial.png"),"Show top/bottom view (Axial)", this);
    ui->mainToolBar->addAction(Axial);
    connect(Axial, SIGNAL(triggered(bool)), SLOT(callAxial()));

    QAction *Coronal = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/coronal.png"),"Show front/back view (Coronal)", this);
    ui->mainToolBar->addAction(Coronal);
    connect(Coronal, SIGNAL(triggered(bool)), SLOT(callCoronal()));

    QAction *Sagittal = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/sagittal.png"),"Show left/right view (Sagittal)", this);
    ui->mainToolBar->addAction(Sagittal);
    connect(Sagittal, SIGNAL(triggered(bool)), SLOT(callSagittal()));



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
                Index[0]= Images.size()-1;

            myScene->addPixmap( QPixmap::fromImage( *Images[Index[0]] ) );
            ui->graphicsView->setScene(myScene);
        }else if(selectedWindow==2){
            Index[1]-=1;
            if(Index[1]<0)
                Index[1]=Images2.size()-1;;

            myScene2->addPixmap( QPixmap::fromImage( *Images2[Index[1]] ) );
            ui->graphicsView_2->setScene(myScene2);
        }
        else if(selectedWindow==3){
            Index[2]+=1;
            if(Index[2]<0)
                Index[2]=Images3.size()-1;;

            myScene3->addPixmap( QPixmap::fromImage( *Images3[Index[2]] ) );
            ui->graphicsView_3->setScene(myScene3);
        }


    }
}

void MainWindow::buttonInGroupClicked(QAbstractButton *b){
    currentSerieNumber = b->property("Id").toInt();
    cout << "property number  " << currentSerieNumber<<endl;
    string buttonName = b->text().toLocal8Bit().constData();
    cout << "button " << buttonName << " clicked, associated serie size: "<< this->allPath[buttonName].size() << endl;
    currentSerie=buttonName;
    cout << "currentSeriesize " <<this->allPath[currentSerie].size() << endl;

    Images.clear();
    createDefaultPlan();
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
    constructAxialPlan();
}

void MainWindow::callCoronal(){
    constructCoronalPlan();
}

void MainWindow::callSagittal(){
    constructSagittalPlan();
}

void MainWindow::callTest(){
    currentPlan=Test;
    constructSagittalPlan();
}

