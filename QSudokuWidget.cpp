// QSudokuWidget.cpp: Qt-Sudoku-Widget, Michael Eisele, 27.02.2006

// ****************************************************************************

#include <QDebug>

#include <QApplication>
#include <QWidget>
#include <QPrinter>
#include <QString>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QStyle>
#include <QStyleOption>

#include "SudokuField.h"
#include "QSudokuWidget.h"

// ****************************************************************************


QSudokuWidget::QSudokuWidget (QWidget *parent)
  : inherited (parent)
{
  thick_block_lines = false;
  design_mode = false;
  field_size = 0;
  top = 0;
  left = 0;
  number_width = 0;
  number_descent = 0;
  number_height = 0;
  label_descent = 0;
  label_height = 0;
  minimum_field_size = 0;
  cursor_position = 0;
  sudoku_field = NULL;
  number_font = QFont ("Helvetica", 12);
  //user_font = QFont ("Script", 12);
  user_font = QFont ("Comic Sans MS", 10);
  label_font = QFont ("Helvetica", 10);
  setupFontMetrics ();
  default_field_size = minimum_field_size;
  setFocusPolicy (Qt::StrongFocus);

  setAutoFillBackground (true);
  setBackgroundRole (QPalette::Window);

  // setEraseColor (Qt::white);
  // setEraseColor (QColor (250, 250, 250));
  //setEraseColor (lightGray);
} // QSudokuWidget::QSudokuWidget


QSudokuWidget::~QSudokuWidget ()
{
  sudoku_field = NULL;
} // QSudokuWidget::~QSudokuWidget


void QSudokuWidget::setDesignMode (bool on)
{
  design_mode = on;
  update ();
} // QSudokuWidget::setDesignMode


void QSudokuWidget::setupFontMetrics ()
{
  // Number font
  QFontMetrics fm (number_font);
  number_height = fm.height ();
  number_descent = fm.descent ();
  number_width = fm.width ("9");
  minimum_field_size = number_width;
  if (number_height > minimum_field_size) minimum_field_size = number_height;
  // minimum_field_size += 3;

  // Label font
  QFontMetrics fml (label_font);
  label_height = fml.height ();
  label_descent = fml.descent ();
} // QSudokuWidget::setupFontMetrics


void QSudokuWidget::setField (SudokuField *f)
{
  sudoku_field = f;
  sudoku_field->check ();
  updateStatus ();
  update ();
} // QSudokuWidget::setField


void QSudokuWidget::updateStatus ()
{
  QString state = "";
  if (sudoku_field) {
    if (sudoku_field->isWrong ()) state = "No solution possible";
    else if (sudoku_field->solveResult () == SudokuField::NotSolved) state = "No solution found";
    else if (sudoku_field->isSolved ()) {
      //if ((sudoku_field->solveResult () == SudokuField::Solved) || (result == SudokuField::MultipleSolutions)) {
      if (sudoku_field->solveResult () == SudokuField::MultipleSolutions) state = "Multiple Solutions";
      else {
        state = "Solved !!";
        state += " (BacktrackingCount=" + QString::number (sudoku_field->getBacktrackingCount ()) + ")";
      }
    }
    // else if (sudoku_field->isEasy ()) state = "Easy";
    else state += "Free=" + QString::number (sudoku_field->getFree ());
  }
  else state = "Internal Error";

  //qDebug ("Status: %s", (const char*) state);

  emit statusChanged (state);
} // QSudokuWidget::updateStatus


void QSudokuWidget::mousePressEvent (QMouseEvent *e)
{
  int x = e->x (),
      y = e->y ();

  if (field_size > 0) {
    if ((x >= left) && (y >= top)) {
      x -= left;
      y -= top;
      if ((x <= SUDOKU_SIZE * field_size) && (y <= SUDOKU_SIZE * field_size)) {
        int fx = x / field_size,
            fy = y / field_size,
            cp =  fy * SUDOKU_SIZE + fx;
        if (cp != cursor_position) {
          cursor_position = cp;
          update ();
        }
      }
    }
  }
} // QSudokuWidget::mousePressEvent


//void QSudokuWidget::mouseReleaseEvent (QMouseEvent *e)
//{
//} // QSudokuWidget::mouseReleaseEvent
//
//
//void QSudokuWidget::mouseDoubleClickEvent (QMouseEvent *e)
//{
//} // QSudokuWidget::mouseDoubleClickEvent
//
//
//void QSudokuWidget::contextMenuEvent (QContextMenuEvent *e)
//{
//} // QSudokuWidget::contextMenuEvent


