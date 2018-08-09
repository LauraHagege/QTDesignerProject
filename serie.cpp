#include "serie.h"

#include <string>



Serie::Serie(int id, Plan plan, vector<string> &paths, int nbframes, double rescale, char *description, int ww, int wc)
{
    defaultPlan=plan;

    imgPaths = paths;

    serieId=id;
    WW=ww;
    WC=wc;

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

//    cout << "creatin serie nb " << id << endl;
//    cout << "first img path "<<imgPaths[0] << endl;
//    cout << "serie plan " << defaultPlan <<endl;

}

QImage Serie::getCurrentImg(Plan currentPlan){
//    cout << serieName << endl;
//    cout << "Xdepth " << pixelXdepth << endl;
//    cout << "Ydepth " << pixelYdepth << endl;
//    cout << "Zdepth " << pixelZdepth << endl;

   cout << "current window Plan" << currentPlan << endl;
   cout << "defaultPlan" << defaultPlan << endl;

    switch(currentPlan){
    case Axial:
        switch(defaultPlan){
        case Coronal:
            return QImage (myPixelsZ[imgYIndex],pixelXdepth,pixelYdepth, QImage::Format_Indexed8).scaled(QSize(pixelXdepth,rescaleYFactor*pixelYdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        case Axial:
            return QImage (myPixelsZ[imgZIndex],pixelXdepth,pixelYdepth, QImage::Format_Indexed8);
        case Sagittal:
            return (QImage (myPixelsZ[imgZIndex],pixelXdepth,pixelYdepth, QImage::Format_Indexed8)).scaled(QSize(rescaleXFactor*pixelXdepth,pixelYdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        }

    case Coronal:
        switch(defaultPlan){
        case Coronal:
            return QImage (myPixelsY[imgYIndex],pixelXdepth,pixelZdepth, QImage::Format_Indexed8);
        case Axial:
            cout << "coronal from axial !" << endl;
            return (QImage (myPixelsY[imgYIndex],pixelXdepth,pixelZdepth, QImage::Format_Indexed8)).scaled(QSize(pixelXdepth,rescaleZFactor*pixelZdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        case Sagittal:
            return (QImage (myPixelsY[imgYIndex],pixelXdepth,pixelZdepth, QImage::Format_Indexed8)).scaled(QSize(rescaleXFactor*pixelXdepth,pixelZdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        }

    case Sagittal:
        switch(defaultPlan){
        case Coronal:
            return (QImage (myPixelsX[imgXIndex],pixelYdepth,pixelZdepth, QImage::Format_Indexed8)).scaled(QSize(rescaleYFactor*pixelYdepth,pixelZdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        case Axial:
            return (QImage (myPixelsX[imgXIndex],pixelYdepth,pixelZdepth, QImage::Format_Indexed8)).scaled(QSize(pixelYdepth,rescaleZFactor*pixelZdepth), Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
        case Sagittal:
           return QImage (myPixelsX[imgXIndex],pixelYdepth,pixelZdepth, QImage::Format_Indexed8);
        }

    default:
        //find a better default
        return QImage (myPixelsZ[imgZIndex],pixelXdepth,pixelYdepth, QImage::Format_Indexed8);

    }

}

string Serie::getPath(int pathNb){
    if(pathNb < nbImages)
        return imgPaths[pathNb];
    else
        return imgPaths[0];

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
    cout << serieName << endl;
    cout << "setDepth" << endl;
    switch(defaultPlan){
    case Axial:
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
    switch(defaultPlan){
    case Axial:
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


void Serie::constructAxialPlan(){
    if(defaultPlan == Sagittal){
        cout << "axial from sagittal" << endl;
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
        }
    }
    imgZIndex=(int)myPixelsZ.size()/2;
}

void Serie::constructCoronalPlan(){
    if(defaultPlan == Axial){
        cout << "coronal from axial" << endl;
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
        }
    }
    imgXIndex=(int)myPixelsX.size()/2;
}

void Serie::setNextIndex(Plan plan){
    switch(plan){
    case Axial:
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
    }

}

void Serie::setPreviousIndex(Plan plan){
    switch(plan){
    case Axial:
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
    }

}
