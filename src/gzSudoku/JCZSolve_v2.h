
#ifndef GZ_SUDOKU_JCZSOLVE_V2_H
#define GZ_SUDOKU_JCZSOLVE_V2_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <memory.h>
#include <assert.h>

#include <cstdint>
#include <cstddef>
#include <cstring>      // For std::memset(), std::memcpy()
#include <vector>
#include <bitset>
#include <array>        // For std::array<T, Size>

#include "BasicSolver.h"
#include "Sudoku.h"
#include "StopWatch.h"
#include "BitUtils.h"
#include "BitSet.h"
#include "PackedBitSet.h"
#include "BitArray.h"
#include "BitVec.h"

#define JCZ_V2_ENABLE_R1_COUNT     0

namespace gzSudoku {
namespace JCZ {
namespace v2 {

static const size_t kSearchMode = SearchMode::OneAnswer;

// Kill all in other blocks locked column / box
static const uint32_t colLockedSingleMaskTbl[512] = {
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07767767767, 07766766766, 07765765765, 07767767767, 07763763763, 07767767767, 07767767767, 07767767767,
    07757757757, 07756756756, 07755755755, 07757757757, 07753753753, 07757757757, 07757757757, 07757757757,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07737737737, 07736736736, 07735735735, 07737737737, 07733733733, 07737737737, 07737737737, 07737737737,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07677677677, 07676676676, 07675675675, 07677677677, 07673673673, 07677677677, 07677677677, 07677677677,
    07667667667, 07666666666, 07665665665, 07667667667, 07663663663, 07667667667, 07667667667, 07667667667,
    07657657657, 07656656656, 07655655655, 07657657657, 07653653653, 07657657657, 07657657657, 07657657657,
    07677677677, 07676676676, 07675675675, 07677677677, 07673673673, 07677677677, 07677677677, 07677677677,
    07637637637, 07636636636, 07635635635, 07637637637, 07633633633, 07637637637, 07637637637, 07637637637,
    07677677677, 07676676676, 07675675675, 07677677677, 07673673673, 07677677677, 07677677677, 07677677677,
    07677677677, 07676676676, 07675675675, 07677677677, 07673673673, 07677677677, 07677677677, 07677677677,
    07677677677, 07676676676, 07675675675, 07677677677, 07673673673, 07677677677, 07677677677, 07677677677,
    07577577577, 07576576576, 07575575575, 07577577577, 07573573573, 07577577577, 07577577577, 07577577577,
    07567567567, 07566566566, 07565565565, 07567567567, 07563563563, 07567567567, 07567567567, 07567567567,
    07557557557, 07556556556, 07555555555, 07557557557, 07553553553, 07557557557, 07557557557, 07557557557,
    07577577577, 07576576576, 07575575575, 07577577577, 07573573573, 07577577577, 07577577577, 07577577577,
    07537537537, 07536536536, 07535535535, 07537537537, 07533533533, 07537537537, 07537537537, 07537537537,
    07577577577, 07576576576, 07575575575, 07577577577, 07573573573, 07577577577, 07577577577, 07577577577,
    07577577577, 07576576576, 07575575575, 07577577577, 07573573573, 07577577577, 07577577577, 07577577577,
    07577577577, 07576576576, 07575575575, 07577577577, 07573573573, 07577577577, 07577577577, 07577577577,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07767767767, 07766766766, 07765765765, 07767767767, 07763763763, 07767767767, 07767767767, 07767767767,
    07757757757, 07756756756, 07755755755, 07757757757, 07753753753, 07757757757, 07757757757, 07757757757,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07737737737, 07736736736, 07735735735, 07737737737, 07733733733, 07737737737, 07737737737, 07737737737,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07377377377, 07376376376, 07375375375, 07377377377, 07373373373, 07377377377, 07377377377, 07377377377,
    07367367367, 07366366366, 07365365365, 07367367367, 07363363363, 07367367367, 07367367367, 07367367367,
    07357357357, 07356356356, 07355355355, 07357357357, 07353353353, 07357357357, 07357357357, 07357357357,
    07377377377, 07376376376, 07375375375, 07377377377, 07373373373, 07377377377, 07377377377, 07377377377,
    07337337337, 07336336336, 07335335335, 07337337337, 07333333333, 07337337337, 07337337337, 07337337337,
    07377377377, 07376376376, 07375375375, 07377377377, 07373373373, 07377377377, 07377377377, 07377377377,
    07377377377, 07376376376, 07375375375, 07377377377, 07373373373, 07377377377, 07377377377, 07377377377,
    07377377377, 07376376376, 07375375375, 07377377377, 07373373373, 07377377377, 07377377377, 07377377377,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07767767767, 07766766766, 07765765765, 07767767767, 07763763763, 07767767767, 07767767767, 07767767767,
    07757757757, 07756756756, 07755755755, 07757757757, 07753753753, 07757757757, 07757757757, 07757757757,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07737737737, 07736736736, 07735735735, 07737737737, 07733733733, 07737737737, 07737737737, 07737737737,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07767767767, 07766766766, 07765765765, 07767767767, 07763763763, 07767767767, 07767767767, 07767767767,
    07757757757, 07756756756, 07755755755, 07757757757, 07753753753, 07757757757, 07757757757, 07757757757,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07737737737, 07736736736, 07735735735, 07737737737, 07733733733, 07737737737, 07737737737, 07737737737,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07767767767, 07766766766, 07765765765, 07767767767, 07763763763, 07767767767, 07767767767, 07767767767,
    07757757757, 07756756756, 07755755755, 07757757757, 07753753753, 07757757757, 07757757757, 07757757757,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07737737737, 07736736736, 07735735735, 07737737737, 07733733733, 07737737737, 07737737737, 07737737737,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777,
    07777777777, 07776776776, 07775775775, 07777777777, 07773773773, 07777777777, 07777777777, 07777777777
};

// Triads mask per row
static const uint32_t rowTriadsMaskTbl[512] = {
    0, 1, 1, 1, 1, 1, 1, 1, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3,
    2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3,
    4, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    4, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    4, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    4, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    4, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    4, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    4, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7
};

// Keep all locked candidates
static const uint32_t keepLockedCandidatesTbl[512] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 07007070700, 07707070700, 07007770700, 07707770700,
    0, 0, 0, 0, 07077070700, 07777070700, 07777770700, 07777770700,
    0, 0, 07007700070, 07077700070, 0, 0, 07007770070, 07077770070,
    0, 0, 07707700070, 07777700070, 0, 0, 07777770070, 07777770070,
    0, 0, 07007700770, 07777700770, 07007070770, 07777070770, 07007770770, 07777770770,
    0, 0, 07707700770, 07777700770, 07077070770, 07777070770, 07777770770, 07777770770,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 07070007700, 07070707700, 07770007700, 07770707700,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 07077007700, 07777707700, 07777007700, 07777707700,
    0, 07070700007, 0, 07077700007, 0, 07070707007, 0, 07077707007,
    0, 07070700707, 0, 07777700707, 07070007707, 07070707707, 07777007707, 07777707707,
    0, 07770700007, 0, 07777700007, 0, 07777707007, 0, 07777707007,
    0, 07770700707, 0, 07777700707, 07077007707, 07777707707, 07777007707, 07777707707,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 07070077700, 07070777700, 07770777700, 07770777700,
    0, 0, 0, 0, 07007077700, 07707777700, 07007777700, 07707777700,
    0, 0, 0, 0, 07077077700, 07777777700, 07777777700, 07777777700,
    0, 07070700077, 07007700077, 07077700077, 0, 07070777077, 07007777077, 07077777077,
    0, 07070700777, 07707700777, 07777700777, 07070077777, 07070777777, 07777777777, 07777777777,
    0, 07770700777, 07007700777, 07777700777, 07007077777, 07777777777, 07007777777, 07777777777,
    0, 07770700777, 07707700777, 07777700777, 07077077777, 07777777777, 07777777777, 07777777777,
    0, 0, 0, 0, 0, 0, 0, 0,
    00, 0, 07700007070, 07700077070, 0, 0, 07770007070, 07770077070,
    00, 07700070007, 0, 07700077007, 0, 07707070007, 0, 07707077007,
    00, 07700070077, 07700007077, 07700077077, 0, 07777070077, 07777007077, 07777077077,
    00, 0, 0, 0, 0, 0, 0, 0,
    00, 0, 07707007070, 07777077070, 0, 0, 07777007070, 07777077070,
    00, 07770070007, 0, 07777077007, 0, 07777070007, 0, 07777077007,
    00, 07770070077, 07707007077, 07777077077, 0, 07777070077, 07777007077, 07777077077,
    00, 0, 0, 0, 0, 0, 0, 0,
    00, 0, 07700707070, 07700777070, 0, 0, 07770777070, 07770777070,
    00, 07700070707, 0, 07700777707, 07007070707, 07707070707, 07007777707, 07707777707,
    00, 07700070777, 07700707777, 07700777777, 07077070777, 07777070777, 07777777777, 07777777777,
    00, 0, 07007707070, 07077777070, 0, 0, 07007777070, 07077777070,
    00, 0, 07707707070, 07777777070, 0, 0, 07777777070, 07777777070,
    00, 07770070777, 07007707777, 07777777777, 07007070777, 07777070777, 07007777777, 07777777777,
    00, 07770070777, 07707707777, 07777777777, 07077070777, 07777070777, 07777777777, 07777777777,
    00, 0, 0, 0, 0, 0, 0, 0,
    00, 0, 07700007770, 07700777770, 07070007770, 07070777770, 07770007770, 07770777770,
    00, 07700770007, 0, 07700777007, 0, 07707777007, 0, 07707777007,
    00, 07700770777, 07700007777, 07700777777, 07077007777, 07777777777, 07777007777, 07777777777,
    00, 07070770007, 0, 07077777007, 0, 07070777007, 0, 07077777007,
    00, 07070770777, 07707007777, 07777777777, 07070007777, 07070777777, 07777007777, 07777777777,
    00, 07770770007, 0, 07777777007, 0, 07777777007, 0, 07777777007,
    00, 07770770777, 07707007777, 07777777777, 07077007777, 07777777777, 07777007777, 07777777777,
    00, 0, 0, 0, 0, 0, 0, 0,
    00, 0, 07700707770, 07700777770, 07070077770, 07070777770, 07770777770, 07770777770,
    00, 07700770707, 0, 07700777707, 07007077707, 07707777707, 07007777707, 07707777707,
    00, 07700770777, 07700707777, 07700777777, 07077077777, 07777777777, 07777777777, 07777777777,
    00, 07070770077, 07007707077, 07077777077, 0, 07070777077, 07007777077, 07077777077,
    00, 07070770777, 07707707777, 07777777777, 07070077777, 07070777777, 07777777777, 07777777777,
    00, 07770770777, 07007707777, 07777777777, 07007077777, 07777777777, 07007777777, 07777777777,
    00, 07770770777, 07707707777, 07777777777, 07077077777, 07777777777, 07777777777, 07777777777
};

// Get the masks of all row triads is single
static const uint32_t rowTriadsSingleMaskTbl[512] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0124, 0124, 0124, 0124, 0, 0, 0, 0, 0124, 0124, 0124, 0124,
    0, 0, 0142, 0142, 0, 0, 0142, 0142, 0, 0, 0142, 0142, 0, 0, 0142, 0142, 0, 0, 0142, 0142, 0124, 0124, 0100, 0100, 0, 0, 0142, 0142, 0124, 0124, 0100, 0100,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0214, 0214, 0214, 0214, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0214, 0214, 0214, 0214,
    0, 0241, 0, 0241, 0, 0241, 0, 0241, 0, 0241, 0, 0241, 0214, 0200, 0214, 0200, 0, 0241, 0, 0241, 0, 0241, 0, 0241, 0, 0241, 0, 0241, 0214, 0200, 0214, 0200,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0214, 0214, 0214, 0214, 0, 0, 0, 0, 0124, 0124, 0124, 0124, 0, 0, 0, 0, 04, 04, 04, 04,
    0, 0241, 0142, 040, 0, 0241, 0142, 040, 0, 0241, 0142, 040, 0214, 0200, 0, 0, 0, 0241, 0142, 040, 0124, 0, 0100, 0, 0, 0241, 0142, 040, 04, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0412, 0412, 0, 0, 0412, 0412, 0, 0421, 0, 0421, 0, 0421, 0, 0421, 0, 0421, 0412, 0400, 0, 0421, 0412, 0400,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0412, 0412, 0, 0, 0412, 0412, 0, 0421, 0, 0421, 0, 0421, 0, 0421, 0, 0421, 0412, 0400, 0, 0421, 0412, 0400,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0412, 0412, 0, 0, 0412, 0412, 0, 0421, 0, 0421, 0124, 020, 0124, 020, 0, 0421, 0412, 0400, 0124, 020, 0, 0,
    0, 0, 0142, 0142, 0, 0, 0142, 0142, 0, 0, 02, 02, 0, 0, 02, 02, 0, 0421, 0142, 0, 0124, 020, 0100, 0, 0, 0421, 02, 0, 0124, 020, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0412, 0412, 0214, 0214, 010, 010, 0, 0421, 0, 0421, 0, 0421, 0, 0421, 0, 0421, 0412, 0400, 0214, 0, 010, 0,
    0, 0241, 0, 0241, 0, 0241, 0, 0241, 0, 0241, 0412, 0, 0214, 0200, 010, 0, 0, 01, 0, 01, 0, 01, 0, 01, 0, 01, 0412, 0, 0214, 0, 010, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0412, 0412, 0214, 0214, 010, 010, 0, 0421, 0, 0421, 0124, 020, 0124, 020, 0, 0421, 0412, 0400, 04, 0, 0, 0,
    0, 0241, 0142, 040, 0, 0241, 0142, 040, 0, 0241, 02, 0, 0214, 0200, 0, 0, 0, 01, 0142, 0, 0124, 0, 0100, 0, 0, 01, 02, 0, 04, 0, 0, 0
};

// Get the masks of combine all columns in band is single
static const uint32_t combColumnSingleMaskTbl[512] = {
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
    00, 00, 00, 00, 00, 00, 00, 00, 00, 0777, 0777, 0666, 0777, 0666, 0666, 0666,
    00, 0777, 0777, 0666, 0777, 0666, 0666, 0666, 00, 0555, 0555, 0444, 0555, 0444, 0444, 0444,
    00, 0777, 0777, 0666, 0777, 0666, 0666, 0666, 00, 0555, 0555, 0444, 0555, 0444, 0444, 0444,
    00, 0555, 0555, 0444, 0555, 0444, 0444, 0444, 00, 0555, 0555, 0444, 0555, 0444, 0444, 0444,
    00, 00, 00, 00, 00, 00, 00, 00, 00, 0777, 0777, 0666, 0777, 0666, 0666, 0666,
    00, 0777, 0777, 0666, 0777, 0666, 0666, 0666, 00, 0555, 0555, 0444, 0555, 0444, 0444, 0444,
    00, 0777, 0777, 0666, 0777, 0666, 0666, 0666, 00, 0555, 0555, 0444, 0555, 0444, 0444, 0444,
    00, 0555, 0555, 0444, 0555, 0444, 0444, 0444, 00, 0555, 0555, 0444, 0555, 0444, 0444, 0444,
    00, 00, 00, 00, 00, 00, 00, 00, 00, 0333, 0333, 0222, 0333, 0222, 0222, 0222,
    00, 0333, 0333, 0222, 0333, 0222, 0222, 0222, 00, 0111, 0111, 00, 0111, 00, 00, 00,
    00, 0333, 0333, 0222, 0333, 0222, 0222, 0222, 00, 0111, 0111, 00, 0111, 00, 00, 00,
    00, 0111, 0111, 00, 0111, 00, 00, 00, 00, 0111, 0111, 00, 0111, 00, 00, 00,
    00, 00, 00, 00, 00, 00, 00, 00, 00, 0777, 0777, 0666, 0777, 0666, 0666, 0666,
    00, 0777, 0777, 0666, 0777, 0666, 0666, 0666, 00, 0555, 0555, 0444, 0555, 0444, 0444, 0444,
    00, 0777, 0777, 0666, 0777, 0666, 0666, 0666, 00, 0555, 0555, 0444, 0555, 0444, 0444, 0444,
    00, 0555, 0555, 0444, 0555, 0444, 0444, 0444, 00, 0555, 0555, 0444, 0555, 0444, 0444, 0444,
    00, 00, 00, 00, 00, 00, 00, 00, 00, 0333, 0333, 0222, 0333, 0222, 0222, 0222,
    00, 0333, 0333, 0222, 0333, 0222, 0222, 0222, 00, 0111, 0111, 00, 0111, 00, 00, 00,
    00, 0333, 0333, 0222, 0333, 0222, 0222, 0222, 00, 0111, 0111, 00, 0111, 00, 00, 00,
    00, 0111, 0111, 00, 0111, 00, 00, 00, 00, 0111, 0111, 00, 0111, 00, 00, 00,
    00, 00, 00, 00, 00, 00, 00, 00, 00, 0333, 0333, 0222, 0333, 0222, 0222, 0222,
    00, 0333, 0333, 0222, 0333, 0222, 0222, 0222, 00, 0111, 0111, 00, 0111, 00, 00, 00,
    00, 0333, 0333, 0222, 0333, 0222, 0222, 0222, 00, 0111, 0111, 00, 0111, 00, 00, 00,
    00, 0111, 0111, 00, 0111, 00, 00, 00, 00, 0111, 0111, 00, 0111, 00, 00, 00,
    00, 00, 00, 00, 00, 00, 00, 00, 00, 0333, 0333, 0222, 0333, 0222, 0222, 0222,
    00, 0333, 0333, 0222, 0333, 0222, 0222, 0222, 00, 0111, 0111, 00, 0111, 00, 00, 00,
    00, 0333, 0333, 0222, 0333, 0222, 0222, 0222, 00, 0111, 0111, 00, 0111, 00, 00, 00,
    00, 0111, 0111, 00, 0111, 00, 00, 00, 00, 0111, 0111, 00, 0111, 00, 00, 00
};

// Hidden single in row, 1 indicate row is a hidden single, mode: 1 to 111
static const uint32_t rowHiddenSingleMaskTbl[512] = {
    0, 1, 1, 1, 1, 1, 1, 1, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3,
    2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3,
    4, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    4, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    4, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    4, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    4, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    4, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    4, 5, 5, 5, 5, 5, 5, 5, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
    6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7, 6, 7, 7, 7, 7, 7, 7, 7,
};

// Hidden single in row, 1 indicate row is not a hidden single, mode: 1 to 111
static const int rowHiddenSingleReverseMaskTbl[512] = {
    7, 6, 6, 6, 6, 6, 6, 6, 5, 4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4,
    5, 4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4, 5, 4, 4, 4, 4, 4, 4, 4,
    3, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    3, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    3, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    3, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    3, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    3, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    3, 2, 2, 2, 2, 2, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0
};

// Rows where single found  000 to 111
static const unsigned int solvedRowsBitMaskTbl[8] = {
    00, 0777, 0777000, 0777777, 0777000000, 0777000777, 0777777000, 0777777777
};

// Rows where single found  000 to 111
static const unsigned int solvedRowsReverseBitMaskTbl[8] = {
    0777777777, 0777777000, 0777000777, 0777000000, 0777777, 0777000, 0777, 00
};

static const int8_t bandBitPosToPos64[2][64] = {
    // bit64[0]
    {
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, -1, -1, -1, -1, -1,
        27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
        43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, -1, -1, -1, -1, -1
    },
        
    // bit64[1]
    {
        54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
        70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    }
};

static const int8_t bandBitPosToPos32[4][32] = {
    // bit32[0]
    {
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, -1, -1, -1, -1, -1
    },

    // bit32[1]
    {
        27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
        43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, -1, -1, -1, -1, -1
    },
        
    // bit32[2]
    {
        54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69,
        70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, -1, -1, -1, -1, -1
    },

    // bit32[3]
    {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
    }
};

class Solver : public BasicSolver {
public:
    typedef BasicSolver                         basic_solver_t;
    typedef Solver                              this_type;

