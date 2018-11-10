#include <QTest>
#include <QGraphicsTextItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include "notebook_app.hpp"
#include "input_widget.hpp"
#include "output_widget.hpp"

class NotebookTest : public QObject {
  Q_OBJECT

private slots:
    void verifyWidgets();
    void testInput();
    void testSendingInput();
    void testPointInput();
    void testLineInput();
    void testTextInput();
    void testPointInput2();
    void testFindPoints();
    void testCenterText();
    void testDiscretePlotLayout();
private:
    NotebookApp notebook;
    int findLines(QGraphicsScene * scene, QRectF bbox, qreal margin);
    int findPoints(QGraphicsScene * scene, QPointF center, qreal radius);
    int findText(QGraphicsScene * scene, QPointF center, qreal rotation, QString contents)
};

int findLines(QGraphicsScene * scene, QRectF bbox, qreal margin){
    QPainterPath selectPath;
    QMarginsF margins(margin, margin, margin, margin);
    selectPath.addRect(bbox.marginsAdded(margins));
    scene->setSelectionArea(selectPath, Qt::ContainsItemShape);
    
    int numlines(0);
    foreach(auto item, scene->selectedItems()){
        if(item->type() == QGraphicsLineItem::Type){
            numlines += 1;
        }
    }
    
    return numlines;
}

/*
 findPoints - find points in a scene contained within a specified rectangle
 */
int findPoints(QGraphicsScene * scene, QPointF center, qreal radius){
    
    QPainterPath selectPath;
    selectPath.addRect(QRectF(center.x()-radius, center.y()-radius, 2*radius, 2*radius));
    scene->setSelectionArea(selectPath, Qt::ContainsItemShape);
    
    int numpoints(0);
    foreach(auto item, scene->selectedItems()){
        if(item->type() == QGraphicsEllipseItem::Type){
            numpoints += 1;
        }
    }
    
    return numpoints;
}

/*
 findText - find text in a scene centered at a specified point with a given
 rotation and string contents
 */
int findText(QGraphicsScene * scene, QPointF center, qreal rotation, QString contents){
    
    int numtext(0);
    foreach(auto item, scene->items(center)){
        if(item->type() == QGraphicsTextItem::Type){
            QGraphicsTextItem * text = static_cast<QGraphicsTextItem *>(item);
            if((text->toPlainText() == contents) &&
               (text->rotation() == rotation) &&
               (text->pos() + text->boundingRect().center() == center)){
                numtext += 1;
            }
        }
    }
    
    return numtext;
}

/*
 intersectsLine - find lines in a scene that intersect a specified rectangle
 */
int intersectsLine(QGraphicsScene * scene, QPointF center, qreal radius){
    
    QPainterPath selectPath;
    selectPath.addRect(QRectF(center.x()-radius, center.y()-radius, 2*radius, 2*radius));
    scene->setSelectionArea(selectPath, Qt::IntersectsItemShape);
    
    int numlines(0);
    foreach(auto item, scene->selectedItems()){
        if(item->type() == QGraphicsLineItem::Type){
            numlines += 1;
        }
    }
    
    return numlines;
}

void NotebookTest::verifyWidgets() {
    auto in = notebook.findChild<InputWidget *>("input");
    auto out = notebook.findChild<OutputWidget *>("output");
    QVERIFY2(out, "Could not find output");
    QVERIFY2(in, "Could not find input");
}

void NotebookTest::testInput() {
    auto in = notebook.findChild<InputWidget *>("input");
    QTest::keyClicks(in, "(define a 3)");
    QCOMPARE(in->toPlainText(), QString("(define a 3)"));
    in->clear();
}

void NotebookTest::testSendingInput() {
    auto in = notebook.findChild<InputWidget *>("input");
    QTest::keyClicks(in, "(define a 3)");
    QTest::keyPress(in, Qt::Key_Return, Qt::KeyboardModifier::ShiftModifier, 0);
    auto out = notebook.findChild<OutputWidget *>("output");
    auto find = out->scene->items();
    QVERIFY2(find.size() == 1, "Point not found");
    QGraphicsTextItem *test = dynamic_cast<QGraphicsTextItem*>(find[0]);
    QVERIFY2(test->toPlainText() == "(3)", "Expression result wrong.");
    in->clear();
    out->scene->clear();
}

