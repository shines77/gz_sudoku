
#ifndef GZ_SUDOKU_SOLVER_V4_H
#define GZ_SUDOKU_SOLVER_V4_H

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
#include "SudokuTable.h"
#include "StopWatch.h"
#include "BitUtils.h"
#include "BitSet.h"
#include "PackedBitSet.h"
#include "BitArray.h"
#include "BitVec.h"

#define V4_LITERAL_ORDER_MODE   0
#define V4_SAVE_COUNT_SIZE      1

namespace gzSudoku {
namespace v4 {

static const size_t kSearchMode = SearchMode::OneSolution;

class Solver : public BasicSolverEx {
public:
    typedef BasicSolverEx                       basic_solver;
    typedef Solver                              this_type;

    typedef typename SudokuTable::NeighborCells NeighborCells;
    typedef typename SudokuTable::CellInfo      CellInfo;
    typedef typename SudokuTable::BoxesInfo     BoxesInfo;

    typedef typename Sudoku::BitMask            BitMask;
    typedef typename Sudoku::BitMaskTable       BitMaskTable;

    static const size_t kAlignment = Sudoku::Alignment;
    static const size_t BoxCellsX = Sudoku::BoxCellsX;      // 3
    static const size_t BoxCellsY = Sudoku::BoxCellsY;      // 3
    static const size_t BoxCountX = Sudoku::BoxCountX;      // 3
    static const size_t BoxCountY = Sudoku::BoxCountY;      // 3
    static const size_t MinNumber = Sudoku::MinNumber;      // 1
    static const size_t MaxNumber = Sudoku::MaxNumber;      // 9

    static const size_t Rows = Sudoku::Rows;
    static const size_t Cols = Sudoku::Cols;
    static const size_t Boxes = Sudoku::Boxes;
    static const size_t BoxSize = Sudoku::BoxSize;
    static const size_t Numbers = Sudoku::Numbers;

    static const size_t BoardSize = Sudoku::BoardSize;
    static const size_t TotalSize = Sudoku::TotalSize;
    static const size_t Neighbors = Sudoku::Neighbors;

    static const size_t Rows16 = Sudoku::Rows16;
    static const size_t Cols16 = Sudoku::Cols16;
    static const size_t Numbers10 = Sudoku::Numbers10;
    static const size_t Numbers16 = Sudoku::Numbers16;
    static const size_t Boxes16 = Sudoku::Boxes16;
    static const size_t BoxSize16 = Sudoku::BoxSize16;
    static const size_t BoardSize16 = Sudoku::BoardSize16;

    static const size_t Rows32 = Rows16 * 2;
    static const size_t Cols32 = Cols16 * 2;
    static const size_t Numbers32 = Numbers16 * 2;
    static const size_t Boxes32 = Boxes16 * 2;
    static const size_t BoxSize32 = BoxSize16 * 2;
    static const size_t BoardSize32 = Boxes32 * BoxSize32;

    static const size_t MaxConfig = 6;
    static const size_t Config8 = AlignedTo<MaxConfig, 8>::value;

    static const size_t TotalCellLiterals = Boxes16 * BoxSize16;
    static const size_t TotalRowLiterals = Rows16 * Numbers16;
    static const size_t TotalColLiterals = Cols16 * Numbers16;
    static const size_t TotalBoxLiterals = Boxes16 * Numbers16;

    static const size_t TotalLiterals =
        TotalCellLiterals + TotalRowLiterals + TotalColLiterals + TotalBoxLiterals;

#if (V4_LITERAL_ORDER_MODE == 0)
    static const size_t LiteralFirst     = 0;
    static const size_t CellLiteralFirst = LiteralFirst;
    static const size_t RowLiteralFirst  = CellLiteralFirst + TotalCellLiterals;
    static const size_t ColLiteralFirst  = RowLiteralFirst + TotalRowLiterals;
    static const size_t BoxLiteralFirst  = ColLiteralFirst + TotalColLiterals;
    static const size_t LiteralLast      = BoxLiteralFirst + TotalBoxLiterals;

    static const size_t CellLiteralLast  = RowLiteralFirst;
    static const size_t RowLiteralLast   = ColLiteralFirst;
    static const size_t ColLiteralLast   = BoxLiteralFirst;
    static const size_t BoxLiteralLast   = LiteralLast;
#else
    static const size_t LiteralFirst     = 0;
    static const size_t CellLiteralFirst = LiteralFirst;
    static const size_t BoxLiteralFirst  = CellLiteralFirst + TotalCellLiterals;
    static const size_t RowLiteralFirst  = BoxLiteralFirst + TotalBoxLiterals;
    static const size_t ColLiteralFirst  = RowLiteralFirst + TotalRowLiterals;
    static const size_t LiteralLast      = ColLiteralFirst + TotalColLiterals;