    typedef typename Sudoku::NeighborCells      NeighborCells;
    typedef typename Sudoku::CellInfo           CellInfo;
    typedef typename Sudoku::BoxesInfo          BoxesInfo;

    typedef typename Sudoku::BitMask            BitMask;
    typedef typename Sudoku::BitMaskTable       BitMaskTable;

    static const size_t kAlignment = Sudoku::kAlignment;
    static const size_t BoxCellsX = Sudoku::kBoxCellsX;      // 3
    static const size_t BoxCellsY = Sudoku::kBoxCellsY;      // 3
    static const size_t BoxCountX = Sudoku::kBoxCountX;      // 3
    static const size_t BoxCountY = Sudoku::kBoxCountY;      // 3
    static const size_t MinNumber = Sudoku::kMinNumber;      // 1
    static const size_t MaxNumber = Sudoku::kMaxNumber;      // 9

    static const size_t Rows = Sudoku::kRows;
    static const size_t Cols = Sudoku::kCols;
    static const size_t Boxes = Sudoku::kBoxes;
    static const size_t BoxSize = Sudoku::kBoxSize;
    static const size_t Numbers = Sudoku::kNumbers;

    static const size_t BoardSize = Sudoku::kBoardSize;
    static const size_t TotalSize = Sudoku::kTotalSize;
    static const size_t Neighbors = Sudoku::kNeighbors;

