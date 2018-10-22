#include "output_widget.hpp"
#include <Qlayout>
#include <QGraphicsTextItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <iostream>

OutputWidget::OutputWidget(QWidget * parent) {
    scene = new QGraphicsScene;
    view = new QGraphicsView(scene);
    //view->setFixedSize(250, 250);
    view->show();
    auto layout = new QGridLayout();
    layout->addWidget(view, 0 , 0);
    setLayout(layout);
}

void OutputWidget::recievePlotscript(Expression result) {
    m_display = result;
    scene->clear();
    eval(result);
}

void OutputWidget::recieveError(std::string error) {
    m_error = error;
    scene->clear();
    QGraphicsTextItem *output = new QGraphicsTextItem((QString::fromStdString(error)));
    output->setPos(QPoint(0,0));
    scene->addItem(output);
}

void OutputWidget::eval(Expression exp) {
    getType(exp);
    if((exp.isHeadNumber() || exp.isHeadComplex() || exp.isHeadString() || exp.isHeadNone() || exp.isHeadSymbol()) && (m_type == None)) {
        printExpression(exp);
    } else if(exp.isHeadLambda()) {
        //do nothing for now
        return;
    }else if(m_type == Point) {
        printPoint(exp);
    } else if(m_type == Line) {
        printLine(exp);
    } else if(m_type == Text) {
        printText(exp);
    } else if(exp.isHeadList()) {
        for(auto e = exp.listConstBegin(); e != exp.listConstEnd(); ++e)
            eval(*e);
    }
}

void OutputWidget::printExpression(Expression exp) {
    std::ostringstream out;
    out << exp;
    std::string output = out.str();
    QGraphicsTextItem *display = new QGraphicsTextItem(QString::fromStdString(output));
    display->setPos(QPoint(0,0));
    scene->addItem(display);
}

void OutputWidget::printPoint(Expression exp) {
    QPoint pos((int)exp.listConstBegin()->head().asNumber(), (int)exp.listConstEnd()->head().asNumber());
    QGraphicsEllipseItem *point = new QGraphicsEllipseItem(QRect(pos, QSize(10, 10)));
    
    Expression sizeExp = exp.get_prop(Expression(Atom("size\"")), exp);
    if(!sizeExp.isHeadNone()) {
        point->setRect(QRect(pos, QSize((int)sizeExp.head().asNumber(), (int)sizeExp.head().asNumber())));
    }
    scene->addItem(point);
}

void OutputWidget::printLine(Expression exp) {
    Expression thicknessExp = exp.get_prop(Expression(Atom("thickness\"")), exp);
    if(!thicknessExp.isHeadNone()) {
        
    }
}

void OutputWidget::printText(Expression exp) {
    Expression positionExp = exp.get_prop(Expression(Atom("position\"")), exp);
    if(!positionExp.isHeadNone()) {
        
    }
}

void OutputWidget::getType(Expression exp) {
    Expression prop = exp.get_prop(Expression(Atom("object-name\"")), exp);
    std::string objname = prop.head().asString();
    if(objname == "point") {
        m_type = Point;
    } else if(objname == "line") {
        m_type = Line;
    } else if(objname == "text") {
        m_type = Text;
    } else m_type = None;
}
