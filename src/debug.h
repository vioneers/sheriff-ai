#pragma once

#include <iostream>
#include <cstdlib> // for std::abort


// Only compile this in Debug mode
#ifdef DEBUG
    #define ASSERT(condition, message) \
        do { \
            if (!(condition)) { \
                std::cerr << "Assertion failed: " << #condition << "\n"; \
                std::cerr << "File: " << __FILE__ << ", Line: " << __LINE__ << "\n"; \
                std::cerr << "Message: " << message << "\n"; \
                std::abort(); \
            } \
        } while (0)

    // Specialized assert that takes a Board object
    #define ASSERT_BOARD(condition, message, board) \
        do { \
            if (!(condition)) { \
                std::cerr << "### ENGINE CRASH ###\n"; \
                std::cerr << "Error: " << message << "\n"; \
                std::cerr << "FEN: " << board.to_fen() << "\n"; \
                std::cerr << "Loc: " << __FILE__ << ":" << __LINE__ << "\n"; \
                std::abort(); \
            } \
        } while (0)
#else
    // In Release mode, these compile to nothing for max speed
    #define ASSERT(condition, message) do {} while (0)
    #define ASSERT_BOARD(condition, message, board) do {} while (0)
#endif