    static const size_t Rows16 = Sudoku::kRows16;
    static const size_t Cols16 = Sudoku::kCols16;
    static const size_t Numbers10 = Sudoku::kNumbers10;
    static const size_t Numbers16 = Sudoku::kNumbers16;
    static const size_t Boxes16 = Sudoku::kBoxes16;
    static const size_t BoxSize16 = Sudoku::kBoxSize16;
    static const size_t BoardSize16 = Sudoku::kBoardSize16;

    static const size_t kAllRowBits = Sudoku::kAllRowBits;
    static const size_t kAllColBits = Sudoku::kAllColBits;
    static const size_t kAllBoxBits = Sudoku::kAllBoxBits;
    static const size_t kAllBoxCellBits = Sudoku::kAllBoxCellBits;
    static const size_t kAllNumberBits = Sudoku::kAllNumberBits;

    static const bool kAllDimIsSame = Sudoku::kAllDimIsSame;

    // all pencil marks set - 27 bits per band
    static const uint32_t kBitSet27     = 0x07FFFFFFUL;
    static const uint64_t kBitSet27_64  = 0x07FFFFFF07FFFFFFULL;

    static const uint32_t kFullRowBits   = 0x01FFUL;
    static const uint32_t kFullRowBits_1 = 0x01FFUL << 9U;
    static const uint32_t kFullRowBits_2 = 0x01FFUL << 18U;

private:
    enum LiteralType {
        NumRowCols,
        NumColRows,
        NumBoxCells,
        BoxCellNums,
        Unknown
    };

#pragma pack(push, 1)

