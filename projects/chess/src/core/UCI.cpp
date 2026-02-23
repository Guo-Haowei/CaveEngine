// =============================================================================
// Supports:
//   uci
//   isready
//   ucinewgame
//   position startpos [moves ...]
//   position fen <fen...> [moves ...]
//   go perft <depth>
//   go depth <depth>            (stub: calls perft by default unless you wire search)
//   quit
// =============================================================================

#include "Position.h"
#include "MoveGen.h"
#include "UCI.h"

#include <atomic>
#include <cassert>
#include <cctype>
#include <cstdint>
#include <future>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace chess::uci {
using namespace core;

static inline std::string Trim(std::string s) {
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    return s;
}

static inline std::vector<std::string> SplitWS(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> out;
    std::string tok;
    while (iss >> tok) out.push_back(tok);
    return out;
}

static inline bool StartsWith(const std::vector<std::string>& t, size_t i, std::string_view s) {
    return i < t.size() && t[i] == s;
}

static inline std::string JoinFenTokens(const std::vector<std::string>& t, size_t fenStart, size_t fenEndExclusive) {
    std::string fen;
    for (size_t i = fenStart; i < fenEndExclusive; ++i) {
        if (!fen.empty()) fen.push_back(' ');
        fen += t[i];
    }
    return fen;
}

static bool ParseMoveUci(const Position& p_pos, std::string_view p_uci, Move& p_move) {
    (void)p_pos;

    if (p_uci.size() < 4)
        return false;

    auto file_to_int = [](char f) -> int8_t {
        return f - 'a';
    };

    auto rank_to_int = [](char r) -> int8_t {
        return r - '1';
    };

    const int8_t src_file = file_to_int(p_uci[0]);
    const int8_t src_rank = rank_to_int(p_uci[1]);
    const int8_t dst_file = file_to_int(p_uci[2]);
    const int8_t dst_rank = rank_to_int(p_uci[3]);

    if (src_file < 0 || src_file > 7 ||
        dst_file < 0 || dst_file > 7 ||
        src_rank < 0 || src_rank > 7 ||
        dst_rank < 0 || dst_rank > 7) {

        return false;
    }

    const Square from = Square::FromFileRank(src_file, src_rank);
    const Square to = Square::FromFileRank(dst_file, dst_rank);

    p_move = Move(from, to, MoveType::Normal, PieceType::Null);
    return true;
}

static uint64_t Perft(Position& p_pos, int p_depth) {
    if (p_depth <= 0) return 1;

    const MoveList moves = MoveGen::LegalMove(p_pos);

    uint64_t nodes = 0;
    for (Move mv : moves) {
        UndoState undo{};

        p_pos.MakeMove(mv, undo);
        nodes += Perft(p_pos, p_depth - 1);
        p_pos.UnmakeMove(mv, undo);
    }

    return nodes;
}

[[maybe_unused]] static uint64_t PerftRootParallel(const Position& p_pos, int p_depth) {
    if (p_depth <= 0) return 1;

    // Copy once to a mutable local for move generation.
    Position pos = p_pos;
    const MoveList moves = MoveGen::LegalMove(pos);

    std::vector<std::future<uint64_t>> futs;
    futs.reserve(moves.Size());

    for (Move mv : moves) {
        futs.emplace_back(std::async(std::launch::async, [p_pos, mv, p_depth]() mutable -> uint64_t {
            Position local = p_pos;  // copy for this task
            UndoState undo{};
            local.MakeMove(mv, undo);
            const uint64_t n = Perft(local, p_depth - 1);
            // local.UnmakeMove(mv, undo); // not needed, local is thrown away
            return n;
        }));
    }

    uint64_t total = 0;
    for (auto& f : futs) total += f.get();
    return total;
}

static uint64_t PerftDivide(Position& pos, int depth) {
    const MoveList moves = MoveGen::LegalMove(pos);

    uint64_t nodes = 0;
    for (Move mv : moves) {
        UndoState undo{};
        pos.MakeMove(mv, undo);
        const uint64_t n = (depth <= 1) ? 1ULL : Perft(pos, depth - 1);
        pos.UnmakeMove(mv, undo);
        std::cout << mv.Uci() << ": " << n << std::endl;
        nodes += n;
    }
    return nodes;
}

// Apply a list of UCI moves to position
static bool ApplyMoves(Position& pos, const std::vector<std::string>& moves, size_t startIdx) {
    for (size_t i = startIdx; i < moves.size(); ++i) {
        const std::string& uci = moves[i];
        Move m;
        if (!ParseMoveUci(pos, uci, m)) {
            std::cerr << "info string illegal/unknown move in position moves: " << uci << "\n";
            return false;
        }
        UndoState u;
        if (!pos.MakeMove(m, u)) {
            std::cerr << "info string move rejected by MakeMove: " << uci << "\n";
            return false;
        }
    }
    return true;
}

static void UciPrintId() {
    // Customize engine name/author if you want.
    std::cout << "id name MyChessEngine\n";
    std::cout << "id author You\n";
    // If you have options, print them here with: option name ... type ...
    std::cout << "uciok\n";
}

static void UciPrintReadyOk() {
    std::cout << "readyok\n";
}