    static const size_t CellLiteralLast  = BoxLiteralFirst;
    static const size_t BoxLiteralLast   = RowLiteralFirst;
    static const size_t RowLiteralLast   = ColLiteralFirst;
    static const size_t ColLiteralLast   = LiteralLast;
#endif // (V4_LITERAL_ORDER_MODE == 0)

    static const size_t kAllRowBits = Sudoku::AllRowBits;
    static const size_t kAllColBits = Sudoku::AllColBits;
    static const size_t kAllBoxBits = Sudoku::AllBoxBits;
    static const size_t kAllBoxCellBits = Sudoku::AllBoxCellBits;
    static const size_t kAllNumberBits = Sudoku::AllNumberBits;

    static const size_t kDisableNumberMask = 0x0200U;

    static const bool kAllDimIsSame = Sudoku::AllDimIsSame;

    static const int kLiteralCntThreshold = 0;
    static const uint32_t kLiteralCntThreshold2 = 0;

private:
#if (V4_LITERAL_ORDER_MODE == 0)
    enum LiteralType {
        BoxCellNums,
        NumRowCols,
        NumColRows,
        NumBoxCells,
        MaxLiteralType
    };
#else
    enum LiteralType {
        BoxCellNums,
        NumBoxCells,
        NumRowCols,
        NumColRows,
        MaxLiteralType
    };
#endif // (V4_LITERAL_ORDER_MODE == 0)

private:

#pragma pack(push, 1)

    struct BandConfigure {
        alignas(32) PackedBitSet2D<Config8, Numbers16> config;          // Band[config][num]
        alignas(16) PackedBitSet2D<Config8, Numbers16> exclude;         // Band[config][num]
    };

    struct Box {
        alignas(32) PackedBitSet2D<BoxSize16, Numbers16> nums;          // Boxes[cell][num]
    };

    struct Row {
        alignas(32) PackedBitSet2D<Rows16, Cols16> cols;                // Number[row][col]
    };

    struct Col {
        alignas(32) PackedBitSet2D<Rows16, Cols16> rows;                // Number[col][row]
    };

    struct BoxCell {
        alignas(32) PackedBitSet2D<Boxes16, BoxSize16> cells;           // Number[box][cell]
    };

    struct InitState {
        alignas(32) PackedBitSet3D<Boxes, BoxSize16, Numbers16>   box_cell_nums;    // [box][cell][num]
        alignas(32) PackedBitSet3D<Numbers, Rows16, Cols16>       num_row_cols;     // [num][row][col]
        alignas(32) PackedBitSet3D<Numbers, Cols16, Rows16>       num_col_rows;     // [num][col][row]
        alignas(32) PackedBitSet3D<Numbers, Boxes16, BoxSize16>   num_box_cells;    // [num][box][cell]
    };

    struct State {
        BandConfigure h_band[BoxCountX];
        BandConfigure v_band[BoxCountY];

        alignas(32) PackedBitSet3D<Boxes, BoxSize16, Numbers16>   box_cell_nums;    // [box][cell][num]
    };

    struct Count {
        struct Sizes {
            alignas(32) uint16_t box_cells[Boxes][BoxSize16];
            alignas(32) uint16_t num_boxes[Numbers][Boxes16];
            alignas(32) uint16_t num_rows[Numbers][Rows16];
            alignas(32) uint16_t num_cols[Numbers][Cols16];
        } sizes;

        struct Counts {
            alignas(16) uint8_t box_cells[Boxes16];
            alignas(16) uint8_t num_boxes[Numbers16];
            alignas(16) uint8_t num_rows[Numbers16];
            alignas(16) uint8_t num_cols[Numbers16];
        } counts;

        struct Index {
            alignas(16) uint8_t box_cells[Boxes16];
            alignas(16) uint8_t num_boxes[Numbers16];
            alignas(16) uint8_t num_rows[Numbers16];
            alignas(16) uint8_t num_cols[Numbers16];
        } indexs;

        struct Total {
            alignas(32) uint16_t min_literal_size[16];
            alignas(32) uint16_t min_literal_index[16];
        } total;
    };

    union literal_count_t {
        struct {
            uint8_t count:7;
            uint8_t enable:1;
        };
        uint8_t value;
    };

    struct LiteralInfo {
        uint32_t literal_size;
        uint16_t literal_type;
        uint16_t literal_index;
    };

    template <size_t nBoxCountX, size_t nBoxCountY>
    struct PeerBoxes {
        static const uint32_t kBoxesCount = (uint32_t)((nBoxCountX - 1) + (nBoxCountY - 1) + 1);

        uint32_t boxes_count() const { return kBoxesCount; }

