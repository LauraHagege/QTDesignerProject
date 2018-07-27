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
#include <QGraphicsScene>
#include <QGraphicsView>
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

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::setFilepath(QString path){
    createButtons();
    invert=0;
    series=0;
    //QMessageBox::information(this,tr("Directory path"), path);

    QByteArray ba = path.toLatin1();
    const char *file = ba.data();

    int size = strlen(file);
    char *filepath=new char[130];

    for(int i=0; i<size-8; i++){
        filepath[i]=file[i];
    }
    filepath[size-8]='\0';

    cout << "path " << filepath <<endl;

    DcmDicomDir myDir(file);

   // DcmFileFormat myFile = myDir ->getDirFileFormat(); //&fileformat ?
    DcmDirectoryRecord *   root = &(myDir.getRootRecord());
    DcmDirectoryRecord *   PatientRecord = NULL;
    DcmDirectoryRecord *   StudyRecord = NULL;
    DcmDirectoryRecord *   SeriesRecord = NULL;
    DcmDirectoryRecord *   FileRecord = NULL;
    OFString            tmpString;

   // int count =0;

    currentSerie = "serie2";


    if(root != NULL)
            {
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

                            //vector to store paths to files
                            vector<string> paths;

                            //vector to store images of the current selected serie
                            vector<QImage*> CurrentSerie;

                            int count =0;


                            int counter=0;
                            while((FileRecord = SeriesRecord->nextSub(FileRecord)) != NULL){
                                string fullpath;
                                 if(FileRecord->findAndGetOFStringArray(DCM_ReferencedFileID,tmpString).good()){
                                     fullpath = filepath + string(tmpString.c_str());
                                 }
                                 count +=1;
                                 if(count ==1){
                                     if (DicomImage(fullpath.c_str()).getStatus() != EIS_Normal)
                                         count =0;
                                 }

                                 paths.push_back(fullpath);


                                /*if(FileRecord->findAndGetOFString(DCM_ImageType,tmpString).good()){
                                     cout <<"Img description: " <<tmpString.c_str() << endl ;
                                }
                               */


                                int column, row, pixelspacing, patientposition;
//                                if(series==1 && counter==0){
//                                    counter +=1;

//                                    if(FileRecord->findAndGetOFStringArray(DCM_ImagePositionPatient,tmpString).good()){
//                                        cout << " image position : " <<tmpString.c_str() << endl ;
//                                        patientposition= atoi(tmpString.c_str());
//                                    }
//                                    if(FileRecord->findAndGetOFStringArray(DCM_ImageOrientationPatient,tmpString).good()){
//                                        cout << " image orientation : " <<tmpString.c_str() << endl ;
//                                        patientposition= atoi(tmpString.c_str());
//                                    }
                                    if(FileRecord->findAndGetOFStringArray(DCM_PixelSpacing,tmpString).good()){
                                        cout << " Pixel spacing : " <<tmpString.c_str() << endl ;
                                        pixelspacing= atoi(tmpString.c_str());
                                   }
//                                    if(FileRecord->findAndGetOFStringArray(DCM_Columns,tmpString).good()){
//                                        cout << " Column : " <<tmpString.c_str() << endl ;
//                                        //column= atoi(tmpString.c_str());
//                                    }
//                                    if(FileRecord->findAndGetOFStringArray(DCM_Rows,tmpString).good()){
//                                        cout << " Rows : " <<tmpString.c_str() << endl ;
//                                        //row= atoi(tmpString.c_str());
//                                    }

//                                    cout << "fullpath: " << fullpath << endl;

//                                }


                                    //DicomImages.push_back(new DicomImage(path_to_file));

                            }


                            if(count !=0){
                                //adding button for each serie
                                char serieName[30]="Series";
                                series +=1;
                                strcat(serieName,to_string(series).c_str());

                                QPushButton *button = new QPushButton(this);

                                button->setText(tr(serieName));
                                button->setVisible(true);
                                buttonGroup->addButton(button);


                               // ui->verticalLayout_2->addWidget(button);

                                QLabel *SerieDesc= new QLabel(this);
                                QLabel *SerieDate = new QLabel(this);
                                QLabel *SerieImgNb = new QLabel(this);

                                //adding serie information
                                if (SeriesRecord->findAndGetOFString(DCM_SeriesDescription, tmpString).good()) {
                                    cout <<"Serie Description: " << tmpString.c_str() << endl ;
                                    SerieDesc->setText(tmpString.c_str()) ;
                                }
                                if (SeriesRecord->findAndGetOFString(DCM_Date, tmpString).good()) {
                                    cout <<"Serie date: " << tmpString.c_str() << endl ;
                                    SerieDate->setText(tmpString.c_str()) ;
                                }

                                char Images[15]= "Images: ";
                                strcat(Images, to_string(count).c_str());
                                SerieImgNb->setText(Images);

                                QFrame *SerieFrame = new QFrame(this);
                                SerieFrame->setFrameShape(QFrame::HLine);
                                SerieFrame->setFrameShadow(QFrame::Sunken);
                                SerieFrame->setLineWidth(1);

//                                QVBoxLayout *SerieLayout = new QVBoxLayout(this);
//                                SerieLayout->addWidget(button);
//                                SerieLayout->addWidget(SerieDesc);
//                                SerieLayout->addWidget(SerieDate);
//                                SerieLayout->addWidget(SerieImgNb);
//                                SerieLayout->addWidget(SerieFrame);

//                                QWidget *serieWidget = new QWidget(this);
//                                serieWidget->setLayout(SerieLayout);
//                                ui->verticalLayout_2->addWidget(serieWidget);

                                ui->verticalLayout_2->addWidget(button);
                                ui->verticalLayout_2->addWidget(SerieDesc);
                                ui->verticalLayout_2->addWidget(SerieDate);
                                ui->verticalLayout_2->addWidget(SerieImgNb);
                                ui->verticalLayout_2->addWidget(SerieFrame);



                                //storing paths for the serie
                                allPath.insert(pair<string,vector<string>>("serie"+to_string(series),paths));

                            }


                        }
                    }
                }
            }

    displayImages();

    delete filepath;
}




