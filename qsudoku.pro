
TEMPLATE        = app

OBJECTS_DIR     = tmp
MOC_DIR         = tmp
UI_DIR          = tmp

INCLUDEPATH     = .

# CONFIG   += qt warn_on debug
CONFIG   += qt warn_on release
QT             += core widgets printsupport

HEADERS         = SudokuField.h \
                  QSudokuWidget.h \
                  QSudoku.h

SOURCES         = SudokuField.cpp \
                  QSudokuWidget.cpp \
                  QSudoku.cpp

FORMS           = QSudokuWindowBase_Q5.ui

TARGET          = qsudoku

