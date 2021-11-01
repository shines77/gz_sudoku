
#ifndef GZ_SUDOKU_TEST_CASE_H
#define GZ_SUDOKU_TEST_CASE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

struct SudokuBoard {
    const char * rows[9];
};

static const SudokuBoard test_case[] = {
    //
    // 0 #
    //
    // Normal (filled = 30)
    // https://leetcode-cn.com/problems/sudoku-solver/
    //
    {
        "53. | .7. | ...",
        "6.. | 195 | ...",
        ".98 | ... | .6.",

        "8.. | .6. | ..3",
        "4.. | 8.3 | ..1",
        "7.. | .2. | ..6",

        ".6. | ... | 28.",
        "... | 419 | ..5",
        "... | .8. | .79",
    },

    //
    // 1 #
    //
    // Advance (filled = 24)
    // https://www.sudoku-cn.com/
    //
    {
        "4.2 | ... | 9..",
        "... | .61 | ...",
        ".19 | ... | ...",

        "7.5 | ... | 6..",
        "2.4 | 7.. | ..5",
        "... | .9. | 7..",

        ".8. | 2.9 | .1.",
        "..7 | ..4 | ...",
        "... | ... | .52",
    },

    //
    // 2 #
    //
    // Hard (filled = 21)
    // https://github.com/tropicalwzc/ice_sudoku.github.io/blob/master/sudoku_solver.c
    //
    {
        "5.. | ... | 3..",
        ".2. | 1.. | .7.",
        "..8 | ... | ..9",

        ".4. | ..7 | ...",
        "... | 821 | ...",
        "... | 6.. | .1.",

        "3.. | ... | 8..",
        ".6. | ..4 | .2.",
        "..9 | ... | ..5",
    },

    // Ice sudoku string: 500000300020100070008000009040007000000821000000600010300000800060004020009000005

    //
    // 3 #
    //
    // Hardcore (filled = 17)
    // http://www.cn.sudokupuzzle.org/play.php
    //
    {
        "5.. | ... | ...",
        ".1. | ... | 32.",
        "... | 84. | ...",

        "... | ... | ...",
        "... | ..3 | 1..",
        "6.8 | 5.. | ...",

        "..7 | ... | .68",
        ".34 | ..1 | ...",
        "... | ... | ...",
    },

    // Ice sudoku string: 500000000010000320000840000000000000000003100608500000007000068034001000000000000

    //
    // 4 #
    //
    // Hardcore (filled = 21)
    // http://news.sohu.com/20130527/n377158555.shtml
    // https://baike.baidu.com/reference/13848819/1bf4HJzRCPCNz9Rypz3HpTtnhc2MpcRr5JMIp0032uiuKPQm4eOMuq2WZWxf77V-UBRjIkyDf9CVZDEjlDeHJBaazlstk30qaDtt
    //
    {
        "8.. | ... | ...",
        "..3 | 6.. | ...",
        ".7. | .9. | 2..",

        ".5. | ..7 | ...",
        "... | .45 | 7..",
        "... | 1.. | .3.",

        "..1 | ... | .68",
        "..8 | 5.. | .1.",
        ".9. | ... | 4..",
    },


    //
    // 5 #, copy from # 4
    //
    // Hardcore (filled = 20)
    //
    {
        "8.. | ... | ...",
        "..3 | 6.. | ...",
        ".7. | .9. | 2..",

        ".5. | ..7 | ...",
        "... | .45 | 7..",
        "... | 1.. | .3.",

        "..1 | ... | ..8",
        "..8 | 5.. | .1.",
        ".9. | ... | 4..",
    },

    //
    // 6 #, copy from /puzzles/sudoku17.txt
    //
    // Hardcore (filled = 16)
    //
    {
        "... | ... | .13",
        ".2. | 5.. | ...",
        "... | ... | ...",
        "1.3 | ... | .7.",
        "... | 8.2 | ...",
        "..4 | ... | ...",
        "... | .4. | 5..",
        "67. | ... | 2..",
        "... | .1. | ...",
    },

    /*************************************************

    // Empty board format (For user custom and copy)
    {
        "... | ... | ...",
        "... | ... | ...",
        "... | ... | ...",

        "... | ... | ...",
        "... | ... | ...",
        "... | ... | ...",

        "... | ... | ...",
        "... | ... | ...",
        "... | ... | ...",
    },

    **************************************************/
};

#endif // GZ_SUDOKU_TEST_CASE_H
