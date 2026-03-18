#include "mainwindow.h"
#include <QApplication>
#include <QDesktopServices>
#include <QDialog>
#include <QFileDialog>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollBar>
#include <QStatusBar>
#include <QTextStream>
#include <QUrl>
#include <QVBoxLayout>
// ─── CodeEditor ────────────────────────────────────────────────────────────

CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent) {
  lineNumberArea = new LineNumberArea(this);

  setLineWrapMode(QPlainTextEdit::NoWrap);

  connect(this, &CodeEditor::blockCountChanged, this,
          &CodeEditor::updateLineNumberAreaWidth);
  connect(this, &CodeEditor::updateRequest, this,
          &CodeEditor::updateLineNumberArea);
  connect(this, &CodeEditor::cursorPositionChanged, this,
          &CodeEditor::highlightCurrentLine);

  updateLineNumberAreaWidth(0);
  highlightCurrentLine();
}

int CodeEditor::lineNumberAreaWidth() {
  int digits = 1;
  int max = qMax(1, blockCount());
  while (max >= 10) {
    max /= 10;
    ++digits;
  }
  return 16 +
         fontMetrics().horizontalAdvance(QLatin1Char('9')) * qMax(digits, 3);
}

void CodeEditor::updateLineNumberAreaWidth(int) {
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy) {
  if (dy)
    lineNumberArea->scroll(0, dy);
  else
    lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

  if (rect.contains(viewport()->rect()))
    updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *e) {
  QPlainTextEdit::resizeEvent(e);
  QRect cr = contentsRect();
  lineNumberArea->setGeometry(
      QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine() {
  QList<QTextEdit::ExtraSelection> extras;
  if (!isReadOnly()) {
    QTextEdit::ExtraSelection sel;
    QColor lineColor = palette().color(QPalette::AlternateBase);
    sel.format.setBackground(lineColor);
    sel.format.setProperty(QTextFormat::FullWidthSelection, true);
    sel.cursor = textCursor();
    sel.cursor.clearSelection();
    extras.append(sel);
  }
  setExtraSelections(extras);
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
  QPainter painter(lineNumberArea);
  painter.fillRect(event->rect(), palette().color(QPalette::Window));

  // Subtle right border
  painter.setPen(palette().color(QPalette::Mid));
  painter.drawLine(lineNumberArea->width() - 1, event->rect().top(),
                   lineNumberArea->width() - 1, event->rect().bottom());

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top =
      qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
  int bottom = top + qRound(blockBoundingRect(block).height());

  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
      bool isCurrent = (blockNumber == textCursor().blockNumber());
      painter.setPen(isCurrent ? palette().color(QPalette::Text)
                               : palette().color(QPalette::PlaceholderText));

      QFont f = painter.font();
      f.setBold(isCurrent);
      painter.setFont(f);

      painter.drawText(0, top, lineNumberArea->width() - 8,
                       fontMetrics().height(), Qt::AlignRight,
                       QString::number(blockNumber + 1));
    }
    block = block.next();
    top = bottom;
    bottom = top + qRound(blockBoundingRect(block).height());
    ++blockNumber;
  }
}

// ─── MainWindow ────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  editor = new CodeEditor(this);
  setCentralWidget(editor);

  menuBar()->hide();

  // Status bar: cursor pos on left, help hint on right
  cursorLabel = new QLabel("Ln 1, Col 1");
  statusBar()->addWidget(cursorLabel);
  statusBar()->addPermanentWidget(new QLabel("Help: Ctrl+H"));
  statusBar()->setSizeGripEnabled(false);

  createMenus();
  updateTitle();
  resize(600, 450);

  connect(editor->document(), &QTextDocument::modificationChanged, this,
          [this](bool mod) {
            modified = mod;
            updateTitle();
          });
  connect(editor, &CodeEditor::cursorPositionChanged, this, [this]() {
    auto c = editor->textCursor();
    cursorLabel->setText(QString("Ln %1, Col %2")
                             .arg(c.blockNumber() + 1)
                             .arg(c.columnNumber() + 1));
  });
}