        int boxes[kBoxesCount];
    };

    template <size_t nBoxCountX, size_t nBoxCountY>
    struct HVPeerBoxes {
        static const uint32_t kHorizontalBoxes = uint32_t(nBoxCountX - 1);
        static const uint32_t kVerticalBoxes = uint32_t(nBoxCountX - 1);

        uint32_t h_boxes_count() const { return kHorizontalBoxes; }
        uint32_t v_boxes_count() const { return kVerticalBoxes; }

        int h_boxes[kHorizontalBoxes];
        int v_boxes[kVerticalBoxes];
    };

    typedef PeerBoxes<BoxCountX, BoxCountY>     peer_boxes_t;
    typedef HVPeerBoxes<BoxCountX, BoxCountY>   hv_peer_boxes_t;

    struct StaticData {
        alignas(32) PackedBitSet3D<BoardSize, Rows16, Cols16>      num_row_mask;
        alignas(32) PackedBitSet3D<BoardSize, Cols16, Rows16>      num_col_mask;
        alignas(32) PackedBitSet3D<BoardSize, Boxes16, BoxSize16>  num_box_mask;

        alignas(32) PackedBitSet3D<Boxes, BoxSize16, Numbers16>    flip_mask[BoardSize][Numbers];

        bool                    mask_is_inited;
        peer_boxes_t            peer_boxes[Boxes];
        hv_peer_boxes_t         hv_peer_boxes[Boxes];

        StaticData() : mask_is_inited(false) {
            if (!Static.mask_is_inited) {
                SudokuTable::initialize();
                this_type::init_mask();
                Static.mask_is_inited = true;
            }
        }
    };

#pragma pack(pop)

    InitState       init_state_;
    State           state_;
    Count           count_;
    LiteralInfo     min_info_;

    alignas(32) literal_count_t literal_info_[TotalLiterals];

    static StaticData Static;

public:
    Solver() : basic_solver() {
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
                peerBoxes.boxes[index++] = box;
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

                std::sort(&peerBoxes.boxes[1], &peerBoxes.boxes[peerBoxes.boxes_count()]);
                Static.peer_boxes[box] = peerBoxes;
            }
        }
    }

    static size_t make_flip_mask(size_t fill_pos, size_t row, size_t col) {
        PackedBitSet2D<Rows16, Cols16> & rows_mask      = Static.num_row_mask[fill_pos];
        PackedBitSet2D<Cols16, Rows16> & cols_mask      = Static.num_col_mask[fill_pos];
        PackedBitSet2D<Boxes16, BoxSize16> & boxes_mask = Static.num_box_mask[fill_pos];

        const CellInfo * pCellInfo = SudokuTable::cell_info;
        size_t box, cell;

        size_t index = 0;

        // Horizontal scanning the board
        size_t pos_y = row * Cols;
        for (size_t x = 0; x < Cols; x++) {
            size_t pos = pos_y + x;

            box = pCellInfo[pos].box;
            cell = pCellInfo[pos].cell;

            rows_mask[row].set(x);
            cols_mask[x].set(row);
            boxes_mask[box].set(cell);

            if (x != col) {
                for (size_t num = 0; num < Numbers; num++) {
                    PackedBitSet3D<Boxes, BoxSize16, Numbers16> & flip_mask
                        = Static.flip_mask[fill_pos][num];
                    flip_mask[box][cell].set(num);
                }

                index++;
            }
        }

        // Vertical scanning the board
        size_t pos_x = col;
        for (size_t y = 0; y < Rows; y++) {
            if (y != row) {
                size_t pos = y * Cols + pos_x;

                box = pCellInfo[pos].box;
                cell = pCellInfo[pos].cell;

                rows_mask[y].set(col);
                cols_mask[col].set(y);
                boxes_mask[box].set(cell);

                for (size_t num = 0; num < Numbers; num++) {
                    PackedBitSet3D<Boxes, BoxSize16, Numbers16> & flip_mask
                        = Static.flip_mask[fill_pos][num];
                    flip_mask[box][cell].set(num);
                }

                index++;
            }
        }

        // Scanning the current box
        size_t box_x = col / BoxCellsX;
        size_t box_y = row / BoxCellsY;
        size_t box_base = (box_y * BoxCellsY) * Cols + box_x * BoxCellsX;
        size_t cell_x = col % BoxCellsX;
        size_t cell_y = row % BoxCellsY;
        size_t pos = box_base;
        for (size_t y = 0; y < BoxCellsY; y++) {
            if (y == cell_y) {
                pos += Cols;
            }
            else {
                for (size_t x = 0; x < BoxCellsX; x++) {
                    if (x != cell_x) {
                        assert(pos != fill_pos);
                        box = pCellInfo[pos].box;
                        cell = pCellInfo[pos].cell;
                        row = pCellInfo[pos].row;
                        col = pCellInfo[pos].col;

                        rows_mask[row].set(col);
                        cols_mask[col].set(row);
                        boxes_mask[box].set(cell);

                        for (size_t num = 0; num < Numbers; num++) {
                            PackedBitSet3D<Boxes, BoxSize16, Numbers16> & flip_mask
                                = Static.flip_mask[fill_pos][num];
                            flip_mask[box][cell].set(num);
                        }

                        index++;
                    }
                    pos++;
                }
                pos += (Cols - BoxCellsX);
            }
        }

        box = pCellInfo[fill_pos].box;
        cell = pCellInfo[fill_pos].cell;

        // Current fill pos, set all number bits
        for (size_t num = 0; num < Numbers; num++) {
            PackedBitSet3D<Boxes, BoxSize16, Numbers16> & flip_mask = Static.flip_mask[fill_pos][num];
            flip_mask[box][cell].fill(kAllNumberBits);
        }

        assert(index == Neighbors);
        return index;
    }

