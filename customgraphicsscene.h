#ifndef CUSTOMGRAPHICSSCENE_H
#define CUSTOMGRAPHICSSCENE_H

#include <QGraphicsScene>

class CustomGraphicsScene: public QGraphicsScene
{
    Q_OBJECT

public:
    CustomGraphicsScene(QObject * parent=0);

    void mousePressEvent(QGraphicsSceneMouseEvent *event );
};

#endif // CUSTOMGRAPHICSSCENE_H
