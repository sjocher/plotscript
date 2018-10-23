#include "output_widget.hpp"
#include <QLayout>
#include <QDebug>
#include <QGraphicsTextItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <iostream>

OutputWidget::OutputWidget(QWidget * parent): QWidget(parent) {
    scene = new QGraphicsScene;
    view = new QGraphicsView(scene);
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
    output->setPos(QPointF());
    scene->addItem(output);
}

void OutputWidget::eval(Expression exp) {
    getType(exp);
    if((exp.isHeadNumber() || exp.isHeadComplex() || exp.isHeadString() || exp.isHeadNone() || exp.isHeadSymbol()) && (m_type == None)) {
        printExpression(exp);
    } else if(exp.isHeadLambda()) {
        return;
    }else if(m_type == Point) {
        printPoint(exp);
    } else if(m_type == Line) {
        printLine(exp);
    } else if(m_type == Text) {
        printText(exp);
    } else if(m_type == List) {
        for(auto e = exp.listConstBegin(); e != exp.listConstEnd(); ++e)
            eval(*e);
    }
}

void OutputWidget::printExpression(Expression exp) {
    std::ostringstream out;
    out << exp;
    std::string output = out.str();
    auto *display = new QGraphicsTextItem(QString::fromStdString(output));
    display->boundingRect().moveCenter(QPointF());
    scene->addItem(display);
}

void OutputWidget::printPoint(Expression exp) {
    int size = exp.get_prop(Expression(Atom("size\"")), exp).head().asNumber();
    if(size < 0)
        recieveError("Error: size is invalid number.");
    QPoint loc = makePoint(exp);
    QRect circle(0,0,size,size);
    circle.moveCenter(loc);
    auto *point = new QGraphicsEllipseItem(circle);
    point->setX(loc.rx());
    point->setY(loc.ry());
    point->setBrush(QBrush(Qt::black, Qt::BrushStyle(Qt::SolidPattern)));
    scene->addItem(point);
}

void OutputWidget::printLine(Expression exp) {
    QPoint start = makePoint(*exp.listConstBegin());
    QPoint end = makePoint(*std::next(exp.listConstBegin()));
    auto *line = new QGraphicsLineItem(QLineF(start, end));
    int thickness = exp.get_prop(Expression(Atom("thickness\"")), exp).head().asNumber();
    if(thickness < 0)
        recieveError("Error: thickness is invalid number.");
    line->setPen(QPen(QBrush(QColor(Qt::black)), thickness));
    scene->addItem(line);
}

void OutputWidget::printText(Expression exp) {
    QString txt = makeString(exp);
    auto *display = new QGraphicsTextItem(txt);
    display->boundingRect().moveCenter(QPointF());
    Expression positionExp = exp.get_prop(Expression(Atom("position\"")), exp);
    if(!positionExp.isHeadNone()) {
        std::string objname = exp.get_prop(Expression(Atom("object-name\"")), exp).head().asString();
        if(objname != "point")
            recieveError("Error: position is not a point.");
        QPoint pnt = makePoint(positionExp);
        display->boundingRect().moveCenter(pnt);
    }
    scene->addItem(display);
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
    } else {
        if(exp.isHeadList()) {
            m_type = List;
        } else m_type = None;
    }
}

QPoint OutputWidget::makePoint(Expression exp) {
    QPoint result;
    result.setX(exp.listConstBegin()->head().asNumber());
    result.setY(std::next(exp.listConstBegin())->head().asNumber());
    return result;
}

QString OutputWidget::makeString(Expression exp) {
    std::ostringstream out;
    out << exp.head();
    std::string output = out.str();
    return(QString::fromStdString(output));
}