    static void init_flip_mask() {
        for (size_t pos = 0; pos < BoardSize; pos++) {
            for (size_t num = 0; num < Numbers; num++) {
                Static.flip_mask[pos][num].reset();
            }
        }
        Static.num_row_mask.reset();
        Static.num_col_mask.reset();
        Static.num_box_mask.reset();

        size_t fill_pos = 0;
        for (size_t row = 0; row < Rows; row++) {
            for (size_t col = 0; col < Cols; col++) {
                make_flip_mask(fill_pos, row, col);
                fill_pos++;
            }
        }

        // Flip all the mask bits
        for (size_t pos = 0; pos < BoardSize; pos++) {
            for (size_t num = 0; num < Numbers; num++) {
                Static.flip_mask[pos][num].flip();
            }
        }

        Static.num_row_mask.flip();
        Static.num_col_mask.flip();
        Static.num_box_mask.flip();
    }

    static void init_mask() {
        if (bPrintSudokuStaticInit) {
            printf("v4::Solver::StaticData::init_mask()\n");
        }

        init_peer_boxes();
        init_flip_mask();
    }

    static const uint16_t kEnableLiteral16 = 0x0000;
    static const uint16_t kDisableLiteral16 = 0xFFFF;

    void init_literal_info() {
        init_literal_count();
        init_literal_index();
        init_literal_total();
    }

    void init_literal_count() {
        for (size_t i = 0; i < Boxes16; i++) {
            this->count_.counts.box_cells[i] = 255;
        }

        for (size_t i = 0; i < Numbers16; i++) {
            this->count_.counts.num_rows[i] = 255;
        }

        for (size_t i = 0; i < Numbers16; i++) {
            this->count_.counts.num_cols[i] = 255;
        }

        for (size_t i = 0; i < Numbers16; i++) {
            this->count_.counts.num_boxes[i] = 255;
        }
    }

    void init_literal_index() {
        for (size_t i = 0; i < Boxes16; i++) {
            this->count_.indexs.box_cells[i] = uint8_t(-1);
        }

        for (size_t i = 0; i < Numbers16; i++) {
            this->count_.indexs.num_rows[i] = uint8_t(-1);
        }

        for (size_t i = 0; i < Numbers16; i++) {
            this->count_.indexs.num_cols[i] = uint8_t(-1);
        }

        for (size_t i = 0; i < Numbers16; i++) {
            this->count_.indexs.num_boxes[i] = uint8_t(-1);
        }
    }

    void init_literal_total() {
        for (size_t i = 0; i < 16; i++) {
            this->count_.total.min_literal_size[i] = 65535;
            this->count_.total.min_literal_index[i] = uint16_t(-1);
        }
    }

    inline void fillNum(InitState & state, size_t pos, size_t row, size_t col,
                        size_t box, size_t cell, size_t num) {
        assert(state.box_cell_nums[box][cell].test(num));
        assert(state.num_row_cols[num][row].test(col));
        assert(state.num_col_rows[num][col].test(row));
        assert(state.num_box_cells[num][box].test(cell));

        PackedBitSet<Numbers16> cell_num_bits = state.box_cell_nums[box][cell];
        state.box_cell_nums[box][cell].reset();

        /*
        size_t box_pos = box * BoxSize16 + cell;
        size_t row_idx = num * Rows16 + row;
        size_t col_idx = num * Cols16 + col;
        size_t box_idx = num * Boxes16 + box;

        disable_cell_literal(box_pos);
        disable_row_literal(row_idx);
        disable_col_literal(col_idx);
        disable_box_literal(box_idx);
        //*/

        size_t num_bits = cell_num_bits.to_ulong();
        // Exclude the current number, because it will be process later.
        num_bits ^= (size_t(1) << num);
        while (num_bits != 0) {
            size_t num_bit = BitUtils::ls1b(num_bits);
            size_t cur_num = BitUtils::bsf(num_bit);
            num_bits ^= num_bit;

            assert(state.num_row_cols[cur_num][row].test(col));
            assert(state.num_col_rows[cur_num][col].test(row));
            assert(state.num_box_cells[cur_num][box].test(cell));

            state.num_row_cols[cur_num][row].reset(col);
            state.num_col_rows[cur_num][col].reset(row);
            state.num_box_cells[cur_num][box].reset(cell);
        }
    }

