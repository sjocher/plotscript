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
    auto find = out->scene->itemAt(QPointF(), QTransform());
    QVERIFY2(find, "Could not find expression.");
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
    QVERIFY2(find[0]->scenePos() == QPointF(0,0), "Point not in right scene location");
}

void NotebookTest::testTextInput() {
    auto in = notebook.findChild<InputWidget *>("input");
    auto out = notebook.findChild<OutputWidget *>("output");
    QTest::keyClicks(in, "(make-text \"test\")");
    QTest::keyPress(in, Qt::Key_Return, Qt::KeyboardModifier::ShiftModifier, 0);
    auto find = out->scene->itemAt(QPointF(), QTransform());
    QVERIFY2(find, "Could not find expression.");
    in->clear();
    out->scene->clear();
}

void NotebookTest::testPointInput2() {
    auto in = notebook.findChild<InputWidget *>("input");
    auto out = notebook.findChild<OutputWidget *>("output");
    QTest::keyClicks(in, "(set-property \"size\" 20 (make-point 20 20))");
    QTest::keyPress(in, Qt::Key_Return, Qt::KeyboardModifier::ShiftModifier, 0);
    auto find = out->view->items();
    QVERIFY2(find.size() == 1, "Point exists");
    in->clear();
    out->scene->clear();
}

void NotebookTest::testLineInput() {
    auto in = notebook.findChild<InputWidget *>("input");
    auto out = notebook.findChild<OutputWidget *>("output");
    QTest::keyClicks(in, "(set-property \"thickness\" 4 (make-line (make-point 0 0) (make-point 20 20)))");
    QTest::keyPress(in, Qt::Key_Return, Qt::KeyboardModifier::ShiftModifier, 0);
    auto find = out->scene->items();
    QVERIFY2(find.size() == 1, "Line exists");
    QVERIFY2(find[0]->scenePos() == QPointF(), "Line starts at origin.");
    in->clear();
    out->scene->clear();
}

void NotebookTest::testTextInput2() {
    auto in = notebook.findChild<InputWidget *>("input");
    auto out = notebook.findChild<OutputWidget *>("output");
    QTest::keyClicks(in, "(set-property \"position\" (make-point 20 20) (make-text \"ree\"))");
    QTest::keyPress(in, Qt::Key_Return, Qt::KeyboardModifier::ShiftModifier, 0);
    auto find = out->scene->items();
    QVERIFY2(find.size() == 1, "Could not find expression.");
    QVERIFY2(find[0]->scenePos() == QPointF(20,20), "Could not find expression.");
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
    QVERIFY2(find[5]->pos() == QPointF(0,0), "Point not in right scene location");
    in->clear();
    out->scene->clear();
}

QTEST_MAIN(NotebookTest)
#include "notebook_test.moc"
