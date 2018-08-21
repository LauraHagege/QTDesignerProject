#include "serie.h"

#include <string>
#include <QPainter>



Serie::Serie(int id, char *studyname, char *serieref, Plan plan, char *absolutePath , vector<string> &paths, int nbframes, double rescale, char *description, int ww, int wc)
{
    //cout << " creating serie " << id <<" :" << endl;
    cout << description << endl;
    //cout << "nb frames " << nbframes <<endl;

    defaultPlan=plan;
    built=false;
    MultiPlan=false;

    imgPaths = paths;

    hasFlagImages=false;
    flagIndex=0;
    nbFlag=0;

    strcpy(studyName,studyname);
    strcpy(serieRef,serieref);

    serieId=id;
    WW=ww;
    WC=wc;

    viewLinked = false;

    rescaleXFactor=1;
    rescaleYFactor=1;
    rescaleZFactor=1;
    switch(plan){
    case Axial:
        rescaleZFactor=rescale;
        break;
    case Coronal:
        rescaleYFactor=rescale;
        break;
    case Sagittal:
        rescaleXFactor=rescale;
        break;

    }


    nbFrames=nbframes;

    if(nbFrames !=0)
        nbImages=nbFrames;
    else
        nbImages=(int)paths.size();

    //imgIndex=(int)nbImages/2;

    strcpy(serieName,string("Series").c_str());
    strcat(serieName,to_string(serieId).c_str());

    strcpy(serieDesc, description);
    strcat(serieDesc, string("<br>").c_str());
    strcat(serieDesc, string ("Images: ").c_str());
    strcat(serieDesc,  to_string(nbImages).c_str());

    pixelXdepth=0;
    pixelYdepth=0;
    pixelZdepth=0;

    setFlags(absolutePath);


   // cout << "default plan " << defaultPlan << endl;
}