void QSudokuWidget::resizeEvent (QResizeEvent *)
{
  bool  repeated = false;

  int   w = 0, //width (),
        h = 0; //height ();

int margin = 6;

calculate_fontsize:

  w = width ();
  h = height ();

  if (thick_block_lines) {
    if (w > 4) w -= 4;
    if (h > 4) h -= 4;
  }

  if (w > margin) w -= margin;
  if (h > margin) h -= margin;
  //if (h > bh) h -=bh;

  int   fsw = w / SUDOKU_SIZE,
        fsh = h / SUDOKU_SIZE,
        fs_min = fsh;

  if (fsw < fs_min) fs_min = fsw;

  while ((minimum_field_size > fs_min) && (number_font.pointSize () > 3)) {
    number_font.setPointSize (number_font.pointSize () - 1);
    user_font.setPointSize (user_font.pointSize () - 1);
    setupFontMetrics ();
  }

  bool shrink = false;
  while ((minimum_field_size < fs_min) && (number_font.pointSize () < 50)) {
    number_font.setPointSize (number_font.pointSize () + 1);
    user_font.setPointSize (user_font.pointSize () + 1);
    setupFontMetrics ();
    shrink = true;
  }

  if (shrink) {
    number_font.setPointSize (number_font.pointSize () - 1);
    user_font.setPointSize (user_font.pointSize () - 1);
    setupFontMetrics ();
  }

  field_size = minimum_field_size;
  if (fs_min > field_size) field_size = fs_min;
  top  = (h - SUDOKU_SIZE * field_size) / 2;
  left = (w - SUDOKU_SIZE * field_size) / 2;

top  += margin / 2;
left += margin / 2;

  bool old_thick_block_lines = thick_block_lines;
  thick_block_lines = (field_size >= 30);

  if (!repeated && (old_thick_block_lines != thick_block_lines)) {
    repeated = true;
    goto calculate_fontsize;
  }
} // QSudokuWidget::resizeEvent


