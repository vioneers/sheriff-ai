#include <cstdint>

// Bitboard type
using Bitboard = uint64_t;

// Define the Piece Types for reference
enum PieceType { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NONE };

enum Color { WHITE, BLACK, BOTH };

// Helper to flip color
constexpr Color operator~(Color c) {
    switch (c) 
	{
		case WHITE: return BLACK; break;
		case BLACK: return WHITE; break;
		default:  return BOTH;  break;
	}
}

// Define the bit constants for the move flags
// Mostly usefull for undoing moves
// Flag should be set with priority inside make_move() since we often cannot determine them in advance without knowing the board position (i.e when converting from UCI format)
constexpr uint16_t QUIET           = 0b0000;
constexpr uint16_t DOUBLE_PUSH     = 0b0001; // Pawn double push
constexpr uint16_t K_CASTLE        = 0b0010;
constexpr uint16_t Q_CASTLE        = 0b0011;
constexpr uint16_t CAPTURE         = 0b0100;
constexpr uint16_t EP_CAPTURE      = 0b0101;

// Promotion Masks
constexpr uint16_t PROMO_N         = 0b1000; // Knight
constexpr uint16_t PROMO_B         = 0b1001; // Bishop
constexpr uint16_t PROMO_R         = 0b1010; // Rook
constexpr uint16_t PROMO_Q         = 0b1011; // Queen

// Capture Promotions (Just add the Capture bit to the Promo bit)
constexpr uint16_t PROMO_N_CAP     = 0b1100;
constexpr uint16_t PROMO_B_CAP     = 0b1101;
constexpr uint16_t PROMO_R_CAP     = 0b1110;
constexpr uint16_t PROMO_Q_CAP     = 0b1111;

// Index of each square on the board
enum Square : int {
    // Rank 1
    A1, B1, C1, D1, E1, F1, G1, H1,
    // Rank 2
    A2, B2, C2, D2, E2, F2, G2, H2,
    // Rank 3
    A3, B3, C3, D3, E3, F3, G3, H3,
    // Rank 4
    A4, B4, C4, D4, E4, F4, G4, H4,
    // Rank 5
    A5, B5, C5, D5, E5, F5, G5, H5,
    // Rank 6
    A6, B6, C6, D6, E6, F6, G6, H6,
    // Rank 7
    A7, B7, C7, D7, E7, F7, G7, H7,
    // Rank 8
    A8, B8, C8, D8, E8, F8, G8, H8
};
