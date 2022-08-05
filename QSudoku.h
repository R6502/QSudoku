// QSudoku.h: Qt-Sudoku-Game, Michael Eisele, 27.02.2006

#ifndef Q_SUDOKU_H
#define Q_SUDOKU_H

// ****************************************************************************

#include <qwidget.h>
#include <qpainter.h>

#include "SudokuField.h"
#include "QSudokuWidget.h"

#include "ui_QSudokuWindowBase_Q5.h"

// ****************************************************************************

class QSudokuCreateThread;

// ****************************************************************************

class QSudoku : public QWidget {
        Q_OBJECT
        typedef QWidget inherited;

        Ui_QSudokuWindowBase ui;

      private:
        SudokuField             *sudoku_field;
        QSudokuCreateThread     *create_thread;
        int                     update_timer_id;
        QString                 current_filename;

      public:
        QSudoku (QWidget *parent = 0);
        virtual ~QSudoku ();

        virtual void timerEvent (QTimerEvent *te);

      public slots:
        void save ();
        void load ();
        void clear ();
        void solve ();
        void create ();

        void show_state ();
        void state_changed ();
      };

// ****************************************************************************

#endif // Q_SUDOKU_H

