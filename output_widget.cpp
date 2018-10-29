#include "output_widget.hpp"
#include <QLayout>
#include <QDebug>
#include <QGraphicsTextItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <iostream>

OutputWidget::OutputWidget() {
    scene = new QGraphicsScene;
    view = new QGraphicsView(scene);
    view->show();
    //scene->addLine(0, 0, 0, 100);
    //scene->addLine(0, 0, 0, -100);
    //scene->addLine(0, 0, 100, 0);
    //scene->addLine(0, 0 ,-100, 0);
    auto layout = new QVBoxLayout();
    layout->addWidget(view);
    setLayout(layout);
}

void OutputWidget::recievePlotscript(Expression result) {
    scene->clear();
    m_result = result;
    eval(m_result);
}

void OutputWidget::recieveError(std::string error) {
    scene->clear();
    QGraphicsTextItem * display = new QGraphicsTextItem(QString::fromStdString(error));
    scene->addItem(display);
    display->setPos(QPointF());
}

void OutputWidget::eval(Expression exp) {
    getType(exp);
    if((exp.isHeadNumber() || exp.isHeadComplex() || exp.isHeadString() || exp.isHeadNone() || exp.isHeadSymbol()) && (m_type == None)) {
        printExpression(exp);
    }else if(m_type == Point) {
        printPoint(exp);
    } else if(m_type == Line) {
        printLine(exp);
    } else if(m_type == Text) {
        printText(exp);
    } else if(m_type == List) {
        for(auto e = exp.listConstBegin(); e != exp.listConstEnd(); ++e)
            eval(*e);
    } else if(m_type == Define) {
        //do nothing
        return;
    }
}

void OutputWidget::printExpression(Expression exp) {
    QString txt = makeQExpression(exp);
    QGraphicsTextItem * display = new QGraphicsTextItem(txt);
    scene->addItem(display);
    display->setPos(QPointF());
}

void OutputWidget::printPoint(Expression exp) {
    Expression sizeExp = exp.get_prop(Expression(Atom("size\"")), exp);
    int size = 0;
    if(!sizeExp.isHeadNone()) {
        size = sizeExp.head().asNumber();
        if(size < 0) {
            recieveError("Error: size is invalid number.");
            return;
        }
    }
    QRectF rect(QPointF(), QSize(size, size));
    QPointF loc = makePoint(exp);
    rect.moveCenter(loc);
    QGraphicsEllipseItem *point = new QGraphicsEllipseItem(rect);
    scene->addItem(point);
    point->setBrush(QBrush(Qt::black, Qt::BrushStyle(Qt::SolidPattern)));
}

void OutputWidget::printLine(Expression exp) {
    QPoint start = makePoint(*exp.listConstBegin());
    QPoint end = makePoint(*std::next(exp.listConstBegin()));
    auto *line = new QGraphicsLineItem(QLineF(start, end));
    int thickness = 1;
    Expression thickExp = exp.get_prop(Expression(Atom("thickness\"")), exp);
    if(!thickExp.isHeadNone()) {
        thickness = thickExp.head().asNumber();
        if(thickness < 0) {
            recieveError("Error: thickness is invalid number.");
            return;
        }
    }
    line->setPen(QPen(QBrush(QColor(Qt::black)), thickness));
    scene->addItem(line);
    line->setLine(QLineF(start, end));
}

void OutputWidget::printText(Expression exp) {
     QString txt = makeString(exp);
     auto *display = new QGraphicsTextItem(txt);
     QPointF pos;
     Expression posExp = exp.get_prop(Expression(Atom("position\"")), exp);
     if(!posExp.isHeadNone()) {
         if(posExp.get_prop(Expression(Atom("object-name\"")), posExp).head().asString() != "point") {
             recieveError("Error: positon is not a point.");
             return;
         }
         pos += makePoint(posExp);
     }
     scene->addItem(display);
     display->setPos(pos);
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
    } else if(exp.head().asSymbol() == "lambda") {
        m_type = Define;
    } else {
        if(exp.isHeadList()) {
            m_type = List;
        } else m_type = None;
    }
}

QPoint OutputWidget::makePoint(Expression exp) {
    QPoint result(exp.listConstBegin()->head().asNumber(), std::next(exp.listConstBegin())->head().asNumber());
    return result;
}

QString OutputWidget::makeString(Expression exp) {
    std::ostringstream out;
    out << exp.head();
    std::string output = out.str();
    return(QString::fromStdString(output));
}

QString OutputWidget::makeQExpression(Expression exp) {
    std::ostringstream out;
    out << exp;
    std::string output = out.str();
    return(QString::fromStdString(output));
}

