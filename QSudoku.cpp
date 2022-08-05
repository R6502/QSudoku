// QSudoku.cpp: Qt-Sudoku-Game, Michael Eisele, 27.02.2006

// ****************************************************************************

#include <stdlib.h> // wg. rand

#include <QDebug> // wg. qDebug () << ...

#include <QApplication>
#include <QWidget>
#include <QFont>
#include <QPushButton>
#include <QCheckBox>
#include <QAction>
#include <QPainter>
#include <QThread>
#include <QDateTime>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QString>
#include <QByteArray>

#include "SudokuField.h"
#include "QSudokuWidget.h"
#include "ui_QSudokuWindowBase_Q5.h"
#include "QSudoku.h"

// ****************************************************************************


const char *field00 = "........."  // 1
                      "........."  // 2
                      "........."  // 3
                      "........."  // 4
                      "........."  // 5
                      "........."  // 6
                      "........."  // 7
                      "........."  // 8
                      "........."; // 9

const char *field_daniel =
                      "7.9.4..2."  // 1
                      "..2378..."  // 2
                      ".4..2..8."  // 3
                      "3...15..."  // 4
                      "....6.41."  // 5
                      ".....7.36"  // 6
                      "9.8..2..1"  // 7
                      "5..13.89."  // 8
                      "2..6..3.7"; // 9

const char *field52 = "1.5.9.6.."  // 1
                      ".7.6..9.4"  // 2
                      "6..3.1..."  // 3
                      "....7...."  // 4
                      ".2.5...9."  // 5
                      "..79..3.."  // 6
                      "21..835.."  // 7
                      ".83...42."  // 8
                      "7.9..5..."; // 9

const char *field53 = "1..6....."
                      "8......19"
                      "9..7..4.."
                      "..8.7..95"
                      "7.4.56..."
                      "615......"
                      "..94..8.2"
                      ".8..63..."
                      ".31.2.5..";

const char *field54 = "9.5..84.."
                      "...5.6.1."
                      "2...39.65"
                      "4.7....9."
                      ".3.7.1.4."
                      "..6.4.2.."
                      ".5.....8."
                      "....8...4"
                      ".1.3.4.7.";


// Spektrum der Wissenschaft, März 2006
const char *field_spec_s104 =
                      ".1..654.."  // 1
                      "....841.."  // 2
                      "4......7."  // 3
                      ".5.19...."  // 4
                      "..3...7.."  // 5
                      "....37.5."  // 6
                      ".8......3"  // 7
                      "..265...."  // 8
                      "..981..2."; // 9


const char *field_spec_s100d =
                      "....3...."  // 1
                      ".15...6.."  // 2
                      "6..2..34."  // 3
                      "...6...8."  // 4
                      ".39...5.."  // 5
                      "5.....9.2"  // 6
                      "........."  // 7
                      "...97.25."  // 8
                      "1...5..7."; // 9

// ****************************************************************************


class QSudokuCreateThread : public QThread {
        SudokuField     *sudoku_field;

      public:
        QSudokuCreateThread (SudokuField *sf) {
          sudoku_field = sf;
        }

        virtual void run () {
          QTime now = QTime::currentTime ();
          int seed = 0;

          seed += now.msec ();
          seed += now.hour ();
          seed += now.minute ();
          seed += now.second ();

          //qDebug ("seed=%d", seed);

          srand (seed);

          //srand (GetTickCount ());
          //qDebug ("rand=%d", rand ());
          //qDebug ("rand=%d", rand ());
          //qDebug ("rand=%d", rand ());

          sudoku_field->clear ();
          sudoku_field->create ();
          sudoku_field->check ();
        }

        void stop () {
          sudoku_field->setStop (true);
        }
      };


// ****************************************************************************


QSudoku::QSudoku (QWidget *parent)
  //: inherited (parent, name, Qt::WStyle_Customize  | Qt::WStyle_NoBorder)
  : inherited (parent)
{
  ui.setupUi (this);

  create_thread = NULL;
  update_timer_id = 0;

  //setWindowOpacity (0.5);

  connect (ui.pb_quit, SIGNAL (clicked ()), qApp, SLOT (quit ()));
  connect (ui.pb_load, SIGNAL (clicked ()), this, SLOT (load ()));
  connect (ui.pb_save, SIGNAL (clicked ()), this, SLOT (save ()));
  connect (ui.pb_clear, SIGNAL (clicked ()), this, SLOT (clear ()));
  connect (ui.pb_solve, SIGNAL (clicked ()), this, SLOT (solve ()));
  connect (ui.pb_new, SIGNAL (clicked ()), this, SLOT (create ()));
  connect (ui.cb_design_mode, SIGNAL (clicked ()), this, SLOT (state_changed ()));
  connect (ui.sw, SIGNAL (statusChanged (const QString &)), ui.tl_status, SLOT (setText (const QString&)));

  sudoku_field = new SudokuField;
  sudoku_field->setupField (field_daniel);
  //sudoku_field->setupField (field_spec_s104);
  ui.sw->setField (sudoku_field);
  //setFont ( QFont ("Script", 20));
  setFont (QFont ("Comic Sans MS", 14));
} // QSudoku::QSudoku


QSudoku::~QSudoku ()
{
  if (create_thread) {
    create_thread->stop ();
    delete create_thread;
    create_thread = NULL;
  }

  if (sudoku_field) delete sudoku_field;
  sudoku_field = NULL;
} // QSudoku::~QSudoku


