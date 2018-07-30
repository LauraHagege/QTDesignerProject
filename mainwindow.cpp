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
    displayImages();

    delete filepath;
}

void MainWindow::processDicom(const char *dicomdirPath, char *filepath){
    //counter for the number of series found in the file
    series=0;

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


    currentSerie = "serie2";


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

                                //DCM_ReferencedFileID give the name of the DICOM Image file
                                if(FileRecord->findAndGetOFStringArray(DCM_ReferencedFileID,tmpString).good()){
                                    //add image name to file path to get the full path
                                    fullpath = filepath + string(tmpString.c_str());
                                }

                                //Check file status, if not EIS_Normal the file is not an Image
                                //If not normal status the "parent" is not a serie

                                if(count ==1){
                                    if (DicomImage(fullpath.c_str()).getStatus() != EIS_Normal)
                                        count =0;
                                }

                                // add path to the local vector
                                 paths.push_back(fullpath);


                                 if (count ==1){
                                    if(FileRecord->findAndGetOFStringArray(DCM_ImagePositionPatient,tmpString).good()){
                                         cout << "image posiition patient " << tmpString.c_str() << endl;
                                    }
                                    if(FileRecord->findAndGetOFStringArray(DCM_ImageOrientationPatient,tmpString).good()){
                                         cout << "image orientation patient " << tmpString.c_str() << endl;
                                    }
                                 }
                            }

                            //count !=0 means the folder contain DICOM images so this is a serie
                            if(count !=0){
                                series +=1;
                                char *desc=new char[100];
                                char *date=new char[10];

                                if (SeriesRecord->findAndGetOFString(DCM_SeriesDescription, tmpString).good())
                                    strcpy(desc,tmpString.c_str());


                                if (SeriesRecord->findAndGetOFString(DCM_Date, tmpString).good())
                                    strcpy(date,tmpString.c_str());


                                addSerieButton(series,desc,date, count);

                                //storing paths for the current serie
                                allPath.insert(pair<string,vector<string>>("serie"+to_string(series),paths));

                            }
                        }
                    }
                }
            }

}