int Main(int p_argc, const char** p_argv) {
    (void)p_argc;
    (void)p_argv;

    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    Position pos = Position::Startpos();

    // If you have engine state (TT, history, etc.), keep it here.
    // Example: Search search; search.Reset();

    std::string line;
    while (std::getline(std::cin, line)) {
        line = Trim(line);
        if (line.empty())
            continue;

        const auto t = SplitWS(line);
        if (t.empty())
            continue;

        const std::string& cmd = t[0];

        if (cmd == "uci") {
            UciPrintId();
            continue;
        }

        if (cmd == "isready") {
            UciPrintReadyOk();
            continue;
        }

        if (cmd == "ucinewgame") {
            pos = Position::Startpos();
            continue;
        }

        if (cmd == "quit") {
            break;
        }

        if (cmd == "position") {
            if (t.size() < 2) {
                std::cerr << "info string position: missing startpos/fen\n";
                continue;
            }

            size_t i = 1;

            if (t[i] == "startpos") {
                pos = Position::Startpos();
                ++i;
            } else if (t[i] == "fen") {
                ++i;
                // Consume fen tokens until "moves" or end.
                size_t fenStart = i;
                while (i < t.size() && t[i] != "moves") ++i;
                size_t fenEnd = i;

                const std::string fen = JoinFenTokens(t, fenStart, fenEnd);
                if (fen.empty()) {
                    std::cerr << "info string position fen: missing fen string\n";
                    continue;
                }
                pos = *Position::FromFen(fen);
            } else {
                std::cerr << "info string position: expected startpos or fen\n";
                continue;
            }

            // Optional moves
            if (i < t.size() && t[i] == "moves") {
                ++i;
                if (!ApplyMoves(pos, t, i)) {
                    // keep position as-is (already partially applied); you can choose to reset instead.
                }
            }
            continue;
        }

        if (cmd == "go") {
            if (t.size() >= 3 && t[1] == "perft") {
                const int depth = std::max(0, std::stoi(t[2]));
                // Per UCI convention, print something helpful:
                const uint64_t nodes = PerftDivide(pos, depth);
                std::cerr << "\nNodes searched: " << nodes << "\n";
                continue;
            }

            if (t.size() >= 3 && t[1] == "depth") {
                const int depth = std::max(0, std::stoi(t[2]));
                // If you have a search, call it here and return bestmove.
                // For now, run perft as a placeholder:
                const uint64_t nodes = Perft(pos, depth);
                std::cerr << "\nNodes searched: " << nodes << "\n";
                continue;
            }

            std::cerr << "info string go: unsupported (try 'go perft N')\n";
            continue;
        }

        // Optional: allow direct perft command without "go"
        if (cmd == "perft" && t.size() >= 2) {
            const int depth = std::max(0, std::stoi(t[1]));
            uint64_t nodes = Perft(pos, depth);
            std::cout << "info string Nodes searched: " << nodes << "\n";
            continue;
        }

        std::cerr << "info string unknown command: " << cmd << "\n";
    }

    return 0;
}

#if defined(CAVE_TEST)

static void PerftTestHelper(const char* p_fen,
                            uint8_t p_depth,
                            std::span<uint64_t> p_expect) {
    Position pos = *Position::FromFen(p_fen);

    printf("testing position: '%s'\n", p_fen);
    for (uint8_t i = 0; i <= p_depth; ++i) {
        const uint64_t nodes = PerftRootParallel(pos, i);
        printf("depth %d: %llu nodes\n", i, nodes);

        // if (nodes != p_expect[i]) {
        //     Position pos2 = *Position::FromFen(p_fen);
        //     PerftDivide(pos2, i);
        // }

        EXPECT_EQ(nodes, p_expect[i]);
    }
}

TEST(MoveGen, perft_test_initial_position) {
    std::array<uint64_t, 8> tests{
        1,
        20,
        400,
        8902,
        197281,
        4865609,
        119060324,
        3195901860,  // depth 7
    };

    constexpr uint8_t depth = 5;

    PerftTestHelper("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
                    depth,
                    tests);
}

TEST(MoveGen, perft_test_position2) {
    std::array<uint64_t, 8> tests{
        1,
        48,
        2039,
        97862,
        4085603,
        193690690,
        8031647685,
    };

    constexpr uint8_t depth = 3;
    PerftTestHelper("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
                    depth,
                    tests);
}

// @TODO: implement promotion
#if 0
TEST(MoveGen, perft_test_position3) {
    std::array<uint64_t, 8> tests{
        1,
        14,
        191,
        2812,
        43238,
        674624,
        11030083,
        178633661,   // depth 7
    };

    constexpr uint8_t depth = 4;
    PerftTestHelper("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
                    depth,
                    tests);
}
#endif

TEST(MoveGen, test_position4) {
    std::array<uint64_t, 8> tests{
        1,
        6,
        264,
        9467,
        422333,
        15833292,
        706045033,
    };

    constexpr uint8_t depth = 1;
    PerftTestHelper(
        "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1",
        depth,
        tests);
}

#if 0
TEST(MoveGen, test_position5) {
    std::array<uint64_t, 8> tests{
        1,
        44,
        1486,
        62379,
        2103487,
        89941194,
    };

    constexpr uint8_t depth = 1;
    PerftTestHelper(
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        depth,
        tests);
}
#endif

TEST(MoveGen, test_position6) {
    std::array<uint64_t, 8> tests{
        1,
        46,
        2079,
        89890,
        3894594,
        164075551,
        6923051137,
        287188994746,
    };

    constexpr uint8_t depth = 3;
    PerftTestHelper(
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
        depth,
        tests);
}

#endif

}  // namespace chess::uci

#if defined(CHESS_UCI)
int main(int p_argc, const char** p_argv) {
    return chess::uci::Main(p_argc, p_argv);
}
#endif
