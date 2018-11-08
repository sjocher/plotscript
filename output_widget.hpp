#ifndef OUTPUT_WIDGET_HPP
#define OUTPUT_WIDGET_HPP

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QFont>
#include "interpreter.hpp"
#include "semantic_error.hpp"

class OutputWidget: public QWidget {
    Q_OBJECT
public:
    OutputWidget();
    QGraphicsScene * scene;
    QGraphicsView * view;
private:
    Expression m_result;
    void eval(Expression exp);
    void printExpression(Expression exp);
    void printPoint(Expression exp);
    void printLine(Expression exp);
    void printText(Expression exp);
    enum Type {Point, Line, Text, List, None, Define, Discrete, Continuos};
    Type m_type;
    void getType(Expression exp);
    QPoint makePoint(Expression exp);
    QString makeString(Expression exp);
    QString makeQExpression(Expression exp);
    void resizeEvent(QResizeEvent* event);
public slots:
    void recievePlotscript(Expression result);
    void recieveError(std::string error);
};
#endif