    union alignas(16) BandBoard {
        uint32_t bands[4];
        uint64_t bands64[2];

        void reset() {
            this->bands64[0] = 0;
            this->bands64[1] = 0;
        }

        void set() {
            this->bands64[0] = kBitSet27_64;
            this->bands64[1] = kBitSet27;
        }
    };

    struct alignas(32) State {
        BandBoard candidates[Numbers10];
        BandBoard prevCandidates[Numbers10];
        BandBoard colCombBits[Numbers10];
        BandBoard solvedCells;
        BandBoard solvedRows;
        BandBoard pairs;
        BandBoard reserve;

        void init() {
            for (size_t num = 0; num < Numbers10; num++) {
                this->candidates[num].set();
                this->prevCandidates[num].reset();
                this->colCombBits[num].reset();
            }            
            this->solvedCells.reset();
            this->solvedRows.reset();
            this->pairs.reset();
        }
    };

    struct alignas(32) InitState {
        PackedBitSet3D<Numbers, Rows16, Cols16>     num_row_cols;     // [num][row][col]
        PackedBitSet3D<Numbers, Cols16, Rows16>     num_col_rows;     // [num][col][row]
        PackedBitSet3D<Numbers, Boxes16, BoxSize16> num_box_cells;    // [num][box][cell]

        PackedBitSet2D<Rows16, Cols16>              row_solved;
        PackedBitSet2D<Cols16, Rows16>              col_solved;
        PackedBitSet2D<Boxes16, BoxSize16>          box_solved;
    };

    union LiteralInfoEx {
        struct {
            uint32_t literal_size;
            uint16_t literal_type;
            uint16_t literal_index;
        };

        uint64_t value;

        LiteralInfoEx(uint64_t _value = 0) : value(_value) {}
        LiteralInfoEx(uint32_t size, uint32_t type, uint32_t index)
            : literal_size(size), literal_type((uint16_t)type), literal_index((uint16_t)index) {}
        LiteralInfoEx(const LiteralInfoEx & src) : value(src.value) {}

        LiteralInfoEx & operator = (const LiteralInfoEx & rhs) {
            this->value = rhs.value;
            return *this;
        }

        bool isValid() const {
            return (this->value != uint64_t(-1));
        }
    };

    union LiteralInfo {
        struct {
            uint32_t literal_type;
            uint32_t literal_index;
        };

        uint64_t value;

        LiteralInfo(uint64_t _value = 0) : value(_value) {}
        LiteralInfo(uint32_t type, uint32_t index)
            : literal_type(type), literal_index(index) {}
        LiteralInfo(const LiteralInfo & src) : value(src.value) {}

        LiteralInfo & operator = (const LiteralInfo & rhs) {
            this->value = rhs.value;
            return *this;
        }

        bool isValid() const {
            return (this->value != uint64_t(-1));
        }

        LiteralInfoEx toLiteralInfoEx(uint32_t literal_size) {
            return LiteralInfoEx(literal_size, literal_type, literal_index);
        }
    };