void QSudokuWidget::paintEvent (QPaintEvent *)
{
  QPainter painter;
  painter.begin (this);

  int fx = 0, fy = 0;

  QColor fline_color (Qt::gray),
         //bline_color (Qt::black), // (red);
         bline_color (QColor (130, 130, 130)),
         pnum_color (Qt::red), // (red);
         //unum_color (Qt::darkGray),
         unum_color (Qt::gray),
         //unum_color (black),
         //unum_color (QColor (50, 50, 90)),
         wrong_color = QColor (255, 160, 160), //(red) .light (),
         good_color  = QColor (160, 255, 160), //(green) .light (),
         cursor_color (Qt::blue);

  int    lext = 0;
  if (thick_block_lines) lext = 4;


  for (fx = 1; fx < SUDOKU_SIZE; ++fx) {
    int x = left + fx * field_size;
    bool thick = false;

    if (thick_block_lines) x += (fx / SUDOKU_BSIZE) * 2;

    if ((fx % SUDOKU_BSIZE) == 0) {
      painter.setPen (bline_color);
      thick = thick_block_lines;
    }
    else painter.setPen (fline_color);


    if (thick) {
      x -= 1;

      painter.drawLine (x - 1,
                        top,
                        x - 1,
                        top + SUDOKU_SIZE * field_size + lext);

      painter.drawLine (x + 1,
                        top,
                        x + 1,
                        top + SUDOKU_SIZE * field_size + lext);
    }

    painter.drawLine (x,
                      top,
                      x,
                      top + SUDOKU_SIZE * field_size + lext);

  }

  for (fy = 1; fy < SUDOKU_SIZE; ++fy) {
    int y = top + fy * field_size;
    bool thick = false;

    if (thick_block_lines) y += (fy / SUDOKU_BSIZE) * 2;

    if ((fy % SUDOKU_BSIZE) == 0) {
      painter.setPen (bline_color);
      thick = thick_block_lines;
    }
    else painter.setPen (fline_color);

    if (thick) {
      y -= 1;

      painter.drawLine (left,
                        y - 1,
                        left + SUDOKU_SIZE * field_size + lext,
                        y - 1);

      painter.drawLine (left,
                        y + 1,
                        left + SUDOKU_SIZE * field_size + lext,
                        y + 1);
    }

    painter.drawLine (left,
                      y,
                      left + SUDOKU_SIZE * field_size + lext,
                      y);

  }

  painter.setFont (number_font);

  for (fx = 0; fx < SUDOKU_SIZE; ++fx) {
    for (fy = 0; fy < SUDOKU_SIZE; ++fy) {
      int field_index = fy * SUDOKU_SIZE + fx;
      int x = left + fx * field_size,
          y = top + fy * field_size;

      if (thick_block_lines) {
        x += (fx / SUDOKU_BSIZE) * 2;
        y += (fy / SUDOKU_BSIZE) * 2;
      }

      // Field contents
      char number = 0;
      bool wrong = false,
           good = false,
           predefined = false;

      unsigned int nmask = 0;

      if (sudoku_field) {
        number = sudoku_field->getNumber (field_index);
        nmask = sudoku_field->getNumberMask (field_index);
        predefined = sudoku_field->isPredefined (field_index);
        wrong = sudoku_field->isWrong (field_index);
        if (!predefined && (number == 0)) {
          good = sudoku_field->hasOnePossibleValue (field_index);
        }
      }

      // Status
      if (wrong || good) {
        if (wrong) painter.fillRect (x + 1, y + 1, field_size - 1, field_size - 1, wrong_color);
              else painter.fillRect (x + 1, y + 1, field_size - 1, field_size - 1, good_color);
      }

      //// Cursor
      //if (field_index == cursor_position) {
      //  QPen cpen;
      //  cpen.setColor (cursor_color);
      //  cpen.setWidth (thick_block_lines ? 3 : 2);
      //  painter.setPen (cpen);
      //  painter.drawRect (x, y, field_size + 1, field_size + 1);
      //}

      // Number
      if (number) {
        char tmp [2];
        tmp [0] = number;
        tmp [1] = 0;

        int tx = x + field_size / 2 - number_width / 2,
            ty = y + field_size / 2 + number_height / 2 - number_descent;// / 2 - 2;

        if (predefined) painter.setFont (number_font);
                   else painter.setFont (user_font);

        // Schatten
        int sw = number_width / 10;
        if (sw && predefined) {
          painter.setPen (Qt::darkGray);
          painter.drawText (tx + sw, ty + sw,  tmp);
        }

        if (predefined) painter.setPen (pnum_color);
                   else painter.setPen (unum_color);

        painter.drawText (tx, ty,  tmp);
      }

      if (!predefined) { // Mask
        int th = number_descent,
            tw = field_size / (2 * SUDOKU_SIZE),
            mx = x + field_size / 2 - (tw * SUDOKU_SIZE / 2),
            my = y + field_size - 1, // - th / 2,
            mi = 0,
            tmask = 1;

        if (tw == 0) tw = 1;

        for (mi = 0; mi < SUDOKU_SIZE; ++mi) {
          if (nmask & tmask) painter.setPen (Qt::red);
                        else painter.setPen (Qt::gray);
          for (int i = 0; i < tw; ++i) {
            if (i < (2*tw/3)) painter.drawLine (mx, my, mx, my - th / 2);
            //mx += tw;
            mx += 1;
          }
          tmask <<= 1;
        }
      }
    }
  }

  for (fx = 0; fx < SUDOKU_SIZE; ++fx) {
    for (fy = 0; fy < SUDOKU_SIZE; ++fy) {
      int field_index = fy * SUDOKU_SIZE + fx;
      int x = left + fx * field_size,
          y = top + fy * field_size;

      if (thick_block_lines) {
        x += (fx / SUDOKU_BSIZE) * 2;
        y += (fy / SUDOKU_BSIZE) * 2;
      }

      // Cursor
      if (field_index == cursor_position) {
        QPen cpen;
        cpen.setColor (cursor_color);
        cpen.setWidth (thick_block_lines ? 3 : 2);
        painter.setPen (cpen);
        painter.drawRect (x, y, field_size + 1, field_size + 1);
      }
    }
  }

#if 0
  // Show status
  QString state = "";
  if (sudoku_field) {
    if (sudoku_field->isWrong ()) state = "No solution possible";
    else if (sudoku_field->solveResult () == SudokuField::NotSolved) state = "No solution found";
    else if (sudoku_field->isSolved ()) {
      //if ((sudoku_field->solveResult () == SudokuField::Solved) || (result == SudokuField::MultipleSolutions)) {
      if (sudoku_field->solveResult () == SudokuField::MultipleSolutions) state = "Multiple Solutions";
      else state = "Solved !!";
    }
    // else if (sudoku_field->isEasy ()) state = "Easy";
    else state += "Free=" + QString::number (sudoku_field->getFree ());
  }
  else state = "Internal Error";

  state = "Status: " + state;

  //state += ", pos=" + QString::number (cursor_position);

  painter.setPen (black);
  painter.setFont (label_font);
  painter.drawText (left, height () - label_descent, state);
#endif

  QStyleOption style_option;
  style_option.state = QStyle::State_Sunken | QStyle::State_Enabled;
  style_option.rect = QRect (0, 0, width (), height ());

  style () ->drawPrimitive (QStyle::PE_FrameWindow,
                            &style_option,
                            &painter,
                            //QRect (0, 0, width (), height ()),
                            this);
                            // /*parentWidget () ->*/ colorGroup (),
                            //
                            // QStyleOption::Default);

  painter.end ();
} // QSudokuWidget::paintEvent