void QSudoku::save ()
{

   // QString  getSaveFileName(QWidget *parent = nullptr,
   //                          const QString &caption = QString(),
   //                          const QString &dir = QString(),
   //                          const QString &filter = QString(),
   //                          QString *selectedFilter = nullptr,
   //                          QFileDialog::Options options = Options())


  QString filename = QFileDialog::getSaveFileName (this,
                                                   "QSudoku Save File",
                                                   current_filename,
                                                   "QSudoku (*.qsudoku);;All Files (*.*)");

  if (!filename.isNull ()) {
    if (!filename.contains ('.')) filename += ".qsudoku";

    QFileInfo fi (filename);
    int r = QMessageBox::Yes;
    if (fi.exists ()) {
      r = QMessageBox::warning (this,
                                "Warning",
                                "File '" + filename + "' does already exist - Overwrite it ?",
                                QMessageBox::Yes,
                                QMessageBox::No);
    }
    if (r == QMessageBox::Yes) {
      char config_string [SUDOKU_FIELDS + 1];

      sudoku_field->getFieldConfigString (config_string);
      QFile wfile (filename);
      if (wfile.open (QIODevice::WriteOnly)) {
        QTextStream wstream (&wfile);
        wstream << QString (config_string) << "\n";
        wfile.close ();
        current_filename = filename;
      }
      else {
        QMessageBox::warning (this,
                              "Warning",
                              "Unable to write file '" + filename + "'",
                              QMessageBox::Ok,
                              QMessageBox::NoButton);
      }
    }
  }
} // Sudoku::save


void QSudoku::load ()
{
  QString filename = QFileDialog::getOpenFileName (this,
                                                   "QSudoku Open File",
                                                   current_filename,
                                                   "QSudoku (*.qsudoku);;All Files (*.*)");

  if (!filename.isNull ()) {
    QFileInfo fi (filename);
    if (fi.isReadable ()) {

      QFile infile (filename);
      if (infile.open (QIODevice::ReadOnly)) {
        char line_buffer [1000];
        QString text;

        while (infile.readLine (line_buffer, sizeof (line_buffer)) >= 0) {
          text += QString::fromLatin1 (line_buffer);
          if (text.length () > 1000) break;
        }

        infile.close ();

        //text = text.stripWhiteSpace ();
        text = text.trimmed ();

        if (text.length () > 0) {
          QByteArray latin1_text (text.toLatin1 ());
          /*bool success = */ sudoku_field->setupField (latin1_text.data ());
          sudoku_field->check ();
          ui.sw->update ();
          ui.sw->updateStatus ();
        }

        current_filename = filename;
      }
      else {
        QMessageBox::warning (this,
                              "Warning",
                              "Unable to read file '" + filename + "'",
                              QMessageBox::Ok,
                              QMessageBox::NoButton);
      }
    }
  }
} // QSudoku::load


void QSudoku::clear ()
{
  //for (int i = 0; i < SUDOKU_FIELDS; ++i) {
  //  if (!sudoku_field->isPredefined (i)) {
  //    sudoku_field->setNumber (i, 0);
  //  }
  //}
  sudoku_field->clearUser ();
  sudoku_field->check ();
  ui.sw->update ();
  ui.sw->updateStatus ();
} // QSudoku::clear


void QSudoku::solve ()
{
  sudoku_field->clearBacktrackingCount ();
        //sudoku_field->getBacktrackingCount ()

  sudoku_field->check ();
  SudokuField::Result result = sudoku_field->solve ();
  if ((result == SudokuField::Solved) || (result == SudokuField::MultipleSolutions)) {
    sudoku_field->restoreSolution ();
  }
  sudoku_field->check ();
  qDebug ("solver -> %d", result);
  ui.sw->update ();
  ui.sw->updateStatus ();
} // QSudoku::solve


void QSudoku::create ()
{
  if (update_timer_id != 0) killTimer (update_timer_id);
  update_timer_id = 0;

  if (create_thread) {
    create_thread->stop ();
    bool wr = create_thread->wait (100);
    if (!wr) qDebug ("terminate failed");
        else qDebug ("terminate successful");
    delete create_thread;
    create_thread = NULL;
  }
  else {
    update_timer_id = startTimer (250);
    qDebug ("update_timer_id=%d", update_timer_id);
    create_thread = new QSudokuCreateThread (sudoku_field);
    create_thread->start ();
  }

  //sudoku_field->clear ();
  //sudoku_field->create ();
  //sudoku_field->check ();

  show_state ();
  ui.sw->update ();
  ui.sw->updateStatus ();
} // QSudoku::create


void QSudoku::show_state ()
{
  bool creation = create_thread && !create_thread->isFinished ();
  ui.pb_load->setEnabled (!creation);
  ui.pb_save->setEnabled (!creation);
  ui.pb_clear->setEnabled (!creation);
  ui.pb_solve->setEnabled (!creation);
  //ui.pb_new->setEnabled (creation);
  ui.cb_design_mode->setEnabled (!creation);
  ui.sw->setEnabled (!creation);
} // QSudoku::show_state


void QSudoku::state_changed ()
{
  ui.sw->setDesignMode (ui.cb_design_mode->isChecked ());
} // QSudoku::state_changed


void QSudoku::timerEvent (QTimerEvent *)
{
  ui.sw->update ();
  ui.sw->updateStatus ();

  if (create_thread) {
    if (create_thread->isFinished ()) {
      delete create_thread;
      create_thread = NULL;
      qDebug ("Thread finished");
      show_state ();
    }
  }

  if (!create_thread && (update_timer_id != 0)) {
    killTimer (update_timer_id);
    update_timer_id = 0;
    show_state ();
  }
} // QSudoku::timerEvent


// ****************************************************************************


int main (int argc, char **argv)
{
  QApplication app (argc, argv);
  QSudoku *qs = NULL;

  qs = new QSudoku ();
  qs->show ();
  //qs->resize (500, 300);
  //app.setMainWidget (qs);

  int r = app.exec ();

  return r;
} // main


// ****************************************************************************