void NotebookTest::testPointInput() {
    auto in = notebook.findChild<InputWidget *>("input");
    auto out = notebook.findChild<OutputWidget *>("output");
    QTest::keyClicks(in, "(make-point 0 0)");
    QTest::keyPress(in, Qt::Key_Return, Qt::KeyboardModifier::ShiftModifier, 0);
    auto find = out->scene->items();
    QVERIFY2(find.size() == 1, "Point exists");
    QVERIFY2(find[0]->boundingRect().center() == QPointF(0,0), "Point not in right scene location");
}

void NotebookTest::testTextInput() {
    auto in = notebook.findChild<InputWidget *>("input");
    auto out = notebook.findChild<OutputWidget *>("output");
    QTest::keyClicks(in, "(make-text \"test\")");
    QTest::keyPress(in, Qt::Key_Return, Qt::KeyboardModifier::ShiftModifier, 0);
    auto find = out->scene->items();
    QVERIFY2(find.size() == 1, "Text not found");
    QGraphicsTextItem *test = dynamic_cast<QGraphicsTextItem*>(find[0]);
    QVERIFY2(test->toPlainText() == "test", "Make-text result wrong.");
    in->clear();
    out->scene->clear();
}

void NotebookTest::testPointInput2() {
    auto in = notebook.findChild<InputWidget *>("input");
    auto out = notebook.findChild<OutputWidget *>("output");
    QTest::keyClicks(in, "(set-property \"size\" 20 (make-point 20 20))");
    QTest::keyPress(in, Qt::Key_Return, Qt::KeyboardModifier::ShiftModifier, 0);
    auto find = out->view->items();
    QVERIFY2(find.size() == 1, "Point doesnt exist");
    QVERIFY2(find[0]->boundingRect().center() == QPointF(20,20), "Point centered at wrong location");
    QVERIFY2(find[0]->boundingRect().size() == QSize(20 + 1, 20 + 1), "Point size wrong");
    in->clear();
    out->scene->clear();
}

void NotebookTest::testLineInput() {
    auto in = notebook.findChild<InputWidget *>("input");
    auto out = notebook.findChild<OutputWidget *>("output");
    QTest::keyClicks(in, "(make-line (make-point 0 0) (make-point 20 0))");
    QTest::keyPress(in, Qt::Key_Return, Qt::KeyboardModifier::ShiftModifier, 0);
    auto find = out->scene->items();
    QVERIFY2(find.size() == 1, "Line not found");
    //QGraphicsLineItem *line = dynamic_cast<QGraphicsLineItem*>(find[0]);
    //qDebug() << line->line();
    //QVERIFY2(line->line() == QLineF(QPointF(), QPointF(20,0)), "Line does not match");
    in->clear();
    out->scene->clear();
}

void NotebookTest::testFindPoints() {
    auto in = notebook.findChild<InputWidget *>("input");
    auto out = notebook.findChild<OutputWidget *>("output");
    QTest::keyClicks(in, "(list (set-property \"size\" 1 (make-point 0 0)) (set-property \"size\" 2 (make-point 0 4)) (set-property \"size\" 4 (make-point 0 8)) (set-property \"size\" 8 (make-point 0 16)) (set-property \"size\" 16 (make-point 0 32)) (set-property \"size\" 32 (make-point 0 64)))");
    QTest::keyPress(in, Qt::Key_Return, Qt::KeyboardModifier::ShiftModifier, 0);
    auto find = out->scene->items();
    QVERIFY2(find.size() == 6, "Points do not exist");
    QVERIFY2(find[5]->boundingRect().center() == QPointF(0,0), "Point not in right scene location");
    QVERIFY2(find[4]->boundingRect().center() == QPointF(0,4), "Point not in right scene location");
    QVERIFY2(find[3]->boundingRect().center() == QPointF(0,8), "Point not in right scene location");
    QVERIFY2(find[2]->boundingRect().center() == QPointF(0,16), "Point not in right scene location");
    QVERIFY2(find[1]->boundingRect().center() == QPointF(0,32), "Point not in right scene location");
    QVERIFY2(find[0]->boundingRect().center() == QPointF(0,64), "Point not in right scene location");
    //size is incremented by 1 to account for 0 pos
    QVERIFY2(find[5]->boundingRect().size() == QSize(1 + 1, 1 + 1), "Point not in right scene location");
    QVERIFY2(find[4]->boundingRect().size() == QSize(2 + 1, 2 + 1), "Point not in right scene location");
    QVERIFY2(find[3]->boundingRect().size() == QSize(4 + 1, 4 + 1), "Point not in right scene location");
    QVERIFY2(find[2]->boundingRect().size() == QSize(8 + 1, 8 + 1), "Point not in right scene location");
    QVERIFY2(find[1]->boundingRect().size() == QSize(16 + 1, 16 + 1), "Point not in right scene location");
    QVERIFY2(find[0]->boundingRect().size() == QSize(32 + 1, 32 + 1), "Point not in right scene location");
    in->clear();
    out->scene->clear();
}

