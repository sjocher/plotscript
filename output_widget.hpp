#ifndef OUTPUT_WIDGET_HPP
#define OUTPUT_WIDGET_HPP

#include <QWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include "interpreter.hpp"
#include "semantic_error.hpp"

class OutputWidget: public QWidget {
    Q_OBJECT
public:
    OutputWidget(QWidget * parent = nullptr);
    QGraphicsScene * scene;
    QGraphicsView * view;
private:
    Expression m_display;
    std::string m_error;
    void eval(Expression exp);
    void printExpression(Expression exp);
    void printPoint(Expression exp);
    void printLine(Expression exp);
    void printText(Expression exp);
    enum Type {Point, Line, Text, List, None};
    Type m_type;
    void getType(Expression exp);
public slots:
    void recievePlotscript(Expression result);
    void recieveError(std::string error);
};
#endif