    template <size_t nBoxCountX, size_t nBoxCountY>
    struct PeerBoxes {
        static const uint32_t kBoxesCount = (uint32_t)((nBoxCountX - 1) + (nBoxCountY - 1));

        uint32_t boxes_count() const { return kBoxesCount; }

        int boxes[kBoxesCount];
    };

    typedef PeerBoxes<BoxCountX, BoxCountY>     peer_boxes_t;

    struct alignas(32) StaticData {
        BandBoard flip_mask[BoardSize + 1];
        BandBoard fill_mask[BoardSize + 1];

        PackedBitSet3D<BoardSize, Rows16, Cols16>   num_row_mask;
        PackedBitSet3D<BoardSize, Rows16, Cols16>   row_fill_mask;
        
        peer_boxes_t    peer_boxes[Boxes];
        bool            mask_is_inited;

        StaticData() : mask_is_inited(false) {
            if (!Static.mask_is_inited) {
                Sudoku::initialize();
                this_type::init_mask();
                Static.mask_is_inited = true;
            }
        }
    };

#pragma pack(pop)

    State       state_;
    InitState   init_state_;

    static StaticData Static;

public:
    Solver() : basic_solver_t() {
    }
    ~Solver() {}

private:
    static void init_peer_boxes() {
        for (size_t box_y = 0; box_y < BoxCellsY; box_y++) {
            size_t box_y_base = box_y * BoxCellsX;
            for (size_t box_x = 0; box_x < BoxCellsX; box_x++) {
                uint32_t box = uint32_t(box_y_base + box_x);
                size_t index = 0;
                peer_boxes_t peerBoxes;
                //peerBoxes.boxes[index++] = box;
                for (size_t box_i = 0; box_i < BoxCellsX; box_i++) {
                    if (box_i != box_x) {
                        peerBoxes.boxes[index++] = uint32_t(box_y * BoxCellsX + box_i);
                    }
                }
                for (size_t box_j = 0; box_j < BoxCellsY; box_j++) {
                    if (box_j != box_y) {
                        peerBoxes.boxes[index++] = uint32_t(box_j * BoxCellsX + box_x);
                    }
                }
                assert(index == peerBoxes.boxes_count());

                //std::sort(&peerBoxes.boxes[1], &peerBoxes.boxes[peerBoxes.boxes_count()]);
                Static.peer_boxes[box] = peerBoxes;
            }
        }
    }

    static void make_flip_mask(size_t fill_pos, size_t row, size_t col) {
        PackedBitSet2D<Rows16, Cols16> & rows_mask = Static.num_row_mask[fill_pos];

        const CellInfo * pCellInfo = Sudoku::cell_info;
        size_t box = pCellInfo[fill_pos].box;
        size_t cell = pCellInfo[fill_pos].cell;

        Static.row_fill_mask[fill_pos][row].set(col);

        size_t box_x = col / BoxCellsX;
        size_t box_y = row / BoxCellsY;
        size_t box_first_y = box_y * BoxCellsY;
        size_t box_first_x = box_x * BoxCellsX;

        // Literal::NumRowCols
        {
            size_t index = 0;
            // horizontal scanning
            for (size_t x = 0; x < Cols; x++) {
                rows_mask[row].set(x);
                index++;
            }
            // vertical scanning
            for (size_t y = 0; y < Rows; y++) {
                if (y != row) {
                    rows_mask[y].set(col);
                    index++;
                }
            }
            // scanning the current box
            for (size_t cy = 0; cy < BoxCellsY; cy++) {
                size_t row_y = box_first_y + cy;
                for (size_t cx = 0; cx < BoxCellsX; cx++) {
                    size_t col_x = box_first_x + cx;
                    rows_mask[row_y].set(col_x);
                    index++;
                }
            }
            assert(index == (Cols + Rows + (BoxCellsY * BoxCellsX) - 1));

            rows_mask[row].reset(col);
        }
    }

    static void transform_to_BandBoard(const PackedBitSet2D<Rows16, Cols16> & bit_mask,
                                       BandBoard & band_mask) {
        static const uint32_t kBoxCountY32 = (uint32_t)BoxCountY;
        static const uint32_t kBoxCellsY32 = (uint32_t)BoxCellsY;
        uint32_t band;
        for (band = 0; band < kBoxCountY32; band++) {
            uint32_t band_bits = 0;
            uint32_t row = band * kBoxCellsY32;
            for (uint32_t cellY = 0; cellY < kBoxCellsY32; cellY++) {
                uint32_t row_bits = bit_mask[row].value();
                row++;
                band_bits |= row_bits << (cellY * 9);
            }
            band_mask.bands[band] = band_bits;
        }
        band_mask.bands[band] = 0;
    }

    static void print_rowHiddenSingleMaskTbl() {
        printf("\n");
        printf("static const uint32_t rowHiddenSingleMaskTbl[512] = {\n");
        for (size_t i = 0; i < 512; i++) {
            if ((i % 32) == 0)
                printf("    ");
            uint32_t rowHiddenSingleMask = rowHiddenSingleReverseMaskTbl[i] ^ 0x07U;
            printf("%d", rowHiddenSingleMask);
            if ((i % 32) == 31)
                printf(",\n");
            else
                printf(", ");
        }
        printf("};\n");
        printf("\n");
    }

    static void init_flip_mask() {
        Static.num_row_mask.reset();
        Static.row_fill_mask.reset();

        size_t fill_pos = 0;
        for (size_t row = 0; row < Rows; row++) {
            for (size_t col = 0; col < Cols; col++) {
                make_flip_mask(fill_pos, row, col);
                transform_to_BandBoard(Static.num_row_mask[fill_pos], Static.flip_mask[fill_pos]);
                transform_to_BandBoard(Static.row_fill_mask[fill_pos], Static.fill_mask[fill_pos]);
                fill_pos++;
            }
        }
    }

    static void init_mask() {
        printf("JCZ::v2::Solver::StaticData::init_mask()\n");

        init_peer_boxes();
        init_flip_mask();

        //print_rowHiddenSingleMaskTbl();
    }

    void init_board(Board & board) {
        if (kSearchMode > SearchMode::OneAnswer) {
            this->answers_.clear();
        }

        this->state_.init();

        size_t pos = 0;
        for (size_t row = 0; row < Rows; row++) {
            for (size_t col = 0; col < Cols; col++) {
                unsigned char val = board.cells[pos];
                if (val != '.') {
                    size_t num = val - '1';
                    assert(num >= (Sudoku::kMinNumber - 1) && num <= (Sudoku::kMaxNumber - 1));
                    this->update_peer_cells(this->state_, pos, num);
                }
                pos++;
            }
        }
        assert(pos == BoardSize);
    }

