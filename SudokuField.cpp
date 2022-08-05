// SudokuField.cpp: Sudoku-Game, Michael Eisele, 27.02.2006

// ****************************************************************************

#include <stdio.h> // wg. printf
#include <stdlib.h> // wg. rand
//#include <qt.h> // wg.qDebug
#include <qDebug.h> // wg.qDebug

#include "SudokuField.h"

// ****************************************************************************

#define FM_PREDEFINED           (1<<31)
#define FM_WRONG                (1<<30)
#define FM_ONEPOS               (1<<29)
#define FM_NUMBER               0x000001ff
#define FM_NPOS                 12
#define FM_NMASK                (FM_NUMBER<<FM_NPOS)

// ****************************************************************************


static inline int bitcount (unsigned int mask)
{
  // calculate bit count of bit mask
  int bc = 0;
  mask &= FM_NUMBER;
  while (mask) {
    if      ((mask & 0xff) == 0) mask >>= 8;
    else if ((mask & 0x0f) == 0) mask >>= 4;
    else {
      if (mask & 0x01) ++bc;
      mask >>= 1;
    }
  }
  return bc;
} // bitcount


// ****************************************************************************


SudokuField::SudokuField ()
{
  //saved_solution = NULL;
  clear ();
} // SudokuField::SudokuField


SudokuField::~SudokuField ()
{
  //if (saved_solution) delete saved_solution;
  //saved_solution = NULL;
} // SudokuField::~SudokuField


void SudokuField::setStop (bool s)
{
  stop = s;
} // SudokuField::setStop


void SudokuField::clear ()
{
  for (int i = 0; i < SUDOKU_FIELDS; ++i) {
    field [i] = 0;
    solution_field [i] = 0;
  }
  solve_result = Unkown;
  wrong = false;
  solved = false;
  easy = false;
  stop = false;
  free = 0;
  backtracking_count = 0;
} // SudokuField::clear


void SudokuField::clearUser ()
{
  for (int i = 0; i < SUDOKU_FIELDS; ++i) {
    if (!isPredefined (i)) setNumber (i, 0);
  }
  solve_result = Unkown;
} // SudokuField::clearUser


void SudokuField::saveSolution ()
{
  for (int i = 0; i < SUDOKU_FIELDS; ++i) {
    solution_field [i] = field [i];
  }
} // SudokuField::saveSolution


void SudokuField::restoreSolution ()
{
  for (int i = 0; i < SUDOKU_FIELDS; ++i) {
    field [i] = solution_field [i];
  }
} // SudokuField::restoreSolution


bool SudokuField::setupField (const char *init_string)
{
  clear ();
  int i = 0;
  while ((i < (SUDOKU_WIDTH * SUDOKU_HEIGHT)) && *init_string) {
    //setPredefinedNumber (i, *init_string);
    char number = *init_string;

    unsigned int nm = 0;

    if ((number >= '1') && (number <= '9')) {
       nm = 1 << (number - '1');
       nm |= FM_PREDEFINED;
    }

    if ((number >= 'a') && (number <= ('a' + 8))) {
       nm = 1 << (number - 'a');
    }

    field [i] = nm;

    init_string++;
    i++;
  }
  return true;
} // SudokuField::setupField


void SudokuField::getFieldConfigString (char *config_string)
{
  int i = 0;
  while (i < (SUDOKU_WIDTH * SUDOKU_HEIGHT)) {
    char ch = getNumber (i);
    if (ch) {
      if (!isPredefined (i)) ch += 'a' - '1';
    }
    else ch = '.';

    *config_string++ = ch;
    i++;
  }

  *config_string = '\0';
} // SudokuField::getFieldConfigString


char SudokuField::getNumber (int field_index) const
{
  char number = 0;
  unsigned int fn = field [field_index] & FM_NUMBER;
  if (fn & (1<<0)) number = '1';
  else if (fn & (1<<1)) number = '2';
  else if (fn & (1<<2)) number = '3';
  else if (fn & (1<<3)) number = '4';
  else if (fn & (1<<4)) number = '5';
  else if (fn & (1<<5)) number = '6';
  else if (fn & (1<<6)) number = '7';
  else if (fn & (1<<7)) number = '8';
  else if (fn & (1<<8)) number = '9';
  return number;
} // SudokuField::getNumber


unsigned int SudokuField::getNumberMask (int field_index) const
{
  return (field [field_index]  >> FM_NPOS) & FM_NUMBER;
} // SudokuField::getNumberMask