void MainWindow::openFileFromPath(const QString &path) {
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    return;
  editor->setPlainText(QTextStream(&f).readAll());
  currentFile = path;
  editor->document()->setModified(false);
  modified = false;
  updateTitle();
}

void MainWindow::createMenus() {
  auto add = [this](QKeySequence key, auto slot) {
    auto *a = new QAction(this);
    a->setShortcut(key);
    connect(a, &QAction::triggered, this, slot);
    addAction(a);
  };

  add(QKeySequence::New, &MainWindow::newFile);
  add(QKeySequence::Open, &MainWindow::openFile);
  add(QKeySequence::Save, &MainWindow::saveFile);
  add(QKeySequence::SaveAs, &MainWindow::saveFileAs);
  add(QKeySequence::Quit, [] { qApp->quit(); });
  add(QKeySequence(Qt::CTRL | Qt::Key_H), &MainWindow::showHelp);
}

void MainWindow::newFile() {
  if (modified) {
    auto r = QMessageBox::question(this, "Unsaved changes",
                                   "Discard current changes?",
                                   QMessageBox::Yes | QMessageBox::No);
    if (r != QMessageBox::Yes)
      return;
  }
  editor->clear();
  currentFile.clear();
  modified = false;
  updateTitle();
}

void MainWindow::openFile() {
  QString path = QFileDialog::getOpenFileName(
      this, "Open File", {},
      "Text files (*.txt *.md *.cpp *.h *.py *.js *.ts *.json *.xml *.html "
      "*.css);;All files (*)");
  if (path.isEmpty())
    return;
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QMessageBox::warning(this, "Error", "Cannot open file: " + path);
    return;
  }
  editor->setPlainText(QTextStream(&f).readAll());
  currentFile = path;
  editor->document()->setModified(false);
  modified = false;
  updateTitle();
}

void MainWindow::saveFile() {
  if (currentFile.isEmpty()) {
    saveFileAs();
    return;
  }
  QFile f(currentFile);
  if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::warning(this, "Error", "Cannot save file.");
    return;
  }
  QTextStream(&f) << editor->toPlainText();
  editor->document()->setModified(false);
  modified = false;
  updateTitle();
}

void MainWindow::saveFileAs() {
  QString path = QFileDialog::getSaveFileName(
      this, "Save As", {}, "Text files (*.txt);;All files (*)");
  if (path.isEmpty())
    return;
  currentFile = path;
  saveFile();
}

void MainWindow::showHelp() {
  auto *dlg = new QDialog(this);
  dlg->setWindowTitle("Keyboard Shortcuts");
  dlg->setMinimumWidth(340);
  dlg->setAttribute(Qt::WA_DeleteOnClose);

  auto *layout = new QVBoxLayout(dlg);
  layout->setSpacing(8);

  auto addRow = [&](const QString &key, const QString &desc) {
    auto *lbl = new QLabel(QString("<b>%1</b> &nbsp; %2").arg(key, desc));
    lbl->setTextFormat(Qt::RichText);
    layout->addWidget(lbl);
  };

  layout->addWidget(new QLabel("<h3 style='margin:0'>Shortcuts</h3>"));

  addRow("Ctrl+N", "New file");
  addRow("Ctrl+O", "Open file");
  addRow("Ctrl+S", "Save");
  addRow("Ctrl+Shift+S", "Save as");
  addRow("Ctrl+H", "Show this help");
  addRow("Ctrl+Q", "Quit");

  layout->addSpacing(8);

  auto *repoLabel =
      new QLabel("<a href='https://github.com/your-username/texteditor'>GitHub "
                 "Repository</a>");
  repoLabel->setTextFormat(Qt::RichText);
  repoLabel->setOpenExternalLinks(true);
  layout->addWidget(repoLabel);

  dlg->exec();
}

void MainWindow::updateTitle() {
  QString name =
      currentFile.isEmpty() ? "Untitled" : QFileInfo(currentFile).fileName();
  setWindowTitle((modified ? "● " : "") + name + " — Text Editor");
}
