#pragma once
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QPainter>
#include <QTextBlock>
#include <QResizeEvent>
#include <QLabel>

class LineNumberArea;

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    QWidget *lineNumberArea;
};

class LineNumberArea : public QWidget {
public:
    explicit LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor) {}

    QSize sizeHint() const override {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *codeEditor;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void openFileFromPath(const QString &path);

private slots:
    void newFile();
    void openFile();
    void saveFile();
    void saveFileAs();
    void showHelp();

private:
    void createMenus();
    void updateTitle();

    CodeEditor *editor;
    QLabel *cursorLabel;
    QString currentFile;
    bool modified = false;
};
