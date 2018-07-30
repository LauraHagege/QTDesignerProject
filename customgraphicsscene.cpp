#include "customgraphicsscene.h"

#include <iostream>

using namespace std;

CustomGraphicsScene::CustomGraphicsScene(QObject *parent): QGraphicsScene(*parent){

}


void mousePressEvent(QGraphicsSceneMouseEvent *event ){

    cout << "mouseevent" << endl;
}
