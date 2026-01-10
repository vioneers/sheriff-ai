#include "engine.h"
#include <iostream>

engine_t::TTentry& engine_t::TTprobe(uint64_t z_key) 
{
#ifdef DEBUG
	TT_PROBES++;
#endif

	return TT[z_key & (TT_SIZE - 1)];
}

void engine_t::TTstore(
    uint64_t z_key,
    int score,
    int depth_left,
    int ply,
    int alpha, 
    int beta,
    move_t bestMove
) {

    TTentry& e = TTprobe(z_key);

    // remove the ply / depth from a mate score
    if (std::abs(score) >= MATE_SCORE - MAX_PLY)
    {
        if (score > 0)
            score += ply;
        else
            score -= ply;
    }

    // determine node (bound) type
    NodeType node_type;
    if (score < alpha)
        node_type = UPPER_BOUND;
    else if (score > beta)
        node_type = LOWER_BOUND;
    else
        node_type = EXACT;

    // replace the node that is in TT if collision
    // for now use depth TODO: implement better strategies
    if (e.key == 0 || e.depth_left <= depth_left)
    {
        e.key = z_key;
        e.depth_left = depth_left;
        e.score = score;
        e.node_type = node_type;
        e.bestMove = bestMove;
    }
}

bool engine_t::checkTT(                   // check if we can use info from the TT to end the search (return TRUE if yes)
    uint64_t key,
    int depth_left,
    int ply,
    int alpha,
    int beta,
    int& outScore,
    move_t& outMove
) {
    TTentry& e = TTprobe(key);

    if (e.key != key)
        return false;

#ifdef DEBUG
    TT_HITS++;
#endif

    // if entry has been searched for less depth than we currently have left we disregard it
    if (e.depth_left < depth_left)
        return false;

    // add / remove the current ply / depth if dealing with a mate score
    if (std::abs(e.score) >= MATE_SCORE - MAX_PLY)
    {
        if (e.score > 0)
            e.score -= ply;
        else
            e.score += ply;
    }

    if (!outMove.is_null())
        outMove = e.bestMove;

    if (e.node_type == EXACT) {
        outScore = e.score;
        return true;
    }

    if (e.node_type == LOWER_BOUND && e.score >= beta) {
        outScore = e.score;
        return true;  // fail high
    }

    if (e.node_type == UPPER_BOUND && e.score <= alpha) {
        outScore = e.score;
        return true;  // fail low
    }

    return false;
}