    inline void updateNeighborCells(InitState & state, size_t fill_pos, size_t box, size_t num) {
        const peer_boxes_t & peerBoxes = Static.peer_boxes[box];
        const PackedBitSet3D<Boxes, BoxSize16, Numbers16> & flip_mask
            = Static.flip_mask[fill_pos][num];
        for (size_t i = 0; i < peerBoxes.boxes_count(); i++) {
            size_t box_idx = peerBoxes.boxes[i];
            state.box_cell_nums[box_idx] &= flip_mask[box_idx];
        }

        state.num_row_cols[num] &= Static.num_row_mask[fill_pos];
        state.num_col_rows[num] &= Static.num_col_mask[fill_pos];
        state.num_box_cells[num] &= Static.num_box_mask[fill_pos];
    }

    LiteralInfo count_literal_size_init() {
        BitVec16x16 disable_mask;
        BitVec16x16 numbits_mask;
        numbits_mask.fill16(kAllNumberBits);
        //numbits_mask.fill16(0xFFFF);

        // Position (Box-Cell) literal
        uint32_t min_cell_size = 255;
        uint32_t min_cell_index = uint32_t(-1);
        for (size_t box = 0; box < Boxes; box++) {
            const PackedBitSet2D<BoxSize16, Numbers16> * bitset;
            bitset = &this->init_state_.box_cell_nums[box];
            BitVec16x16 box_bits;
            box_bits.loadAligned(bitset);

            disable_mask = box_bits.whichIsZeros();
            disable_mask.and_equal(numbits_mask);

            box_bits.or_equal(disable_mask);

            BitVec16x16 popcnt16 = box_bits.popcount16<BoxSize, Numbers>();
#if V4_SAVE_COUNT_SIZE
            popcnt16.saveAligned(&this->count_.sizes.box_cells[box][0]);
#endif
            int min_index = -1;
            uint32_t min_size = popcnt16.minpos16<Numbers>(min_cell_size, min_index);
            this->count_.counts.box_cells[box] = (uint8_t)min_size;
            if (likely(min_index != -1)) {
                size_t cell_index = box * BoxSize16 + min_index;
                this->count_.indexs.box_cells[box] = (uint8_t)cell_index;
                min_cell_index = (uint32_t)cell_index;
            }
            else {
                this->count_.indexs.box_cells[box] = (uint8_t)min_index;
            }
        }
        this->count_.total.min_literal_size[0] = (uint16_t)min_cell_size;
        this->count_.total.min_literal_index[0] = (uint16_t)min_cell_index;

        BitVec16x16 num_rows_mask;
        num_rows_mask.fill16(kAllColBits);

        // Row literal
        uint32_t min_row_size = 255;
        uint32_t min_row_index = uint32_t(-1);
        for (size_t num = 0; num < Numbers; num++) {
            const PackedBitSet2D<Rows16, Cols16> * bitset;
            bitset = &this->init_state_.num_row_cols[num];
            BitVec16x16 num_row_bits;
            num_row_bits.loadAligned(bitset);

            disable_mask = num_row_bits.whichIsZeros();
            disable_mask.and_equal(num_rows_mask);

            num_row_bits.or_equal(disable_mask);

            BitVec16x16 popcnt16 = num_row_bits.popcount16<Rows, Cols>();
#if V4_SAVE_COUNT_SIZE
            popcnt16.saveAligned(&this->count_.sizes.num_rows[num][0]);
#endif
            int min_index = -1;
            uint32_t min_size = popcnt16.minpos16<Cols>(min_row_size, min_index);
            this->count_.counts.num_rows[num] = (uint8_t)min_size;
            if (likely(min_index != -1)) {
                size_t row_index = num * Rows16 + min_index;
                this->count_.indexs.num_rows[num] = (uint8_t)row_index;
                min_row_index = (uint32_t)row_index;
            }
            else {
                this->count_.indexs.num_rows[num] = (uint8_t)min_index;
            }
        }
        this->count_.total.min_literal_size[1] = (uint16_t)min_row_size;
        this->count_.total.min_literal_index[1] = (uint16_t)min_row_index;

        BitVec16x16 num_cols_mask;
        num_cols_mask.fill16(kAllRowBits);

        // Col literal
        uint32_t min_col_size = 255;
        uint32_t min_col_index = uint32_t(-1);
        for (size_t num = 0; num < Numbers; num++) {
            const PackedBitSet2D<Cols16, Rows16> * bitset;
            bitset = &this->init_state_.num_col_rows[num];
            BitVec16x16 num_col_bits;
            num_col_bits.loadAligned(bitset);

            disable_mask = num_col_bits.whichIsZeros();
            disable_mask.and_equal(num_cols_mask);

            num_col_bits.or_equal(disable_mask);

            BitVec16x16 popcnt16 = num_col_bits.popcount16<Cols, Rows>();
#if V4_SAVE_COUNT_SIZE
            popcnt16.saveAligned(&this->count_.sizes.num_cols[num][0]);
#endif
            int min_index = -1;
            uint32_t min_size = popcnt16.minpos16<Rows>(min_col_size, min_index);
            this->count_.counts.num_cols[num] = (uint8_t)min_size;
            if (likely(min_index != -1)) {
                size_t col_index = num * Cols16 + min_index;
                this->count_.indexs.num_cols[num] = (uint8_t)col_index;
                min_col_index = (uint32_t)col_index;
            }
            else {
                this->count_.indexs.num_cols[num] = (uint8_t)min_index;
            }
        }
        this->count_.total.min_literal_size[2] = (uint16_t)min_col_size;
        this->count_.total.min_literal_index[2] = (uint16_t)min_col_index;

        BitVec16x16 num_box_mask;
        num_box_mask.fill16(kAllBoxCellBits);

        // Box-Cell literal
        uint32_t min_box_size = 255;
        uint32_t min_box_index = uint32_t(-1);
        for (size_t num = 0; num < Numbers; num++) {
            const PackedBitSet2D<Boxes16, BoxSize16> * bitset;
            bitset = &this->init_state_.num_box_cells[num];
            BitVec16x16 num_box_bits;
            num_box_bits.loadAligned(bitset);

            disable_mask = num_box_bits.whichIsZeros();
            disable_mask.and_equal(num_box_mask);

            num_box_bits.or_equal(disable_mask);

            BitVec16x16 popcnt16 = num_box_bits.popcount16<Boxes, BoxSize>();
#if V4_SAVE_COUNT_SIZE
            popcnt16.saveAligned(&this->count_.sizes.num_boxes[num][0]);
#endif
            int min_index = -1;
            uint32_t min_size = popcnt16.minpos16<BoxSize>(min_box_size, min_index);
            this->count_.counts.num_boxes[num] = (uint8_t)min_size;
            if (likely(min_index != -1)) {
                size_t box_index = num * Boxes16 + min_index;
                this->count_.indexs.num_boxes[num] = (uint8_t)box_index;
                min_box_index = (uint32_t)box_index;
            }
            else {
                this->count_.indexs.num_boxes[num] = (uint8_t)min_index;
            }
        }
        this->count_.total.min_literal_size[3] = (uint16_t)min_box_size;
        this->count_.total.min_literal_index[3] = (uint16_t)min_box_index;

        BitVec08x16 min_literal;
        min_literal.loadAligned(&this->count_.total.min_literal_size[0]);
        int min_literal_type;
        uint32_t min_literal_size = min_literal.minpos16<4>(min_literal_type);
        uint32_t min_literal_index = this->count_.total.min_literal_index[min_literal_type];

        LiteralInfo literalInfo;
        literalInfo.literal_size = min_literal_size;
        literalInfo.literal_type = (uint16_t)min_literal_type;
        literalInfo.literal_index = (uint16_t)min_literal_index;
        return literalInfo;
    }