void MainWindow::addSerieButton(int serieNumber, char *serieDescription, char *date, int imgNb){
    //adding button for each serie
    char serieName[30]="Series";

    strcat(serieName,to_string(serieNumber).c_str());

    QPushButton *button = new QPushButton(this);

    button->setText(tr(serieName));
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

void MainWindow::mousePressEvent(QMouseEvent* e){
    if(ui->graphicsView->underMouse()){
        selectedWindow=1;
        ui->graphicsView->setFrameStyle(3);
        ui->graphicsView_2->setFrameStyle(1);
        ui->graphicsView_3->setFrameStyle(1);
        ui->graphicsView_4->setFrameStyle(1);
    }
    else if(ui->graphicsView_2->underMouse()){
        selectedWindow=2;
        ui->graphicsView->setFrameStyle(1);
        ui->graphicsView_2->setFrameStyle(3);
        ui->graphicsView_3->setFrameStyle(1);
        ui->graphicsView_4->setFrameStyle(1);
    }
    else if(ui->graphicsView_3->underMouse()){
        selectedWindow=3;
        ui->graphicsView->setFrameStyle(1);
        ui->graphicsView_2->setFrameStyle(1);
        ui->graphicsView_3->setFrameStyle(3);
        ui->graphicsView_4->setFrameStyle(1);
    }
    else if(ui->graphicsView_4->underMouse()){
        selectedWindow=4;
        ui->graphicsView->setFrameStyle(1);
        ui->graphicsView_2->setFrameStyle(1);
        ui->graphicsView_3->setFrameStyle(1);
        ui->graphicsView_4->setFrameStyle(3);
    }


}


void MainWindow::displayImages(){
    //initializing index for images
    Index = new int[4];
    for(int i=0; i<4; i++){
        Index[i]=0;
    }

    vector<DicomImage*> DicomImages;
    myPixelsZ.clear();

    for(int i=0; i<allPath[currentSerie].size(); i++){
        //cout << "path " << allPath[currentSerie][i] << endl;
        DicomImages.push_back(new DicomImage(allPath[currentSerie][i].c_str()));
        //DiconImage img = new DicomImage(path);


    }

    //**********DEALING WITH ALL IMAGES OF THE SERIE**********//


    int check=true;
    for(int i=0; i<DicomImages.size(); i++){
        if(DicomImages[i] == NULL)
            check=false;
        else if(!(DicomImages[i]->getStatus() == EIS_Normal))
            check=false;
    }


    if (check){
        //if (DicomImages[0]->getStatus() == EIS_Normal && DicomImages[1]->getStatus() == EIS_Normal)
        cout << "width " << DicomImages[0]->getWidth() << endl;
        cout << "height "<<  DicomImages[0]->getHeight() << endl;
        for(int i=0; i<DicomImages.size(); i++){
            DicomImages[0] ->setMinMaxWindow();
            Uint8* pixelData = (Uint8 *)(DicomImages[i]->getOutputData(8 )); // bits per sample
            myPixelsZ.push_back((uint8_t *)pixelData);

            uint8_t* pixelData2 = (uint8_t *)(DicomImages[i]->getOutputData(8 ));


            if (pixelData != NULL){
                // do something useful with the pixel data
                QImage *img=new QImage (pixelData,DicomImages[0]->getWidth(), DicomImages[0]->getHeight(), QImage::Format_Indexed8);
                //QImage *copy = new QImage(img->createHeuristicMask());
                Images.push_back(img);
                //delete img;

                for( int j = 0; j < 256; ++j )
                    Images[i]->setColor(j, qRgb(j,j,j));
            }
            }
    } else
            cerr << "Error: cannot load DICOM image (" << DicomImage::getString(DicomImages[0]->getStatus()) << ")" << endl;


   constructPlans((int)DicomImages[0]->getWidth(),(int)DicomImages[0]->getHeight());

    //creating scene

    myScene= new QGraphicsScene(this);
    myScene->addPixmap( QPixmap::fromImage( *Images[0] ) );

   // Cscene =  new CustomGraphicsScene();

    ui->graphicsView->setScene(myScene);
    //ui->graphicsView->setScene(Cscene);




    ui->graphicsView->fitInView(myScene->sceneRect(),Qt::KeepAspectRatioByExpanding);

    ui->graphicsView->fitInView(QRectF(0,0,ui->graphicsView->width(), ui->graphicsView->height()),Qt::KeepAspectRatio);

    ui->graphicsView->setFrameRect(QRect(0,0,ui->graphicsView->width(), ui->graphicsView->height()));
    ui->graphicsView->setFrameStyle(3);
    selectedWindow=1;


    //connect(myScene, SIGNAL(mousePressEvent(QGraphicsSceneMouseEvent*)), SLOT(scene_clicked(QGraphicsScene*)));

}



void MainWindow::constructPlans(int width, int height){

   // int countY=0;

    int depth=myPixelsZ.size();
    //cout << "depth " << depth << " width " << width << " height " << height << endl;

    //X fixed <-> width
    for (int x=0; x<width; x++){
       uint8_t *mypixel= new uint8_t[height*depth];
       int countX=0;
        for(int z=0; z<depth; z++){
            int i=0;
            for(int y=x; y<height*width ; y+=width){
                mypixel[countX]=myPixelsZ[z][y];
                countX +=1;
                i+=1;
            }
        }
            myPixelsX.push_back(mypixel);
            QImage *img= new QImage(mypixel,height, depth, QImage::Format_Indexed8);
            QImage *copy =  new QImage(img->scaled(QSize(height,1.8*depth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
            delete img;
        Images2.push_back(copy);
//        for( int j = 0; j < 256; ++j )
//            Images2[x]->setColor(j, qRgb(j,j,j));
    }

    cout << "size " << myPixelsZ.size() << endl;
    myScene2= new QGraphicsScene(this);
    ui->graphicsView_2->setScene(myScene2);
    myScene2->addPixmap( QPixmap::fromImage( *Images2[1] ) );

    ui->graphicsView_2->fitInView(QRectF(0,0,ui->graphicsView_2->width(), ui->graphicsView_2->height()),Qt::KeepAspectRatio);
    ui->graphicsView_2->setFrameRect(QRect(0,0,ui->graphicsView_2->width(), ui->graphicsView_2->height()));

    ui->graphicsView_2->setFrameStyle(1);

    cout << "y now" << endl;
    //Y fixed <-> width
    for (int y=0; y<height; y++){
       uint8_t *mypixel= new uint8_t[width*depth];
       int countY=0;
        for(int z=0; z<depth; z++){
            int l=0;
            for(int x=y*width; x<(y+1)*width ; x++){
                mypixel[countY]=myPixelsZ[z][x];
                countY +=1;
                l+=1;
            }
        }
            myPixelsY.push_back(mypixel);
            QImage *img = new QImage (mypixel,width, depth, QImage::Format_Indexed8);
            QImage *copy =  new QImage(img->scaled(QSize(height,1.8*depth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation));
            delete img;

        Images3.push_back(copy);
//        for( int j = 0; j < 256; ++j )
//            Images3[y]->setColor(j, qRgb(j,j,j));
    }

    //cout << "size " << myPixelsZ.size() << endl;
    myScene3= new QGraphicsScene(this);
    ui->graphicsView_3->setScene(myScene3);
    myScene3->addPixmap( QPixmap::fromImage( *Images3[1] ) );

    ui->graphicsView_3->fitInView(QRectF(0,0,ui->graphicsView_3->width(), ui->graphicsView_3->height()),Qt::KeepAspectRatio);
    ui->graphicsView_3->setFrameRect(QRect(0,0,ui->graphicsView_3->width(), ui->graphicsView_3->height()));
    ui->graphicsView_3->setFrameStyle(1);


    myScene4= new QGraphicsScene(this);
    ui->graphicsView_4->setScene(myScene4);
    ui->graphicsView_4->setFrameRect(QRect(0,0,ui->graphicsView_4->width(), ui->graphicsView_4->height()));
    ui->graphicsView_4->setFrameStyle(1);

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

    connect(ZoomPlus, SIGNAL(triggered(bool)), SLOT(zoomPlus()));

    QAction *ZoomMinus= new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/zoom-minus.png"),"Zoom",this);
    ui->mainToolBar->addAction(ZoomMinus);

    connect(ZoomMinus, SIGNAL(triggered(bool)), SLOT(zoomMinus()));

    ui->mainToolBar->addSeparator();

    QAction *Flag = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/flag.png"),"Flag Information", this);
    ui->mainToolBar->addAction(Flag);

    ui->mainToolBar->addSeparator();

    QAction *Display1Window = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/w1.png"),"Show only one window", this);
    ui->mainToolBar->addAction(Display1Window);
    connect(Display1Window,SIGNAL(triggered(bool)),this,SLOT(on_w1_clicked()) );

    QAction *Display2Window = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/w2.png"),"Show only one window", this);
    ui->mainToolBar->addAction(Display2Window);
    connect(Display2Window,SIGNAL(triggered(bool)),this,SLOT(on_w2_clicked()) );

    QAction *Display4Window = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/w4.png"),"Show only one window", this);
    ui->mainToolBar->addAction(Display4Window);
    connect(Display4Window,SIGNAL(triggered(bool)),this,SLOT(on_w4_clicked()) );

    ui->mainToolBar->addSeparator();

    QAction *Label = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/label.png"),"Show labels", this);
    ui->mainToolBar->addAction(Label);

    ui->mainToolBar->addSeparator();

    QAction *Scroll = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/scroll.png"),"Scroll Images", this);
    ui->mainToolBar->addAction(Scroll);

    ui->mainToolBar->addSeparator();

    QAction *Examples = new QAction("Show me examples", this);
    ui->mainToolBar->addAction(Examples);

    ui->mainToolBar->addSeparator();

    QAction *Reset = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/reset.png"),"Reset", this);
    ui->mainToolBar->addAction(Reset);

    ui->mainToolBar->addSeparator();

    QAction *Advanced = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/add.png"),"Advanced settings", this);
    ui->mainToolBar->addAction(Advanced);
    connect(Advanced,SIGNAL(triggered(bool)),this,SLOT(on_Advanced_clicked()) );

    ui->mainToolBar->addSeparator();


    //creating advanced setting tool bar
    ui->AdvancedSettings->setVisible(false);

    QAction *Invert = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/grayscale.png"),"Invert Grayscale", this);
    ui->AdvancedSettings->addAction(Invert);

    //setting invert var to zero = normal grayscale displayed
    invertGrayScale=0;

    QAction *Presets = new QAction("Contrast Presets", this);
    ui->AdvancedSettings->addAction(Presets);

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

    QAction *Hide = new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/remove.png"),"Hide Settings", this);
    ui->AdvancedSettings->addAction(Hide);
    connect(Hide,SIGNAL(triggered(bool)),this,SLOT(on_Hide_clicked()) );


    ui->Informations->setLayout(ui->SeriesLayout);






}



//********************************************************************************//
//---------------------ALL PRIVATE SLOTS FOLLOWING -----------------------------//
//********************************************************************************//


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
    string buttonName = b->text().toLocal8Bit().constData();
    cout << "button " << buttonName << " clicked, associated serie size: "<< this->allPath[buttonName].size() << endl;
    currentSerie=buttonName;
    cout << "currentSeriesize " <<this->allPath[currentSerie].size() << endl;

    Images.clear();
    displayImages();
}



void MainWindow::on_Hide_clicked()
{
    ui->AdvancedSettings->setVisible(false);
}

void MainWindow::on_InvertGray_clicked()
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

void MainWindow::on_Advanced_clicked()
{
      ui->AdvancedSettings->setVisible(true);
}

void MainWindow::on_w1_clicked()
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

void MainWindow::on_w2_clicked()
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

void MainWindow::on_w4_clicked()
{
    ui->graphicsView_2->setVisible(true);
    ui->graphicsView_3->setVisible(true);
    ui->graphicsView_4->setVisible(true);
}

void MainWindow::zoomPlus(){
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

void MainWindow::zoomMinus(){
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