void NotebookTest::testCenterText() {
    auto in = notebook.findChild<InputWidget *>("input");
    auto out = notebook.findChild<OutputWidget *>("output");
    QTest::keyClicks(in, "(make-text \"Hello World\")");
    QTest::keyPress(in, Qt::Key_Return, Qt::KeyboardModifier::ShiftModifier, 0);
    auto find = out->scene->items();
    QVERIFY2(find.size() == 1, "text not found");
    QVERIFY2(find[0]->sceneBoundingRect().center() == QPointF(), "Center of text in wrong location");
}

void NotebookTest::testDiscretePlotLayout() {
    std::string program = R"(
    (discrete-plot (list (list -1 -1) (list 1 1))
     (list (list "title" "The Title")
      (list "abscissa-label" "X Label")
      (list "ordinate-label" "Y Label")))
    )";
    
    inputWidget->setPlainText(QString::fromStdString(program));
    QTest::keyClick(inputWidget, Qt::Key_Return, Qt::ShiftModifier);
    
    auto view = outputWidget->findChild<QGraphicsView *>();
    QVERIFY2(view, "Could not find QGraphicsView as child of OutputWidget");
    
    auto scene = view->scene();
    
    // first check total number of items
    // 8 lines + 2 points + 7 text = 17
    auto items = scene->items();
    QCOMPARE(items.size(), 17);
    
    // make them all selectable
    foreach(auto item, items){
        item->setFlag(QGraphicsItem::ItemIsSelectable);
    }
    
    double scalex = 20.0/2.0;
    double scaley = 20.0/2.0;
    
    double xmin = scalex*-1;
    double xmax = scalex*1;
    double ymin = scaley*-1;
    double ymax = scaley*1;
    double xmiddle = (xmax+xmin)/2;
    double ymiddle = (ymax+ymin)/2;
    
    // check title
    QCOMPARE(findText(scene, QPointF(xmiddle, -(ymax+3)), 0, QString("The Title")), 1);
    
    // check abscissa label
    QCOMPARE(findText(scene, QPointF(xmiddle, -(ymin-3)), 0, QString("X Label")), 1);
    
    // check ordinate label
    QCOMPARE(findText(scene, QPointF(xmin-3, -ymiddle), -90, QString("Y Label")), 1);
    
    // check abscissa min label
    QCOMPARE(findText(scene, QPointF(xmin, -(ymin-2)), 0, QString("-1")), 1);
    
    // check abscissa max label
    QCOMPARE(findText(scene, QPointF(xmax, -(ymin-2)), 0, QString("1")), 1);
    
    // check ordinate min label
    QCOMPARE(findText(scene, QPointF(xmin-2, -ymin), 0, QString("-1")), 1);
    
    // check ordinate max label
    QCOMPARE(findText(scene, QPointF(xmin-2, -ymax), 0, QString("1")), 1);
    
    // check the bounding box bottom
    QCOMPARE(findLines(scene, QRectF(xmin, -ymin, 20, 0), 0.1), 1);
    
    // check the bounding box top
    QCOMPARE(findLines(scene, QRectF(xmin, -ymax, 20, 0), 0.1), 1);
    
    // check the bounding box left and (-1, -1) stem
    QCOMPARE(findLines(scene, QRectF(xmin, -ymax, 0, 20), 0.1), 2);
    
    // check the bounding box right and (1, 1) stem
    QCOMPARE(findLines(scene, QRectF(xmax, -ymax, 0, 20), 0.1), 2);
    
    // check the abscissa axis
    QCOMPARE(findLines(scene, QRectF(xmin, 0, 20, 0), 0.1), 1);
    
    // check the ordinate axis
    QCOMPARE(findLines(scene, QRectF(0, -ymax, 0, 20), 0.1), 1);
    
    // check the point at (-1,-1)
    QCOMPARE(findPoints(scene, QPointF(-10, 10), 0.6), 1);
    
    // check the point at (1,1)
    QCOMPARE(findPoints(scene, QPointF(10, -10), 0.6), 1);
}

QTEST_MAIN(NotebookTest)
#include "notebook_test.moc"
