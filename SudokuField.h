// SudokuField.h: Sudoku-Game, Michael Eisele, 27.02.2006

#ifndef SUDOKU_FIELD_H
#define SUDOKU_FIELD_H

// ****************************************************************************

#define SUDOKU_SIZE             9
#define SUDOKU_BSIZE            3
#define SUDOKU_WIDTH            (SUDOKU_SIZE)
#define SUDOKU_HEIGHT           (SUDOKU_SIZE)
#define SUDOKU_FIELDS           (SUDOKU_SIZE*SUDOKU_SIZE)
#define SUDOKU_FIELD_MAX        (SUDOKU_FIELDS-1)

// ****************************************************************************

class SudokuField {
      public:
        enum Result {
          Unkown,
          NotSolved,
          Solved,
          MultipleSolutions,
        };

      private:
        // Field data
        unsigned int    field [SUDOKU_FIELDS];
        unsigned int    solution_field [SUDOKU_FIELDS];

        // Board status
        bool            wrong,  // no solution possible
                        solved, // solution found
                        easy,   // easy move possible
                        stop;   // stop solver&creator

         int            free;   // free fields
        // Result of solver
        Result          solve_result; // result from previous call to solve

        SudokuField     *saved_solution;

        int             backtracking_count;

      private:
        void reduce ();

      public:
        SudokuField ();
        ~SudokuField ();

        void setStop (bool s);

        void clear ();
        void clearUser ();

        void saveSolution ();
        void restoreSolution ();

        bool setupField (const char *init_string);
        void getFieldConfigString (char *config_string); // string size must be at least SUDOKU_FIELDS + 1, i.e. 9*9+1 characters

        char getNumber (int field_index) const;
        unsigned int getNumberMask (int field_index) const;

        bool hasOnePossibleValue (int field_index) const;

        bool isEmpty (int field_index) const;
        bool isWrong (int field_index) const;
        bool isPredefined (int field_index) const;

        void setNumber (int field_index, char number);
        void setPredefinedNumber (int field_index, char number);

        // Solver, checker & status update
        Result solve (int field_index = 0);
        void check (); // check the board and set the board status
        void create (); // create a new game

        int getFree () const { return free; }
        bool isWrong () const { return wrong; }
        bool isSolved () const { return solved; }
        bool isEasy () const { return easy; }
        Result solveResult () const { return solve_result; }

        void clearBacktrackingCount () { backtracking_count = 0; }
        int getBacktrackingCount () const { return backtracking_count; }
      };


// ****************************************************************************

#endif // SUDOKU_FIELD_H

