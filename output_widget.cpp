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
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto layout = new QVBoxLayout();
    layout->addWidget(view);
    setLayout(layout);
}

void OutputWidget::recievePlotscript(Expression result) {
    scene->clear();
    m_result = result;
    eval(m_result);
    view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

void OutputWidget::recieveError(std::string error) {
    scene->clear();
    QGraphicsTextItem * display = new QGraphicsTextItem(QString::fromStdString(error));
    scene->addItem(display);
    display->setPos(QPointF());
    view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
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
    double size = 0;
    if(!sizeExp.isHeadNone()) {
        size = sizeExp.head().asNumber();
        if(size < 0) {
            recieveError("Error: size is invalid number.");
            return;
        }
    }
    QRectF rect(QPointF(), QSizeF(size, size));
    QPointF loc = makePoint(exp);
    rect.moveCenter(loc);
    QGraphicsEllipseItem *point = new QGraphicsEllipseItem(rect);
    scene->addItem(point);
    point->setPen(QPen(Qt::PenStyle(Qt::NoBrush)));
    point->setBrush(QBrush(Qt::BrushStyle(Qt::SolidPattern)));
}

void OutputWidget::printLine(Expression exp) {
    QPointF start = makePoint(*exp.listConstBegin());
    QPointF end = makePoint(*std::next(exp.listConstBegin()));
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
    auto font = QFont("Courier");
    font.setStyleHint(QFont::TypeWriter);
    font.setPointSize(1);
    //position
    scene->addItem(display);
    QPointF pos;
    Expression posExp = exp.get_prop(Expression(Atom("position\"")), exp);
    if(!posExp.isHeadNone()) {
         if(posExp.get_prop(Expression(Atom("object-name\"")), posExp).head().asString() != "point") {
             recieveError("Error: positon is not a point.");
             return;
         }
         pos = makePoint(posExp);
     }
    //scale
    Expression scaleExp = exp.get_prop(Expression(Atom("scale\"")), exp);
    if(!scaleExp.isHeadNone()) {
        int scale = scaleExp.head().asNumber();
        if(scale < 0) {
            recieveError("Error: scale is invalid");
            return;
        }
        display->setScale(scale);
    }
    display->setFont(font);
    display->setPos(pos);
    //center the position of the text
    double xoffset = -((display->boundingRect().width()) / 2);
    double yoffset = -((display->boundingRect().height()) / 2);
    display->moveBy(xoffset, yoffset);
    //rotation
    Expression rotExp = exp.get_prop(Expression(Atom("rotation\"")), exp);
    if(!rotExp.isHeadNone()) {
        display->setTransformOriginPoint(display->boundingRect().center());
        double rot = rotExp.head().asNumber();
        rot = (rot * 180) / M_PI;
        display->setRotation(rot);
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
    } else if(exp.head().asSymbol() == "lambda") {
        m_type = Define;
    } else {
        if(exp.isHeadList()) {
            m_type = List;
        } else m_type = None;
    }}

QPointF OutputWidget::makePoint(Expression exp) {
    QPointF result(exp.listConstBegin()->head().asNumber(), std::next(exp.listConstBegin())->head().asNumber());
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

void OutputWidget::resizeEvent(QResizeEvent* event) {
    QResizeEvent * p = event;
    if(p == event) {
        view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    }
}
