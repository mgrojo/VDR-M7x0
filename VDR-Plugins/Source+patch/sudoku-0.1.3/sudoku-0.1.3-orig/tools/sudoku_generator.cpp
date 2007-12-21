/*
 * Sudoku: A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * $Id: sudoku_generator.cpp 11 2005-10-28 01:00:01Z tom $
 */

#include "../puzzle.h"
#include "../solver.h"
#include "../generator.h"
#include <stdio.h>
#include <string.h>
#include <getopt.h>

using namespace Sudoku;


void print_copyleft(unsigned int givens_count)
{
  printf("Sudoku with %d givens generated by sudoku_generator %s\n"
         "  Copyright (C) 2005, Thomas G�nther <tom@toms-cafe.de>\n"
         "     This puzzle can be used without any limitations.\n"
         "\n", givens_count, VERSION);
}

void print_usage()
{
  printf("Usage: sudoku_generator [-n|--non-sym] [-d|--dump] [<givens_count>]\n"
         "         Generate a Sudoku puzzle.\n"
         "           givens_count   Number of givens (<= 81). Default is 36.\n"
         "                          Less than 26 givens takes very long.\n"
         "           -n, --non-sym  Generate a non-symmetric Sudoku puzzle.\n"
         "           -d, --dump     Dump the generated Sudoku (don't print).\n"
         "\n"
         "       sudoku_generator -s|--solve <sudoku_dump>\n"
         "         Solve a Sudoku puzzle.\n"
         "           sudoku_dump    String with 81 * 1-9 or _ (+ ignored).\n"
         "\n"
         "       sudoku_generator -p|--print <sudoku_dump>\n"
         "         Print a Sudoku puzzle.\n"
         "           sudoku_dump    String with 81 * 1-9 or _ (+ ignored).\n"
         "\n"
#ifdef WITH_TEST
         "       sudoku_generator -t|--test\n"
         "         Perform some test procedures.\n"
         "\n"
#endif
         "       sudoku_generator -h|--help\n"
         "         Print this help.\n"
         "\n");
}

void print_sudoku(const Numbers* sudoku_list[], unsigned int count,
                  unsigned int givens_count = 0)
{
  printf("\n");
  for (unsigned int row = 0; row <= DIM; ++row)
  {
    if (row % RDIM == 0)
    {
      for (unsigned int idx = 0; idx < count; ++idx)
      {
        printf("              ");
        for (unsigned int col = 0; col < DIM; ++col)
        {
          if (col % RDIM == 0)
            printf("  ");
          printf("- ");
        }
        printf("  ");
      }
      printf("\n");
    }
    if (row < DIM)
    {
      for (unsigned int idx = 0; idx < count; ++idx)
      {
        printf("              ");
        for (unsigned int col = 0; col < DIM; ++col)
        {
          if (col % RDIM == 0)
            printf("| ");
          unsigned int n = sudoku_list[idx]->get(Pos(col, row));
          if (n)
            printf("%d ", n);
          else
            printf("  ");
        }
        printf("| ");
      }
      printf("\n");
    }
  }
  printf("\n");
  if (givens_count != 0)
    print_copyleft(givens_count);
}

void print_sudoku(const Numbers& sudoku, unsigned int givens_count = 0)
{
  const Numbers* sudoku_list[] = { &sudoku };
  print_sudoku(sudoku_list, 1, givens_count);
}

void print_sudoku(const Numbers& sudoku1, const Numbers& sudoku2,
                  unsigned int givens_count = 0)
{
  const Numbers* sudoku_list[] = { &sudoku1, &sudoku2 };
  print_sudoku(sudoku_list, 2, givens_count);
}

void dump_sudoku(const Numbers& sudoku)
{
  for (unsigned int row = 0; row < DIM; ++row)
  {
    for (unsigned int col = 0; col < DIM; ++col)
    {
      unsigned int n = sudoku.get(Pos(col, row));
      if (n)
        printf("%d", n);
      else
        printf("_");
    }
    if (row < DIM-1)
      printf("+");
  }
  printf("\n");
}

int generate_puzzle(unsigned int givens_count, bool non_sym, bool dump)
{
  Puzzle puzzle;
  Generator generator(puzzle, givens_count, !non_sym);
  generator.find_next_solution();
  if (!generator.solution_is_valid())
    return 1;
  if (dump)
    dump_sudoku(puzzle);
  else
    print_sudoku(puzzle, givens_count);
  return 0;
}

Numbers dump_to_numbers(const char *dump)
{
  Numbers numbers;
  for (Pos p = Pos::first(); *dump && p <= Pos::last(); ++dump)
  {
    if (*dump != '+')
    {
      if (*dump > '0' && *dump - '0' <= DIM)
        numbers.set(p, *dump - '0');
      p = p.next();
    }
  }
  return numbers;
}

int solve_puzzle(const char *dump)
{
  Numbers numbers(dump_to_numbers(dump));
  bool given_marks[SDIM];
  for (Pos p = Pos::first(); p <= Pos::last(); p = p.next())
    given_marks[p] = numbers.get(p) != 0;
  Puzzle puzzle;
  puzzle.set_givens(numbers, given_marks);
  Solver solver(puzzle);
  solver.find_next_solution();
  if (!solver.solution_is_valid())
    return 1;
  print_sudoku(numbers, puzzle);
  solver.find_next_solution();
  if (!solver.solution_is_valid())
    return 0;
  printf("Error: Sudoku has more than one solution!\n");
  return 1;
}

int print_puzzle(const char *dump)
{
  Numbers numbers(dump_to_numbers(dump));
  print_sudoku(numbers);
  return 0;
}