void QSudokuWidget::keyPressEvent (QKeyEvent *e)
{

  switch (e->key ()) {

    case Qt::Key_Left:
         cursor_position -= 1;
         if (cursor_position < 0) cursor_position = SUDOKU_FIELD_MAX;
         break;

    case Qt::Key_Right:
         cursor_position += 1;
         if (cursor_position > SUDOKU_FIELD_MAX) cursor_position = 0;
         break;

    case Qt::Key_Up:
         cursor_position -= SUDOKU_SIZE;
         if (cursor_position < 0) cursor_position += SUDOKU_FIELDS;
         break;

    case Qt::Key_Down:
         cursor_position += SUDOKU_SIZE;
         if (cursor_position > SUDOKU_FIELD_MAX) cursor_position -= SUDOKU_FIELDS;
         break;

    case Qt::Key_Space:
         if (sudoku_field) {
           if (!sudoku_field->isPredefined (cursor_position) || design_mode) {
             char number = sudoku_field->getNumber (cursor_position);
             if (number != 0) {
               if (design_mode) sudoku_field->setPredefinedNumber (cursor_position, 0);
                           else sudoku_field->setNumber (cursor_position, 0);
               sudoku_field->check ();
               updateStatus ();
             }
           }
         }
         break;

    case Qt::Key_Plus:
         if (sudoku_field) {
           if (!sudoku_field->isPredefined (cursor_position) || design_mode) {
             char number = sudoku_field->getNumber (cursor_position);
             if (number == 0) number = '1';
             else if (number == '9') number = 0;
             else ++number;
             if (design_mode) sudoku_field->setPredefinedNumber (cursor_position, number);
                         else sudoku_field->setNumber (cursor_position, number);
             sudoku_field->check ();
             updateStatus ();
           }
         }
         break;

    case Qt::Key_Minus:
         if (sudoku_field) {
           if (!sudoku_field->isPredefined (cursor_position) || design_mode) {
             char number = sudoku_field->getNumber (cursor_position);
             if (number == 0) number = '9';
             else if (number == '1') number = 0;
             else --number;
             if (design_mode) sudoku_field->setPredefinedNumber (cursor_position, number);
                         else sudoku_field->setNumber (cursor_position, number);
             sudoku_field->check ();
             updateStatus ();
           }
         }
         break;

    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
         if (sudoku_field) {
           if (!sudoku_field->isPredefined (cursor_position) || design_mode) {
             char number = sudoku_field->getNumber (cursor_position);
             char ascii = 0;
             if      (e->key () == Qt::Key_0) ascii = '0';
             else if (e->key () == Qt::Key_1) ascii = '1';
             else if (e->key () == Qt::Key_2) ascii = '2';
             else if (e->key () == Qt::Key_3) ascii = '3';
             else if (e->key () == Qt::Key_4) ascii = '4';
             else if (e->key () == Qt::Key_5) ascii = '5';
             else if (e->key () == Qt::Key_6) ascii = '6';
             else if (e->key () == Qt::Key_7) ascii = '7';
             else if (e->key () == Qt::Key_8) ascii = '8';
             else if (e->key () == Qt::Key_9) ascii = '9';

             if (number != ascii) {
               if (design_mode) sudoku_field->setPredefinedNumber (cursor_position,ascii);
                           else sudoku_field->setNumber (cursor_position, ascii);
               sudoku_field->check ();
               updateStatus ();
             }
           }
         }
         break;
  }

  update ();
} // QSudokuWidget::keyPressEvent


void QSudokuWidget::timerEvent (QTimerEvent *)
{
} // QSudokuWidget::timerEvent


QSize QSudokuWidget::sizeHint () const
{
  //int w = 9 * (default_field_size + 1) + 6;
  //return QSize (w, w);
  return minimumSizeHint ();
} // QSudokuWidget::sizeHint


QSize QSudokuWidget::minimumSizeHint () const
{
  int w = 9 * (default_field_size + 1) + 6;
  // int w = 50;
  return QSize (w, w);
} // QSudokuWidget::minimumSizeHint


// ****************************************************************************