void MainWindow::wheelEvent(QWheelEvent *event)
{

    index+=1;
    if(index+1 > Images.size())
        index=0;

    myScene->addPixmap( QPixmap::fromImage( *Images[index] ) );
    ui->graphicsView->setScene(myScene);



    index2+=1;
    if(index2+1 > Images2.size())
        index2=0;

    myScene2->addPixmap( QPixmap::fromImage( *Images2[index2] ) );
    ui->graphicsView_2->setScene(myScene2);


    index3+=1;
    if(index3+1 > Images3.size())
        index3=0;

    myScene3->addPixmap( QPixmap::fromImage( *Images3[index3] ) );
    ui->graphicsView_3->setScene(myScene3);
}

void MainWindow::buttonInGroupClicked(QAbstractButton *b){
    string buttonName = b->text().toLocal8Bit().constData();
    cout << "button " << buttonName << " clicked, associated serie size: "<< this->allPath[buttonName].size() << endl;
    currentSerie=buttonName;
    cout << "currentSeriesize " <<this->allPath[currentSerie].size() << endl;

    Images.clear();
    displayImages();
}

void MainWindow::displayImages(){
    index=0;
    index2=0;
    index3=0;
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

//            if(i==50){
//                int size=DicomImages[i]->getOutputDataSize(8);
//                cout << "size "<<size << endl;
//                cout << "pixels " << endl;
//                for(int j=0; j<200 ; j++){
//                    cout << to_string(pixelData2[j]) << " " ;
//                    if(j% DicomImages[i]->getWidth() ==0){
//                        cout<< endl;
//                    }
//                }

//                cout << "pixels2 " << endl;
//                for(int j=0; j<200 ; j++){
//                    cout << to_string( myPixelsZ[12][j])  << " ";
//                    if(j% DicomImages[i]->getWidth() ==0){
//                        cout<< endl;
//                    }
//                }


//            }



            if (pixelData != NULL){
                // do something useful with the pixel data
                Images.push_back(new QImage (pixelData,DicomImages[0]->getWidth(), DicomImages[0]->getHeight(), QImage::Format_Indexed8));
//                for( int j = 0; j < 256; ++j )
//                    Images[i]->setColor(j, qRgb(j,j,j));
            }
            }
    } else
            cerr << "Error: cannot load DICOM image (" << DicomImage::getString(DicomImages[0]->getStatus()) << ")" << endl;


   make3D((int)DicomImages[0]->getWidth(),(int)DicomImages[0]->getHeight());

    //creating scene
    myScene= new QGraphicsScene(this);
    ui->graphicsView->setScene(myScene);
   // QFrame *Frame = new QFrame(this);
    //Frame->setFrameShape(QFrame::HLine);
    //Frame->setFrameShadow(QFrame::Sunken);
    //Frame->setLineWidth(1);
    ui->graphicsView->setFrameRect(QRect());

    myScene->addPixmap( QPixmap::fromImage( *Images[0] ) );

}

void MainWindow::make3D(int width, int height){

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

}


void MainWindow::createButtons(){
    ui->mainToolBar->setIconSize(QSize(33,33));
    ui->AdvancedSettings->setIconSize(QSize(33,33));

    QAction *Zoom= new QAction(QIcon("C:/Users/simms/Desktop/Laura/img/zoom.png"),"Zoom",this);
    ui->mainToolBar->addAction(Zoom);

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


    ui->widget_2->setLayout(ui->verticalLayout_2);






}




void MainWindow::on_Hide_clicked()
{
    ui->AdvancedSettings->setVisible(false);
}

void MainWindow::on_InvertGray_clicked()
{
    if(invert ==0){
        for(int j=0; j<Images.size(); j++){
//            for( int i = 0; i < 256; ++i )
//                Images[j]->setColor(255-i, qRgb(i,i,i));
            Images[j]->invertPixels();
    }
    invert=1;
    }else {
        for(int j=0; j<Images.size(); j++){
//            for( int i = 0; i < 256; ++i )
//                Images[j]->setColor(i, qRgb(i,i,i));
            Images[j]->invertPixels();
        }
        invert=0;
    }

    myScene->addPixmap( QPixmap::fromImage( *Images[index] ) );
    ui->graphicsView->setScene(myScene);
}

void MainWindow::on_Advanced_clicked()
{
      ui->AdvancedSettings->setVisible(true);
}

void MainWindow::on_w1_clicked()
{
    ui->graphicsView_2->setVisible(false);
    ui->graphicsView_3->setVisible(false);
    ui->graphicsView_4->setVisible(false);
}

void MainWindow::on_w2_clicked()
{
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