#ifdef WITH_TEST
bool test_search_first()
{
  bool correct = true;
  printf("Search the first Sudoku!\n");
  Puzzle puzzle;
  Solver solver(puzzle);
  solver.find_next_solution();
  if (solver.solution_is_valid())
    print_sudoku(puzzle);
  else
    correct = false, printf("Error: Sudoku is invalid!\n");
  printf("Search the second Sudoku!\n");
  solver.find_next_solution();
  if (solver.solution_is_valid())
    print_sudoku(puzzle);
  else
    correct = false, printf("Error: Sudoku is invalid!\n");
  return correct;
}

bool test_search_random()
{
  bool correct = true;
  printf("Search a random Sudoku!\n");
  Puzzle puzzle;
  Solver solver(puzzle, true);
  solver.find_next_solution();
  if (solver.solution_is_valid())
    print_sudoku(puzzle);
  else
    correct = false, printf("Error: Sudoku is invalid!\n");
  printf("Search another random Sudoku!\n");
  solver.reset();
  solver.find_next_solution();
  if (solver.solution_is_valid())
    print_sudoku(puzzle);
  else
    correct = false, printf("Error: Sudoku is invalid!\n");
  return correct;
}

bool test_generate_symmetric()
{
  bool correct = true;
  printf("Generate a random Sudoku with 36 symmetric givens!\n");
  Puzzle puzzle;
  Generator generator(puzzle, 36);
  generator.find_next_solution();
  if (generator.solution_is_valid())
    print_sudoku(puzzle);
  else
    correct = false, printf("Error: Sudoku is invalid!\n");
  printf("Solve the generated Sudoku!\n");
  Solver solver(puzzle);
  solver.find_next_solution();
  if (solver.solution_is_valid())
    print_sudoku(puzzle);
  else
    correct = false, printf("Error: Sudoku is invalid!\n");
  solver.find_next_solution();
  if (solver.solution_is_valid())
    correct = false, print_sudoku(puzzle),
      printf("Error: Sudoku has more than one solution!\n");
  else
    printf("Sudoku has only one solution!\n");
  printf("\n");
  return correct;
}

bool test_generate_non_symmetric()
{
  bool correct = true;
  printf("Generate a random Sudoku with 26 non-symmetric givens!\n");
  Puzzle puzzle;
  Generator generator(puzzle, 26, false);
  generator.find_next_solution();
  if (generator.solution_is_valid())
    print_sudoku(puzzle);
  else
    correct = false, printf("Error: Sudoku is invalid!\n");
  printf("Solve the generated Sudoku!\n");
  Solver solver(puzzle);
  solver.find_next_solution();
  if (solver.solution_is_valid())
    print_sudoku(puzzle);
  else
    correct = false, printf("Error: Sudoku is invalid!\n");
  solver.find_next_solution();
  if (solver.solution_is_valid())
    correct = false, print_sudoku(puzzle),
      printf("Error: Sudoku has more than one solution!\n");
  else
    printf("Sudoku has only one solution!\n");
  printf("\n");
  return correct;
}

int test_sudoku()
{
  unsigned int count = 0;
  unsigned int error = 0;
  ++count;
  if (!test_search_first())
    ++error;
  ++count;
  if (!test_search_random())
    ++error;
  ++count;
  if (!test_generate_symmetric())
    ++error;
  ++count;
  if (!test_generate_non_symmetric())
    ++error;
  if (error > 0)
    printf("%d errors in %d tests\n", error, count);
  else
    printf("All %d tests correct\n", count);
  if (error > 0)
    return 1;
  return 0;
}
#endif // WITH_TEST

int main(int argc, char* argv[])
{
  static const struct option long_options[] =
  {
    { "non-sym", no_argument, NULL, 'n' },
    { "dump",    no_argument, NULL, 'd' },
    { "solve",   no_argument, NULL, 's' },
    { "print",   no_argument, NULL, 'p' },
#ifdef WITH_TEST
    { "test",    no_argument, NULL, 't' },
#endif
    { "help",    no_argument, NULL, 'h' },
    { NULL }
  };
#ifdef WITH_TEST
  static const char* options = "ndspth";
#else
  static const char* options = "ndsph";
#endif
  bool non_sym = false;
  bool dump    = false;
  bool solve   = false;
  bool print   = false;
  bool test    = false;
  bool help    = false;
  bool error   = false;
  int c;
  while ((c = getopt_long(argc, argv, options, long_options, NULL)) != -1)
  {
    switch (c)
    {
      case 'n': non_sym = true; break;
      case 'd': dump    = true; break;
      case 's': solve   = true; break;
      case 'p': print   = true; break;
#ifdef WITH_TEST
      case 't': test    = true; break;
#endif
      case 'h': help    = true; break;
      default:  error   = true;
    }
  }
  int arg_count = argc - optind;

  unsigned int givens_count = 36;
  if ((arg_count == 0 ||
       (arg_count == 1 && sscanf(argv[optind], "%u", &givens_count) == 1)) &&
      givens_count > 0 && givens_count <= SDIM &&
      !solve && !print && !test && !help && !error)
    return generate_puzzle(givens_count, non_sym, dump);

  if (solve && arg_count == 1 && strlen(argv[optind]) >= SDIM &&
      !non_sym && !dump && !test && !help && !error)
    return solve_puzzle(argv[optind]);

  if (print && arg_count == 1 && strlen(argv[optind]) >= SDIM &&
      !non_sym && !dump && !test && !help && !error)
    return print_puzzle(argv[optind]);

#ifdef WITH_TEST
  if (test && arg_count == 0 &&
      !non_sym && !dump && !print && !help && !error)
    return test_sudoku();
#endif

  print_usage();
  return 2;
}