    LiteralInfo count_all_literal_size() {
        BitVec16x16 disable_mask;
        BitVec16x16 filter_mask;
        BitVec16x16 numbits_mask;
        filter_mask.fill16(kDisableNumberMask);
        numbits_mask.fill16(kAllNumberBits);

        // Position (Box-Cell) literal
        uint32_t min_cell_size = 255;
        uint32_t min_cell_index = uint32_t(-1);
        for (size_t box = 0; box < Boxes; box++) {
            const PackedBitSet2D<BoxSize16, Numbers16> * bitset;
            bitset = &this->init_state_.box_cell_nums[box];
            BitVec16x16 box_bits;
            box_bits.loadAligned(bitset);

            disable_mask = box_bits.whichIsEqual(filter_mask);
            disable_mask.and_equal(numbits_mask);

            box_bits.and_equal(numbits_mask);
            box_bits.or_equal(disable_mask);

            BitVec16x16 popcnt16 = box_bits.popcount16<BoxSize, Numbers>();
#if V4_SAVE_COUNT_SIZE
            popcnt16.saveAligned(&this->count_.sizes.box_cells[box * BoxSize16]);
#endif
            int min_index = -1;
            uint32_t min_size = popcnt16.minpos16<Numbers>(min_cell_size, min_index);
            this->count_.counts.box_cells[box] = (uint8_t)min_size;
            if (likely(min_index != -1)) {
                size_t cell_index = box * BoxSize16 + min_index;
                this->count_.indexs.box_cells[box] = (uint8_t)cell_index;
                min_cell_index = (uint32_t)cell_index;
            }
            else {
                this->count_.indexs.box_cells[box] = (uint8_t)min_index;
            }
        }
        this->count_.total.min_literal_size[0] = (uint16_t)min_cell_size;
        this->count_.total.min_literal_index[0] = (uint16_t)min_cell_index;

        // Row literal
        uint32_t min_row_size = 255;
        uint32_t min_row_index = uint32_t(-1);
        for (size_t num = 0; num < Numbers; num++) {
            const PackedBitSet2D<Rows16, Cols16> * bitset;
            bitset = &this->init_state_.num_row_cols[num];
            BitVec16x16 num_row_bits;
            num_row_bits.loadAligned(bitset);

            BitVec16x16 popcnt16 = num_row_bits.popcount16<Rows, Cols>();
#if V4_SAVE_COUNT_SIZE
            popcnt16.saveAligned(&this->count_.sizes.num_rows[num * Rows16]);
#endif
            int min_index = -1;
            uint32_t min_size = popcnt16.minpos16<Cols>(min_row_size, min_index);
            this->count_.counts.num_rows[num] = (uint8_t)min_size;
            if (likely(min_index != -1)) {
                size_t row_index = num * Rows16 + min_index;
                this->count_.indexs.num_rows[num] = (uint8_t)row_index;
                min_row_index = (uint32_t)row_index;
            }
            else {
                this->count_.indexs.num_rows[num] = (uint8_t)min_index;
            }
        }
        this->count_.total.min_literal_size[1] = (uint16_t)min_row_size;
        this->count_.total.min_literal_index[1] = (uint16_t)min_row_index;

        // Col literal
        uint32_t min_col_size = 255;
        uint32_t min_col_index = uint32_t(-1);
        for (size_t num = 0; num < Numbers; num++) {
            const PackedBitSet2D<Cols16, Rows16> * bitset;
            bitset = &this->init_state_.num_col_rows[num];
            BitVec16x16 num_col_bits;
            num_col_bits.loadAligned(bitset);

            BitVec16x16 popcnt16 = num_col_bits.popcount16<Cols, Rows>();
#if V4_SAVE_COUNT_SIZE
            popcnt16.saveAligned(&this->count_.sizes.num_cols[num * Cols16]);
#endif
            int min_index = -1;
            uint32_t min_size = popcnt16.minpos16<Rows>(min_col_size, min_index);
            this->count_.counts.num_cols[num] = (uint8_t)min_size;
            if (likely(min_index != -1)) {
                size_t col_index = num * Cols16 + min_index;
                this->count_.indexs.num_cols[num] = (uint8_t)col_index;
                min_col_index = (uint32_t)col_index;
            }
            else {
                this->count_.indexs.num_cols[num] = (uint8_t)min_index;
            }
        }
        this->count_.total.min_literal_size[2] = (uint16_t)min_col_size;
        this->count_.total.min_literal_index[2] = (uint16_t)min_col_index;

        // Box-Cell literal
        uint32_t min_box_size = 255;
        uint32_t min_box_index = uint32_t(-1);
        for (size_t num = 0; num < Numbers; num++) {
            const PackedBitSet2D<Boxes16, BoxSize16> * bitset;
            bitset = &this->init_state_.num_box_cells[num];
            BitVec16x16 num_box_bits;
            num_box_bits.loadAligned(bitset);

            BitVec16x16 popcnt16 = num_box_bits.popcount16<Boxes, BoxSize>();
#if V4_SAVE_COUNT_SIZE
            popcnt16.saveAligned(&this->count_.sizes.num_boxes[num * Boxes16]);
#endif
            int min_index = -1;
            uint32_t min_size = popcnt16.minpos16<BoxSize>(min_box_size, min_index);
            this->count_.counts.num_boxes[num] = (uint8_t)min_size;
            if (likely(min_index != -1)) {
                size_t box_index = num * Boxes16 + min_index;
                this->count_.indexs.num_boxes[num] = (uint8_t)box_index;
                min_box_index = (uint32_t)box_index;
            }
            else {
                this->count_.indexs.num_boxes[num] = (uint8_t)min_index;
            }
        }
        this->count_.total.min_literal_size[3] = (uint16_t)min_box_size;
        this->count_.total.min_literal_index[3] = (uint16_t)min_box_index;

        BitVec08x16 min_literal;
        min_literal.loadAligned(&this->count_.total.min_literal_size[0]);
        int min_literal_type;
        uint32_t min_literal_size = min_literal.minpos16<4>(min_literal_type);
        uint32_t min_literal_index = this->count_.total.min_literal_index[min_literal_type];

        LiteralInfo literalInfo;
        literalInfo.literal_size = min_literal_size;
        literalInfo.literal_type = (uint16_t)min_literal_type;
        literalInfo.literal_index = (uint16_t)min_literal_index;
        return literalInfo;
    }