bool SudokuField::hasOnePossibleValue (int field_index) const
{
  return (field [field_index] & FM_ONEPOS) != 0;
} // SudokuField::hasOnePossibleValue


bool SudokuField::isEmpty (int field_index) const
{
  return getNumber (field_index) == 0;
} // SudokuField::isEmpty


bool SudokuField::isWrong (int field_index) const
{
  return (field [field_index] & FM_WRONG) != 0;
} // SudokuField::isWrong


bool SudokuField::isPredefined (int field_index) const
{
  return (field [field_index] & FM_PREDEFINED) != 0;
} // SudokuField::isPredefined


void SudokuField::setNumber (int field_index, char number)
{
  unsigned int nm = 0;
  if ((number >= '1') && (number <= '9')) {
     nm = 1 << (number - '1');
  }
  field [field_index] = nm;
} // SudokuField::setNumber


void SudokuField::setPredefinedNumber (int field_index, char number)
{
  unsigned int nm = 0;
  if ((number >= '1') && (number <= '9')) {
     nm = 1 << (number - '1');
  }
  field [field_index] = nm;
  if (nm) field [field_index] |= FM_PREDEFINED;
} // SudokuField::setPredefinedNumber


SudokuField::Result SudokuField::solve (int field_index)
{
  check ();
  //printf ("solve: fi=%d, free=%d \n", field_index, free);

  //if (wrong) printf (" -> NotSolved\n\n");
  //if (solved) printf (" -> Solved !!!\n\n");
  if (solved) saveSolution ();

  if (stop) return NotSolved;

  if (wrong) return NotSolved;
  else if (solved) return Solved;

  Result result = NotSolved;

  if (easy) { // try with easy placements
    for (int fi = 0; fi < SUDOKU_FIELDS; ++fi) {
      unsigned int num = field [fi];
      if (((num & FM_NUMBER) == 0) && (num & FM_ONEPOS))  { // empty field to test
        unsigned int fmask = (field [fi] >> FM_NPOS) & FM_NUMBER;
        field [fi] = fmask & FM_NUMBER;

        result = solve (field_index);

        //printf ("  solve simple: fi=%d, mask=%03x -> %d\n", fi, fmask, result);

        field [fi] = 0; // restore previous contents
        break;
      }
    }
  }
  else { // try with back tracking
    //backtracking_count += 1;

    while (field_index < SUDOKU_FIELDS) {
      unsigned int num = field [field_index] & FM_NUMBER;

      if (num == 0) { // empty field to test
        unsigned int fmask = (field [field_index] >> FM_NPOS) & FM_NUMBER,
                     tmask = 1 << (SUDOKU_SIZE - 1);

        int solutions = 0;

        while (tmask) {
          if (fmask & tmask) {
            field [field_index] = tmask & FM_NUMBER;

            Result tres = solve (field_index + 1);

            //printf ("  solve bt: fi=%d, mask=%03x ->  %d\n", field_index, tmask, tres);

            if (tres == Solved) backtracking_count += 1;

            if (tres == Solved) solutions += 1;
            else if (tres == MultipleSolutions) solutions += 10;

            if (solutions > 1) break;
          }
          tmask >>= 1;
        }

        field [field_index] = 0; // restore previous contents

        if (solutions == 1) result = Solved;
        else if (solutions > 1) result = MultipleSolutions;

        break;
      }

      ++field_index;
    }
  }

  solve_result = result;

  return result;
} // SudokuField::solve


