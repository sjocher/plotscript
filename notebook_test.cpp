#include <QTest>
#include "notebook_app.hpp"
#include "input_widget.hpp"
#include "output_widget.hpp"
class NotebookTest : public QObject {
  Q_OBJECT

private slots:
    void initTestCase();
    void findOutput();
    void testInput();
private:
    NotebookApp notebook;
};

void NotebookTest::initTestCase(){
    auto in = notebook.findChild<InputWidget *>("input");
    QVERIFY2(in, "Could not find input");
}

void NotebookTest::findOutput() {
    auto out = notebook.findChild<OutputWidget *>("output");
    QVERIFY2(out, "Could not find output");
}

void NotebookTest::testInput() {
    notebook.repl(QString::fromStdString("(define a 3)"));
}

QTEST_MAIN(NotebookTest)
#include "notebook_test.moc"