    void init_board(const Board & board) {
        init_literal_info();
        /*
        this->init_state_.box_cell_nums.fill(kAllNumberBits);
        this->init_state_.num_row_cols.fill(kAllColBits);
        this->init_state_.num_col_rows.fill(kAllRowBits);
        this->init_state_.num_box_cells.fill(kAllBoxCellBits);
        //*/

        std::memset((void *)&this->count_.sizes.box_cells[0], 0, sizeof(this->count_.sizes.box_cells));
        std::memset((void *)&this->count_.sizes.num_boxes[0], 0, sizeof(this->count_.sizes.num_boxes));
        std::memset((void *)&this->count_.sizes.num_rows[0], 0, sizeof(this->count_.sizes.num_rows));
        std::memset((void *)&this->count_.sizes.num_cols[0], 0, sizeof(this->count_.sizes.num_cols));

        for (size_t box = 0; box < Boxes; box++) {
            for (size_t cell = 0; cell < BoxSize; cell++) {
                this->init_state_.box_cell_nums[box][cell].fill(kAllNumberBits);
                this->count_.sizes.box_cells[box][cell] = Numbers;
            }
        }

        for (size_t num = 0; num < Numbers; num++) {
            for (size_t row = 0; row < Rows; row++) {
                this->init_state_.num_row_cols[num][row].fill(kAllColBits);
                this->count_.sizes.num_rows[num][row] = Cols;
            }
            for (size_t col = 0; col < Cols; col++) {
                this->init_state_.num_col_rows[num][col].fill(kAllRowBits);
                this->count_.sizes.num_cols[num][col] = Rows;
            }
            for (size_t box = 0; box < Boxes; box++) {
                this->init_state_.num_box_cells[num][box].fill(kAllBoxCellBits);
                this->count_.sizes.num_boxes[num][box] = BoxSize;
            }
        }

        if (kSearchMode > SearchMode::OneSolution) {
            this->answers_.clear();
        }

        size_t empties = this->calc_empties(board);
        this->empties_ = empties;

        size_t pos = 0;
        for (size_t row = 0; row < Rows; row++) {
            size_t box_y = (row / BoxCellsY) * BoxCountX;
            size_t cell_y = (row % BoxCellsY) * BoxCellsX;
            for (size_t col = 0; col < Cols; col++) {
                unsigned char val = board.cells[pos];
                if (val != '.') {
                    size_t box_x = col / BoxCellsX;
                    size_t box = box_y + box_x;
                    size_t cell_x = col % BoxCellsX;
                    size_t cell = cell_y + cell_x;
                    size_t num = val - '1';

                    this->fillNum(this->init_state_, pos, row, col, box, cell, num);
                    this->updateNeighborCells(this->init_state_, pos, box, num);
                }
                pos++;
            }
        }
        assert(pos == BoardSize);

        LiteralInfo literalInfo;
        literalInfo = this->count_literal_size_init();
        this->min_info_ = literalInfo;
    }

public:
    bool search(Board & board, size_t empties, const LiteralInfo & literalInfo) {
        if (empties == 0) {
            if (kSearchMode > SearchMode::OneSolution) {
                this->answers_.push_back(board);
                if (kSearchMode == SearchMode::MoreThanOneSolution) {
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

    int solve(const Board & board, Board & solution, int limitSolutions = 1) {
        this->init_board(board);
        solution = board;
        bool success = this->search(solution, this->empties_, this->min_info_);
        return (success) ? Status::Solved : Status::Invalid;
    }

    void display_result(Board & board, double elapsed_time,
                        bool print_answer = true,
                        bool print_all_answers = true) {
        basic_solver::display_result<kSearchMode>(board, elapsed_time, print_answer, print_all_answers);
    }
};

Solver::StaticData Solver::Static;

} // namespace v4
} // namespace gzSudoku

#endif // GZ_SUDOKU_SOLVER_V4_H
