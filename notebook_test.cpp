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
    void testTextInput2();
    void testFindPoints();
private:
    NotebookApp notebook;
};

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

void NotebookTest::testTextInput2() {
    auto in = notebook.findChild<InputWidget *>("input");
    auto out = notebook.findChild<OutputWidget *>("output");
    QTest::keyClicks(in, "(set-property \"position\" (make-point 20 20) (make-text \"ree\"))");
    QTest::keyPress(in, Qt::Key_Return, Qt::KeyboardModifier::ShiftModifier, 0);
    auto find = out->scene->items();
    QVERIFY2(find.size() == 1, "Text not found");
    QGraphicsTextItem *test = dynamic_cast<QGraphicsTextItem*>(find[0]);
    QVERIFY2(test->toPlainText() == "ree", "Make-text result wrong.");
    QVERIFY2(test->scenePos() == QPointF(20,20), "text is in wrong location");
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

QTEST_MAIN(NotebookTest)
#include "notebook_test.moc"