void SudokuField::check ()
{
  solved = false;
  wrong = false;
  easy = false;
  free = 0;

  for (int fi = 0; fi < SUDOKU_FIELDS; ++fi) {
    unsigned int num = field [fi] & FM_NUMBER,
                 mask = FM_NUMBER;

    bool duplicate = false;

    if (num == 0) ++free;

    // check column
    int n = SUDOKU_SIZE;
    int fi_col = fi % SUDOKU_SIZE;
    while (n--) {
      unsigned int field_number_col = field [fi_col];
      if ((fi_col != fi) && (num & field_number_col)) duplicate = true;
      mask &= ~ field_number_col;
      fi_col += SUDOKU_SIZE;
    }

    // check row
    int fi_row = (fi / SUDOKU_SIZE) * SUDOKU_SIZE;
    n = SUDOKU_SIZE;
    while (n--) {
      unsigned int field_number_row = field [fi_row];
      if ((fi_row != fi) && (num & field_number_row)) duplicate = true;
      mask &= ~ field_number_row;
      fi_row++;
    }

    // check block
    int z = fi / SUDOKU_SIZE;
    int bz = (z / (SUDOKU_SIZE / SUDOKU_BSIZE)) * (SUDOKU_SIZE / SUDOKU_BSIZE);
    int s = fi % SUDOKU_SIZE;
    int bs = (s / (SUDOKU_SIZE / SUDOKU_BSIZE)) * (SUDOKU_SIZE / SUDOKU_BSIZE);
    int fi_block = bz * SUDOKU_SIZE + bs;
    int m = SUDOKU_SIZE / SUDOKU_BSIZE;
    while (m--) {
      int n = SUDOKU_SIZE / SUDOKU_BSIZE;
      while (n--) {
        unsigned int field_number_block = field [fi_block];
        if ((fi_block != fi) && (num & field_number_block)) duplicate = true;
        mask &= ~ field_number_block;
        fi_block++;
      }
      fi_block -= SUDOKU_SIZE / SUDOKU_BSIZE;
      fi_block += SUDOKU_SIZE;
    }

    field [fi] &= ~ (FM_NMASK | FM_WRONG | FM_ONEPOS);

    if (duplicate) {
      field [fi] |= FM_WRONG;
      wrong = true;
    }

    if (num != 0) mask = 0;

    field [fi] |= (mask & FM_NUMBER) << FM_NPOS;

    // calculate bit count of bit mask
    int bc = bitcount (mask);

    if (bc == 1) {
      field [fi] |= FM_ONEPOS;
      if (num == 0) easy = true;
    }
    else if ((bc == 0) && (num == 0)) {
      field [fi] |= FM_WRONG;
      wrong = true;
    }
  }

  //if (!easy && !wrong) {
  if (!wrong) {
    // Maske von oben ist nicht alles: Zusätzlich prüfen ob ein Bit die einzige Möglichkeit ist
    reduce ();
  }

  solved = (free == 0) && !wrong;
} // SudokuField::check


void SudokuField::reduce ()
{
  for (int fi = 0; fi < SUDOKU_FIELDS; ++fi) {
    //bool debug = false;

    //debug = (fi == 77);

    unsigned int possible_numbers_original = (field [fi] >> FM_NPOS) & FM_NUMBER;
    unsigned int possible_numbers = 0;
    //unsigned int number = field [fi] & FM_NUMBER;

            //if (debug) {
            //  qDebug ("%d/0: %02x num=%d %08x bc=%d", fi, possible_numbers, number, field [fi], bitcount (possible_numbers));
            //}

    if (bitcount (possible_numbers_original) > 1) {
      // check column
      int bc = 0;
      int n = SUDOKU_SIZE;
      int fi_col = fi % SUDOKU_SIZE;

      possible_numbers = possible_numbers_original;

      while (n--) {
        unsigned int field_mask_col = (field [fi_col] >> FM_NPOS) & FM_NUMBER;
        unsigned int number_col = field [fi_col] & FM_NUMBER;
        if (fi_col != fi) possible_numbers &= ~ (field_mask_col | number_col);
        fi_col += SUDOKU_SIZE;
      }

      bc = bitcount (possible_numbers);

            //if (debug) {
            //  qDebug ("%d/c: bc=%d %02x", fi, bc, possible_numbers);
            //}

      if (bc == 0) possible_numbers = possible_numbers_original;

      if (bc != 1) { // check row
        int fi_row = (fi / SUDOKU_SIZE) * SUDOKU_SIZE;
        //possible_numbers = possible_numbers_original;
        n = SUDOKU_SIZE;
        while (n--) {
          unsigned int field_mask_row = (field [fi_row] >> FM_NPOS) & FM_NUMBER;
          unsigned int number_row = field [fi_row] & FM_NUMBER;
          if (fi_row != fi) possible_numbers &= ~ (field_mask_row | number_row);
          fi_row++;
        }

        bc = bitcount (possible_numbers);
      }

            //if (debug) {
            //  qDebug ("%d/r: bc=%d %02x", fi, bc, possible_numbers);
            //}

      if (bc == 0) possible_numbers = possible_numbers_original;

      if (bc != 1) { // check block
        int z = fi / SUDOKU_SIZE;
        int bz = (z / (SUDOKU_SIZE / SUDOKU_BSIZE)) * (SUDOKU_SIZE / SUDOKU_BSIZE);
        int s = fi % SUDOKU_SIZE;
        int bs = (s / (SUDOKU_SIZE / SUDOKU_BSIZE)) * (SUDOKU_SIZE / SUDOKU_BSIZE);
        int fi_block = bz * SUDOKU_SIZE + bs;
        int m = SUDOKU_SIZE / SUDOKU_BSIZE;

        //possible_numbers = possible_numbers_original;

        while (m--) {
          int n = SUDOKU_SIZE / SUDOKU_BSIZE;
          while (n--) {
            unsigned int field_mask_block = (field [fi_block] >> FM_NPOS) & FM_NUMBER;
            unsigned int number_block = field [fi_block] & FM_NUMBER;
            if (fi_block != fi) possible_numbers &= ~ (field_mask_block | number_block);
            fi_block++;
            //if (debug) {
            //  qDebug ("%d: %08x", fi_block, possible_numbers);
            //}
          }
          fi_block -= SUDOKU_SIZE / SUDOKU_BSIZE;
          fi_block += SUDOKU_SIZE;
        }

        bc = bitcount (possible_numbers);
      }

            //if (debug) {
            //  qDebug ("%d/b: bc=%d %02x", fi, bc, possible_numbers);
            //}

      //if (bc == 0) possible_numbers = possible_numbers_original;

      if (bc == 1) { // store changed mask
        field [fi] &= ~ (FM_NMASK);
        field [fi] |= (possible_numbers & FM_NUMBER) << FM_NPOS;
        field [fi] |= FM_ONEPOS;

        unsigned int number = field [fi] & FM_NUMBER;
        if (number == 0) easy = true;
      }
    }
  }
} // SudokuField::reduce