QPixmap Serie::getCurrentImg(Plan currentPlan){
//    cout << serieName << endl;
//    cout << "Xdepth " << pixelXdepth << endl;
//    cout << "Ydepth " << pixelYdepth << endl;
//    cout << "Zdepth " << pixelZdepth << endl;

    cout <<"get current img from plan " << currentPlan <<  endl;

    QImage Image;

    switch(currentPlan){
    case Axial:
        switch(defaultPlan){
        case Coronal:
            Image = QImage (myPixelsZ[imgYIndex],pixelXdepth,pixelYdepth, QImage::Format_Indexed8).scaled(QSize(pixelXdepth,rescaleYFactor*pixelYdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
            break;
        case Axial:
            Image= QImage (myPixelsZ[imgZIndex],pixelXdepth,pixelYdepth, QImage::Format_Indexed8);
            break;
        case Sagittal:
            Image= (QImage (myPixelsZ[imgZIndex],pixelXdepth,pixelYdepth, QImage::Format_Indexed8)).scaled(QSize(rescaleXFactor*pixelXdepth,pixelYdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
            break;
        }
        break;

    case Coronal:
        switch(defaultPlan){
        case Coronal:
            Image= QImage (myPixelsY[imgYIndex],pixelXdepth,pixelZdepth, QImage::Format_Indexed8);
            break;
        case Axial:
            Image= (QImage (myPixelsY[imgYIndex],pixelXdepth,pixelZdepth, QImage::Format_Indexed8)).scaled(QSize(pixelXdepth,rescaleZFactor*pixelZdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
            break;
        case Sagittal:
            Image= (QImage (myPixelsY[imgYIndex],pixelXdepth,pixelZdepth, QImage::Format_Indexed8)).scaled(QSize(rescaleXFactor*pixelXdepth,pixelZdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
            break;
        }
        break;

    case Sagittal:
        switch(defaultPlan){
        case Coronal:
            Image= (QImage (myPixelsX[imgXIndex],pixelYdepth,pixelZdepth, QImage::Format_Indexed8)).scaled(QSize(rescaleYFactor*pixelYdepth,pixelZdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
            break;
        case Axial:
            Image= (QImage (myPixelsX[imgXIndex],pixelYdepth,pixelZdepth, QImage::Format_Indexed8)).scaled(QSize(pixelYdepth,rescaleZFactor*pixelZdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
            break;
        case Sagittal:
           Image= QImage (myPixelsX[imgXIndex],pixelYdepth,pixelZdepth, QImage::Format_Indexed8);
           break;
        }
        break;
    //default is unkown plan
    default:
        Image= QImage (myPixelsZ[imgZIndex],pixelXdepth,pixelYdepth, QImage::Format_Indexed8);
        break;
    }

    return QPixmap::fromImage(Image);

}

string Serie::getPath(int pathNb){
    if(pathNb < nbImages)
        return imgPaths[pathNb];
    else
        return imgPaths[0];

}

bool Serie::isBuilt(){
    return built;
}

char* Serie::getName(){
    return serieName;
}

char * Serie::getDescription(){
    return serieDesc;
}

Plan Serie::getdefaultPlan(){
    return defaultPlan;
}

int Serie::getId(){
    return serieId;
}

int Serie::getNbImages(){
    return nbImages;
}

int Serie::getNbFrames(){
    return nbFrames;
}

int Serie::getXIndex(){
    return imgXIndex;
}

int Serie::getYIndex(){
    return imgYIndex;
}

int Serie::getZIndex(){
    return imgZIndex;
}


int Serie::getWW(){
    return WW;
}

int Serie::getWC(){
    return WC;
}

int Serie::getXdepth(){
    return pixelXdepth;
}

int Serie::getYdepth(){
    return pixelYdepth;
}

int Serie::getZdepth(){
    return pixelZdepth;
}


void Serie::setDepths(int width, int height){
    built=true;
    switch(defaultPlan){
    case Axial:
    case Unknown:
        setXdepth(width);
        setYdepth(height);
        //if currentplan is Axial, while creating other plan, pixelZdepth will need to be rescales to rescaleFatcor*pixelZdepth
        //  nbImage is not properly the depth, but this is the value that will be used to recreate the other plans
        setZdepth(nbImages);

        break;
    case Coronal:
        setXdepth(width);
        //if currentplan is Coronal, while creating other plan, pixelYdepth will need to be rescales to rescaleFatcor*pixelYdepth
        setYdepth(nbImages);
        setZdepth(height);

        break;
    case Sagittal:
        //if currentplan is Sagittal, while creating other plan, pixelXdepth will need to be rescales to rescaleFatcor*pixelXdepth
        setXdepth(nbImages);
        setYdepth(width);
        setZdepth(height);

        break;
    default:
        break;

    }

//    cout << "Xdepth " << pixelXdepth << endl;
//    cout << "Ydepth " << pixelYdepth << endl;
//    cout << "Zdepth " << pixelZdepth << endl;

}

void Serie::setXdepth(int depth){
    pixelXdepth=depth;
}

void Serie::setYdepth(int depth){
    pixelYdepth=depth;
}

void Serie::setZdepth(int depth){
    pixelZdepth=depth;
}

void Serie::storePixel(uint8_t *pixelData){
    //cout << "storePixels" << endl;
    switch(defaultPlan){
    case Axial:
    case Unknown:
        myPixelsZ.push_back(pixelData);
        imgZIndex=(int)myPixelsZ.size()/2;
        break;
    case Sagittal:
        myPixelsX.push_back(pixelData);
        imgXIndex=(int)myPixelsX.size()/2;
        break;
    case Coronal:
        myPixelsY.push_back(pixelData);
        imgYIndex=(int)myPixelsY.size()/2;
        break;

    }

}

void Serie::clearPixels(){
    myPixelsX.clear();
    myPixelsY.clear();
    myPixelsZ.clear();

}
void Serie::constructPlans(){
    cout << "construc plan " << endl;
    MultiPlan=true;
    switch(defaultPlan){
    case Axial:
        constructCoronalPlan();
        constructSagittalPlan();
        break;
    case Coronal:
        constructAxialPlan();
        constructSagittalPlan();
        break;
    case Sagittal:
        constructAxialPlan();
        constructCoronalPlan();
        break;
    default:
        break;
    }

}

void Serie::constructAxialPlan(){
    if(defaultPlan == Sagittal){
        //cout << "axial from sagittal" << endl;
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
        }
    }
    else if(defaultPlan == Coronal){
        //cout << "axial from coronal" << endl;
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
        }
    }
    imgZIndex=(int)myPixelsZ.size()/2;
}

void Serie::constructCoronalPlan(){
    if(defaultPlan == Axial){
        //cout << "coronal from axial" << endl;
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
        }

    }else if(defaultPlan == Sagittal){
        //cout << "coronal from sagittal" << endl;
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
        }
    }
    imgYIndex=(int)myPixelsY.size()/2;

}

void Serie::constructSagittalPlan(){
    if(defaultPlan == Axial){
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
        }


    }
    else if(defaultPlan == Coronal){
        //cout << "sagittal from coronal" << endl;
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
        }
    }
    imgXIndex=(int)myPixelsX.size()/2;
}

void Serie::setNextIndex(Plan plan){
    switch(plan){
    case Axial:
    case Unknown:
        imgZIndex+=1;
        if(imgZIndex+1 > pixelZdepth)
            imgZIndex=0;
        break;
    case Sagittal:
        imgXIndex+=1;
        if(imgXIndex+1 > pixelXdepth)
            imgXIndex=0;
        break;
    case Coronal:
        imgYIndex+=1;
        if(imgYIndex+1 > pixelYdepth)
            imgYIndex=0;
        break;
    case FlagImg:
        flagIndex+=1;
        if(flagIndex+1 > nbFlag)
            flagIndex=0;
        break;
    }

}

void Serie::setPreviousIndex(Plan plan){
    switch(plan){
    case Axial:
    case Unknown:
        imgZIndex-=1;
        if(imgZIndex <0)
            imgZIndex=pixelZdepth-1;
        break;
    case Sagittal:
        imgXIndex-=1;
        if(imgXIndex<0)
            imgXIndex=pixelXdepth-1;
        break;
    case Coronal:
        imgYIndex-=1;
        if(imgYIndex<0)
            imgYIndex=pixelYdepth-1;
        break;
    case FlagImg:
        flagIndex-=1;
        if(flagIndex<0)
            flagIndex=nbFlag-1;
        break;


    }

}


void Serie::setViewLinked(){
    if(MultiPlan){
        if(viewLinked)
            viewLinked=false;
        else
            viewLinked=true;
    }

}

//might be useless
bool Serie::getViewLinked(){
    return viewLinked;
}

void Serie::setPlanWindows(int windowSerieNb[4], Plan windowCurrentPlan[4]){
    axialWindow.clear();
    coronalWindow.clear();
    sagittalWindow.clear();

    axialWindow.push_back(-1);
    sagittalWindow.push_back(-1);
    coronalWindow.push_back(-1);

    for(int i =0; i<4; i++){
        if(windowSerieNb[i]==serieId){
            switch(windowCurrentPlan[i]){
            case Axial:
                if(axialWindow[0]==-1)
                    axialWindow[0]=i+1;
                else
                    axialWindow.push_back(i+1);
                break;
            case Sagittal:
                if(sagittalWindow[0]==-1)
                    sagittalWindow[0]=i+1;
                else
                    sagittalWindow.push_back(i+1);
                break;
            case Coronal:
                if(coronalWindow[0]==-1)
                    coronalWindow[0]=i+1;
                else
                    coronalWindow.push_back(i+1);
                break;
            default:
                break;
            }
        }
    }
//    cout << "Axialwindow size " << axialWindow.size() << endl;
//    for(int i=0; i<axialWindow.size(); i++)
//        cout << " axial window " << axialWindow[i] << endl;
//    cout << "coronal " << coronalWindow << endl;
//    cout << "sagtt " << sagittalWindow << endl;

}


vector<int> Serie::getAxialWindow(){
    return axialWindow;
}

vector<int> Serie::getCoronalWindow(){
    return coronalWindow;
}

vector<int> Serie::getSagittalWindow(){
    return sagittalWindow;
}

vector<int> Serie::getwindow(Plan plan){
    switch(plan){
    case Axial:
        return axialWindow;
    case Coronal:
        return coronalWindow;
    case Sagittal:
        return sagittalWindow;
    default:
        return axialWindow;
    }
}

void Serie::setFlags(char * absolutePath){
    cout << "setflags " << endl;

    char flagpath[200];
    int count=1;
    //char nb;


    strcpy(flagpath,absolutePath);
    strcat(flagpath,string("FLAGGED/").c_str());
    strcat(flagpath,studyName);
    strcat(flagpath,"/");
    strcat(flagpath,serieRef);
    strcat(flagpath,"/");
    strcat(flagpath,string("/FLAG1.jpg").c_str());

    int length = strlen(flagpath);

    QPixmap *flag;
    flag = new QPixmap(flagpath);

    cout << "JPPPPP " << flag->data_ptr() << endl;


    while(flag->data_ptr()){
        cout <<flagpath << endl;
        hasFlagImages=true;
        FlagImages.push_back(flag);
        count++;
        char nb=to_string(count)[0];
        flagpath[length-5]=nb;
        flag = new QPixmap(flagpath);
    }

    nbFlag=FlagImages.size();

    cout <<"nb flag " << nbFlag;
}

bool Serie::getMultiplan(){
    return MultiPlan;
}

QPixmap Serie::getFlags(){
    if(hasFlagImages)
        return *FlagImages[flagIndex];
    else
        return NULL;
}

bool Serie::hasFlag(){
    return hasFlagImages;

}
