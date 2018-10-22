#include "output_widget.hpp"
#include <QLayout>
#include <QGraphicsTextItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <iostream>

OutputWidget::OutputWidget(QWidget * parent) {
    setParent(parent);
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
    output->setPos(QPoint(0,0));
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
    QGraphicsTextItem *display = new QGraphicsTextItem(QString::fromStdString(output));
    display->setPos(QPoint(0,0));
    scene->addItem(display);
}

void OutputWidget::printPoint(Expression exp) {
    int x = (int)exp.listConstBegin()->head().asNumber();
    int y = (int)std::next(exp.listConstBegin())->head().asNumber();
    QPoint pos(x,y);
    QRect circle;
    QGraphicsEllipseItem *point = new QGraphicsEllipseItem(QRect(QPoint(), QSize(0, 0)));
    Expression sizeExp = exp.get_prop(Expression(Atom("size\"")), exp);
    if(!sizeExp.isHeadNone()) {
        int size = sizeExp.head().asNumber();
        if(size < 0)
            recieveError("Error: size is invalid number.");
        circle.setRect(0, 0, size, size);
        circle.moveCenter(pos);
        point->setRect(circle);
        point->setBrush(QBrush(Qt::black, Qt::BrushStyle(Qt::SolidPattern)));
    }
    scene->addItem(point);
}

void OutputWidget::printLine(Expression exp) {
    //cant access m_list directly, must use listConstBegin and listConstEnd
    if(exp.listSize() != 2)
        recieveError("Error: wrong number of arguments in print line");
    //get points as expressions
    Expression p1 = *exp.listConstBegin();
    Expression p2 = *std::next(exp.listConstBegin());
    if(!p1.isHeadList() || !p2.isHeadList() || (((p1.listSize() != 2) || (p2.listSize() != 2))))
        recieveError("Error: arguments to make-line are not points");
    //convert points to QPOINTS
    QPoint start((int)p1.listConstBegin()->head().asNumber(), (int)std::next(p1.listConstBegin())->head().asNumber());
    QPoint end((int)p2.listConstBegin()->head().asNumber(), (int)std::next(p2.listConstBegin())->head().asNumber());
    //setup line in QT
    QGraphicsLineItem *line = new QGraphicsLineItem(QLineF(start, end));
    Expression thicknessExp = exp.get_prop(Expression(Atom("thickness\"")), exp);
    if(!thicknessExp.isHeadNone()) {
        line->setPen(QPen(QBrush(QColor(Qt::black)), (int)thicknessExp.head().asNumber()));
    }
    scene->addItem(line);
}

void OutputWidget::printText(Expression exp) {
    std::ostringstream out;
    out << exp.head();
    std::string output = out.str();
    QGraphicsTextItem *display = new QGraphicsTextItem(QString::fromStdString(output));
    display->setPos(QPoint(0,0));
    Expression positionExp = exp.get_prop(Expression(Atom("position\"")), exp);
    if(!positionExp.isHeadNone()) {
        display->setPos(QPointF((int)positionExp.listConstBegin()->head().asNumber(), (int)std::next(positionExp.listConstBegin())->head().asNumber()));
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