void SudokuField::create ()
{
  stop = false;
  //clearUser ();
  clear ();

  if (!stop) {
    int loops = 0;
    int placed = 0;

    while (solve_result != Solved) {
      int fi = rand () % SUDOKU_FIELDS;
      char num = '1' + rand () % SUDOKU_SIZE;

      if (getNumber (fi) == 0) {
        placed += 1;
        setPredefinedNumber (fi, num);

        SudokuField::Result result = solve ();

        const char *rc = "";
        if (result == NotSolved)         rc = "NotSolved";
        if (result == Solved)            rc = "Solved";
        if (result == MultipleSolutions) rc = "MultipleSolutions";

        qDebug ("L1(%3d) fi=%2d, num=%c, placed=%2d -> %d (%s)", loops, fi, num, placed, result, rc);

        if (result == NotSolved) {
          setPredefinedNumber (fi, 0);
          --placed;
        }
        //else if ((result == MultipleSolutions) && (free < 60)) {
        else if ((result == MultipleSolutions) && (placed > 8)) {
          break;
        }

        ++loops;
        //if (loops > 1000000) break;
        if (loops > 100) break;
      }
      if (stop) break;
    }
  }

  if (!stop) {
    int loops = 0;
    while (solve_result != Solved) {
      int fi = rand () % SUDOKU_FIELDS;

      if (!isPredefined (fi)) {
        // fix one number
        restoreSolution ();
        setPredefinedNumber (fi, getNumber (fi));
        clearUser ();

        SudokuField::Result result = solve ();

        const char *rc = "";
        if (result == NotSolved)         rc = "NotSolved";
        if (result == Solved)            rc = "Solved";
        if (result == MultipleSolutions) rc = "MultipleSolutions";

        qDebug ("L2(%3d) fi=%2d, -> %d (%s)", loops, fi, result, rc);

        if (result == NotSolved) break;
        //if (result == MultipleSolutions) {
        //  restoreSolution ();
        //}

        ++loops;
        if (loops > 100) break;
      }

      if (stop) break;
    }
  }

  if (!stop) {
    int loops = 0;
  //  bool remove_successful;

  //again:
    //remove_successful = false;

    for (int fi = 0; fi < SUDOKU_FIELDS; ++ fi) {
      if (isPredefined (fi)) {
        char num = getNumber (fi);

        setPredefinedNumber (fi, 0);

        SudokuField::Result result = solve ();

        const char *rc = "";
        if (result == NotSolved)         rc = "NotSolved";
        if (result == Solved)            rc = "Solved";
        if (result == MultipleSolutions) rc = "MultipleSolutions";
        qDebug ("L3(%3d) fi=%2d, -> %d (%s)", loops, fi, result, rc);

        if (result == Solved) {
          //remove_successful = true;
        }
        else setPredefinedNumber (fi, num);

        if (stop) break;
      }
      ++loops;
    }

    //if (!stop && remove_successful) goto again;
  }
} // SudokuField::create


// ****************************************************************************