    inline void update_peer_cells(State & state, size_t fill_pos, size_t fill_num) {
        assert(fill_pos < Sudoku::kBoardSize);
        assert(fill_num >= (Sudoku::kMinNumber - 1) && fill_num <= (Sudoku::kMaxNumber - 1));

        size_t rowBit = fill_num * Rows + tables.div9[fill_pos];
        uint32_t band = tables.div27[rowBit];
        uint32_t shift = tables.mod27[rowBit];
        state.solvedRows.bands[band] |= 1U << shift;

        BitVec08x16 cells16, mask16;
        void * pCells16, * pMask16;

        BitVec08x16 fill_mask, solved_cells;
        pCells16 = (void *)&state.solvedCells;
        pMask16 = (void *)&Static.fill_mask[fill_pos];
        solved_cells.loadAligned(pCells16);
        fill_mask.loadAligned(pMask16);
        solved_cells |= fill_mask;
        solved_cells.saveAligned(pCells16);

        for (size_t num = 0; num < Numbers; num++) {
            pCells16 = (void *)&state.candidates[num];
            cells16.loadAligned(pCells16);
            cells16.and_not(fill_mask);
            cells16.saveAligned(pCells16);
        }

        pCells16 = (void *)&state.candidates[fill_num];
        pMask16 = (void *)&Static.flip_mask[fill_pos];
        cells16.loadAligned(pCells16);
        mask16.loadAligned(pMask16);
        cells16.and_not(mask16);
        cells16._or(fill_mask);
        cells16.saveAligned(pCells16);
    }

    template <uint32_t digit, uint32_t self, uint32_t peer1, uint32_t peer2>
    inline uint32_t update_up_down_cells(State & state, uint32_t & band) {
        uint32_t rowTriadsMask = rowTriadsMaskTbl[band & kFullRowBits] |
                                (rowTriadsMaskTbl[(band >> 9U) & kFullRowBits] << 3U) |
                                (rowTriadsMaskTbl[(band >> 18U) & kFullRowBits] << 6U);
        uint32_t lockedCandidates = keepLockedCandidatesTbl[rowTriadsMask];
        band &= lockedCandidates;
        if (band != 0) {
            uint32_t colCombBits = (band | (band >> 9U) | (band >> 18U)) & kFullRowBits;
            //if (colCombBits != state.colCombBits[digit].bands[self]) {
                //state.colCombBits[digit].bands[self] = colCombBits;
                uint32_t colLockedSingleMask = colLockedSingleMaskTbl[colCombBits];
                state.candidates[digit].bands[peer1] &= colLockedSingleMask;
                state.candidates[digit].bands[peer2] &= colLockedSingleMask;
            //}
            state.candidates[digit].bands[self] = band;
            state.prevCandidates[digit].bands[self] = band;
            uint32_t newSolvedRows = rowHiddenSingleMaskTbl[rowTriadsSingleMaskTbl[rowTriadsMask] &
                                                            combColumnSingleMaskTbl[colCombBits]];
            return newSolvedRows;
        }
        else {
            return (uint32_t)-1;
        }
    }

    template <uint32_t digit, uint32_t band_id>
    inline void update_solved_rows(State & state, uint32_t band, uint32_t bandSolvedRows) {
        uint32_t solvedCells = band & solvedRowsBitMaskTbl[bandSolvedRows];
        assert(solvedCells != 0);
        state.solvedCells.bands[band_id] |= solvedCells;
        uint32_t unsolvedCells = ~solvedCells;
        if (digit != 0)
            state.candidates[0].bands[band_id] &= unsolvedCells;
        if (digit != 1)
            state.candidates[1].bands[band_id] &= unsolvedCells;
        if (digit != 2)
            state.candidates[2].bands[band_id] &= unsolvedCells;
        if (digit != 3)
            state.candidates[3].bands[band_id] &= unsolvedCells;
        if (digit != 4)
            state.candidates[4].bands[band_id] &= unsolvedCells;
        if (digit != 5)
            state.candidates[5].bands[band_id] &= unsolvedCells;
        if (digit != 6)
            state.candidates[6].bands[band_id] &= unsolvedCells;
        if (digit != 7)
            state.candidates[7].bands[band_id] &= unsolvedCells;
        if (digit != 8)
            state.candidates[8].bands[band_id] &= unsolvedCells;
    }

