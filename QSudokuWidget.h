// QSudokuWidget.h: Qt-Sudoku-Widget, Michael Eisele, 27.02.2006

#ifndef Q_SUDOKU_WIDGET_H
#define Q_SUDOKU_WIDGET_H

// ****************************************************************************

#include <qwidget.h>
#include <qfont.h>
#include <qpainter.h>

#include "SudokuField.h"

// ****************************************************************************

class QSudokuWidget : public QWidget {
        Q_OBJECT
        typedef QWidget inherited;

      private:
        SudokuField     *sudoku_field;

        int             cursor_position;

        QFont           number_font,
                        user_font,
                        label_font;

        int             number_width,
                        number_height,
                        number_descent,
                        label_height,
                        label_descent,
                        default_field_size,
                        minimum_field_size,
                        field_size,
                        top,
                        left;
        bool            thick_block_lines,
                        design_mode;

      private:
        void setupFontMetrics ();

      public:
        QSudokuWidget (QWidget *parent = 0);
        ~QSudokuWidget ();

        void setField (SudokuField *f);
        void setDesignMode (bool on);
        void updateStatus ();

        virtual void mousePressEvent (QMouseEvent * e);
        //virtual void mouseReleaseEvent (QMouseEvent * e);
        //virtual void mouseDoubleClickEvent (QMouseEvent * e);
        //virtual void contextMenuEvent (QContextMenuEvent * e);
        virtual void resizeEvent (QResizeEvent *e);
        virtual void paintEvent (QPaintEvent *e);
        virtual void keyPressEvent (QKeyEvent *ke);
        virtual void timerEvent (QTimerEvent *te);

        virtual QSize sizeHint () const;
        virtual QSize minimumSizeHint () const;

      public slots:

      signals:
        void statusChanged (const QString &status);
      };


// ****************************************************************************

#endif // Q_SUDOKU_WIDGET_H