    int find_hidden_single_literal(State & state) {
        register uint32_t solvedRows;
        register uint32_t bandSolvedRows;

        do {
            bandSolvedRows = (uint32_t)-1;

            /********* Number 1-3 Start *********/
        
            // Number 1
            solvedRows = state.solvedRows.bands[0];        
            if ((solvedRows & kFullRowBits) != kFullRowBits) {
                static const uint32_t digit = 0;

                // Number 1 - band 0
                register uint32_t band = state.candidates[digit].bands[0];
                if (band != state.prevCandidates[digit].bands[0]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 0, 1, 2>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 0U;
                    if ((solvedRows & (0x007U << 0U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 0>(state, band, bandSolvedRows);
                    }
                }

                // Number 1 - band 1
                band = state.candidates[digit].bands[1];
                if (band != state.prevCandidates[digit].bands[1]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 1, 0, 2>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 3U;
                    if ((solvedRows & (0x007U << 3U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 1>(state, band, bandSolvedRows);
                    }
                }

                // Number 1 - band 2
                band = state.candidates[digit].bands[2];
                if (band != state.prevCandidates[digit].bands[2]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 2, 0, 1>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 6U;
                    if ((solvedRows & (0x007U << 6U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 2>(state, band, bandSolvedRows);
                    }
                }
            }

            // Number 2
            if ((solvedRows & kFullRowBits_1) != kFullRowBits_1) {
                static const uint32_t digit = 1;

                // Number 2 - band 0
                register uint32_t band = state.candidates[digit].bands[0];
                if (band != state.prevCandidates[digit].bands[0]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 0, 1, 2>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 9U;
                    if ((solvedRows & (0x007U << 9U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 0>(state, band, bandSolvedRows);
                    }
                }

                // Number 2 - band 1
                band = state.candidates[digit].bands[1];
                if (band != state.prevCandidates[digit].bands[1]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 1, 0, 2>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 12U;
                    if ((solvedRows & (0x007U << 12U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 1>(state, band, bandSolvedRows);
                    }
                }

                // Number 2 - band 2
                band = state.candidates[digit].bands[2];
                if (band != state.prevCandidates[digit].bands[2]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 2, 0, 1>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 15U;
                    if ((solvedRows & (0x007U << 15U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 2>(state, band, bandSolvedRows);
                    }
                }
            }

            // Number 3
            if ((solvedRows & kFullRowBits_2) != kFullRowBits_2) {
                static const uint32_t digit = 2;

                // Number 3 - band 0
                register uint32_t band = state.candidates[digit].bands[0];
                if (band != state.prevCandidates[digit].bands[0]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 0, 1, 2>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 18U;
                    if ((solvedRows & (0x007U << 18U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 0>(state, band, bandSolvedRows);
                    }
                }

                // Number 3 - band 1
                band = state.candidates[digit].bands[1];
                if (band != state.prevCandidates[digit].bands[1]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 1, 0, 2>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 21U;
                    if ((solvedRows & (0x007U << 21U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 1>(state, band, bandSolvedRows);
                    }
                }

                // Number 3 - band 2
                band = state.candidates[digit].bands[2];
                if (band != state.prevCandidates[digit].bands[2]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 2, 0, 1>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 24U;
                    if ((solvedRows & (0x007U << 24U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 2>(state, band, bandSolvedRows);
                    }
                }
            }

            state.solvedRows.bands[0] = solvedRows;

            /********* Number 1-3 End *********/

            /********* Number 4-6 Start *********/
        
            // Number 4
            solvedRows = state.solvedRows.bands[1];        
            if ((solvedRows & kFullRowBits) != kFullRowBits) {
                static const uint32_t digit = 3;

                // Number 4 - band 0
                register uint32_t band = state.candidates[digit].bands[0];
                if (band != state.prevCandidates[digit].bands[0]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 0, 1, 2>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 0U;
                    if ((solvedRows & (0x007U << 0U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 0>(state, band, bandSolvedRows);
                    }
                }

                // Number 4 - band 1
                band = state.candidates[digit].bands[1];
                if (band != state.prevCandidates[digit].bands[1]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 1, 0, 2>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 3U;
                    if ((solvedRows & (0x007U << 3U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 1>(state, band, bandSolvedRows);
                    }
                }

                // Number 4 - band 2
                band = state.candidates[digit].bands[2];
                if (band != state.prevCandidates[digit].bands[2]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 2, 0, 1>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 6U;
                    if ((solvedRows & (0x007U << 6U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 2>(state, band, bandSolvedRows);
                    }
                }
            }

            // Number 5
            if ((solvedRows & kFullRowBits_1) != kFullRowBits_1) {
                static const uint32_t digit = 4;

                // Number 5 - band 0
                register uint32_t band = state.candidates[digit].bands[0];
                if (band != state.prevCandidates[digit].bands[0]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 0, 1, 2>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 9U;
                    if ((solvedRows & (0x007U << 9U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 0>(state, band, bandSolvedRows);
                    }
                }

                // Number 5 - band 1
                band = state.candidates[digit].bands[1];
                if (band != state.prevCandidates[digit].bands[1]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 1, 0, 2>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 12U;
                    if ((solvedRows & (0x007U << 12U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 1>(state, band, bandSolvedRows);
                    }
                }

                // Number 5 - band 2
                band = state.candidates[digit].bands[2];
                if (band != state.prevCandidates[digit].bands[2]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 2, 0, 1>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 15U;
                    if ((solvedRows & (0x007U << 15U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 2>(state, band, bandSolvedRows);
                    }
                }
            }

            // Number 6
            if ((solvedRows & kFullRowBits_2) != kFullRowBits_2) {
                static const uint32_t digit = 5;

                // Number 6 - band 0
                register uint32_t band = state.candidates[digit].bands[0];
                if (band != state.prevCandidates[digit].bands[0]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 0, 1, 2>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 18U;
                    if ((solvedRows & (0x007U << 18U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 0>(state, band, bandSolvedRows);
                    }
                }

                // Number 6 - band 1
                band = state.candidates[digit].bands[1];
                if (band != state.prevCandidates[digit].bands[1]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 1, 0, 2>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 21U;
                    if ((solvedRows & (0x007U << 21U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 1>(state, band, bandSolvedRows);
                    }
                }

                // Number 6 - band 2
                band = state.candidates[digit].bands[2];
                if (band != state.prevCandidates[digit].bands[2]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 2, 0, 1>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 24U;
                    if ((solvedRows & (0x007U << 24U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 2>(state, band, bandSolvedRows);
                    }
                }
            }

            state.solvedRows.bands[1] = solvedRows;

            /********* Number 4-6 End *********/

            /********* Number 7-9 Start *********/
        
            // Number 7
            solvedRows = state.solvedRows.bands[2];        
            if ((solvedRows & kFullRowBits) != kFullRowBits) {
                static const uint32_t digit = 6;

                // Number 7 - band 0
                register uint32_t band = state.candidates[digit].bands[0];
                if (band != state.prevCandidates[digit].bands[0]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 0, 1, 2>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 0U;
                    if ((solvedRows & (0x007U << 0U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 0>(state, band, bandSolvedRows);
                    }
                }

                // Number 7 - band 1
                band = state.candidates[digit].bands[1];
                if (band != state.prevCandidates[digit].bands[1]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 1, 0, 2>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 3U;
                    if ((solvedRows & (0x007U << 3U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 1>(state, band, bandSolvedRows);
                    }
                }

                // Number 7 - band 2
                band = state.candidates[digit].bands[2];
                if (band != state.prevCandidates[digit].bands[2]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 2, 0, 1>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 6U;
                    if ((solvedRows & (0x007U << 6U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 2>(state, band, bandSolvedRows);
                    }
                }
            }

            // Number 8
            if ((solvedRows & kFullRowBits_1) != kFullRowBits_1) {
                static const uint32_t digit = 7;

                // Number 8 - band 0
                register uint32_t band = state.candidates[digit].bands[0];
                if (band != state.prevCandidates[digit].bands[0]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 0, 1, 2>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 9U;
                    if ((solvedRows & (0x007U << 9U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 0>(state, band, bandSolvedRows);
                    }
                }

                // Number 8 - band 1
                band = state.candidates[digit].bands[1];
                if (band != state.prevCandidates[digit].bands[1]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 1, 0, 2>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 12U;
                    if ((solvedRows & (0x007U << 12U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 1>(state, band, bandSolvedRows);
                    }
                }

                // Number 8 - band 2
                band = state.candidates[digit].bands[2];
                if (band != state.prevCandidates[digit].bands[2]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 2, 0, 1>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 15U;
                    if ((solvedRows & (0x007U << 15U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 2>(state, band, bandSolvedRows);
                    }
                }
            }

            // Number 9
            if ((solvedRows & kFullRowBits_2) != kFullRowBits_2) {
                static const uint32_t digit = 8;

                // Number 9 - band 0
                register uint32_t band = state.candidates[digit].bands[0];
                if (band != state.prevCandidates[digit].bands[0]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 0, 1, 2>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 18U;
                    if ((solvedRows & (0x007U << 18U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 0>(state, band, bandSolvedRows);
                    }
                }

                // Number 9 - band 1
                band = state.candidates[digit].bands[1];
                if (band != state.prevCandidates[digit].bands[1]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 1, 0, 2>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 21U;
                    if ((solvedRows & (0x007U << 21U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 1>(state, band, bandSolvedRows);
                    }
                }

                // Number 9 - band 2
                band = state.candidates[digit].bands[2];
                if (band != state.prevCandidates[digit].bands[2]) {
                    bandSolvedRows = this->update_up_down_cells<digit, 2, 0, 1>(state, band);
                    if (bandSolvedRows == (uint32_t)-1)
                        return 0;
                    uint32_t newSolvedRows = bandSolvedRows << 24U;
                    if ((solvedRows & (0x007U << 24U)) != newSolvedRows) {
                        solvedRows |= newSolvedRows;
                        this->update_solved_rows<digit, 2>(state, band, bandSolvedRows);
                    }
                }
            }

            state.solvedRows.bands[2] = solvedRows;

            /********* Number 7-9 End *********/
        } while (bandSolvedRows != uint32_t(-1));

        return 1;
    }

    int fast_find_unique_candidate_cells(State & state) {
        BitVec08x16 R1, R2;

        void * pCells16 = (void *)&state.candidates[0];
        R1.loadAligned(pCells16);
        R2.setAllZeros();

        for (size_t num = 1; num < Numbers; num++) {
            BitVec08x16 row_bits;
            pCells16 = (void *)&state.candidates[num];
            row_bits.loadAligned(pCells16);

            R2 |= R1 & row_bits;
            R1 |= row_bits;
        }

        BitVec08x16 full_mask(kBitSet27, kBitSet27, kBitSet27, 0);
        bool is_legal = R1.isEqual(full_mask);
        //assert(is_legal);
        if (!is_legal) return -1;

        BitVec08x16 solved_bits;
        solved_bits.loadAligned((void *)&state.solvedCells);

        R1.and_not(R2);
        R1.and_not(solved_bits);

        int cell_count = 0;
        if (R1.isNotAllZeros()) {
#if 0
            BitVec08x16 zeros;
            BitVec08x16 neg_R1, low_bit;
            zeros.setAllZeros();
            neg_R1 = _mm_sub_epi64(zeros.m128, R1.m128);
            low_bit = R1 & neg_R1;
            R1 ^= low_bit;
#endif
#if JCZ_V2_ENABLE_R1_COUNT
            int R1_count = R1.popcount();
            assert(R1_count > 0);
#endif
            for (size_t num = 0; num < Numbers; num++) {
                BitVec08x16 row_bits;
                void * pCells16 = (void *)&state.candidates[num];
                row_bits.loadAligned(pCells16);

                row_bits &= R1;
                if (row_bits.isNotAllZeros()) {
                    // Find the position of low bit, and fill the num.
                    IntVec128 row_vec;
                    row_bits.saveAligned((void *)&row_vec);

 #if defined(WIN64) || defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
  || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
                    for (size_t i = 0; i < 2; i++) {
                        uint64_t bits64 = row_vec.u64[i];
                        while (bits64 != 0) {
                            size_t bit_pos = BitUtils::bsf64(bits64);
                            size_t pos = bandBitPosToPos64[i][bit_pos];
                            assert(pos != size_t(-1));

                            //assert(board.cells[pos] == '.');
                            //board.cells[pos] = (char)(num + '1');

                            this->update_peer_cells(this->state_, pos, num);
                            cell_count++;

                            uint64_t bit = BitUtils::ls1b64(bits64);
                            bits64 ^= bit;
                        }
                    }
#else
                    for (size_t i = 0; i < 3; i++) {
                        uint32_t bits32 = row_vec.u32[i];
                        while (bits32 != 0) {
                            size_t bit_pos = BitUtils::bsf32(bits32);
                            size_t pos = bandBitPosToPos32[i][bit_pos];
                            assert(pos != size_t(-1));

                            //assert(board.cells[pos] == '.');
                            //board.cells[pos] = (char)(num + '1');

                            this->update_peer_cells(this->state_, pos, num);
                            cell_count++;

                            uint32_t bit = BitUtils::ls1b32(bits32);
                            bits32 ^= bit;
                        }
                    }
#endif
#if JCZ_V2_ENABLE_R1_COUNT
                    if (cell_count >= R1_count) {
                        assert(cell_count > 0);
                        break;
                    }
#endif
                }
            }
            assert(cell_count > 0);
        }

        return cell_count;
    }

public:
    bool search(Board & board, ptrdiff_t empties) {
        if (empties <= 0) {
            if (kSearchMode > SearchMode::OneAnswer) {
                this->answers_.push_back(board);
                if (kSearchMode == SearchMode::MoreThanOneAnswer) {
                    if (this->answers_.size() > 1)
                        return true;
                }
            }
            else {
                return true;
            }
        }

        return false;
    }

    int find_all_unique_candidates(Board & board, ptrdiff_t empties) {
        do {
            int success = this->find_hidden_single_literal(this->state_);
            if (success == 0) return -1;

            if ((this->state_.solvedCells.bands64[0] == kBitSet27_64) &&
                (this->state_.solvedCells.bands64[1] == kBitSet27)) {
                return 1;
            }

            int unique_cells = this->fast_find_unique_candidate_cells(this->state_);
            if (unique_cells <= 0)
                break;
        } while (1);

        return 0;
    }

    bool solve(Board & board) {
        this->init_board(board);

        ptrdiff_t empties = this->calc_empties(board);
        if (empties < (ptrdiff_t)Sudoku::kMinInitCandidates)
            return false;

        int status = find_all_unique_candidates(board, empties);
        if (status < 0)
            return false;
        else if (status == 1)
            return true;

        bool success = this->search(board, empties);
        return success;
    }

    void display_result(Board & board, double elapsed_time,
                        bool print_answer = true,
                        bool print_all_answers = true) {
        basic_solver_t::display_result<kSearchMode>(board, elapsed_time, print_answer, print_all_answers);
    }
};

Solver::StaticData Solver::Static;

} // namespace v2
} // namespace JCZ
} // namespace gzSudoku

#endif // GZ_SUDOKU_JCZSOLVE_V2_H