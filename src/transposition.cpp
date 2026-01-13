#include "engine.h"
#include <iostream>

engine_t::TTentry& engine_t::TTprobe(uint64_t z_key) 
{
#ifdef DEBUG
	TT_PROBES++;
#endif

	return TT[z_key & ((1<<TT_SIZE) - 1)];
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
    if (score <= alpha)
        node_type = UPPER_BOUND;
    else if (score >= beta)
        node_type = LOWER_BOUND;
    else
        node_type = EXACT;

    // replace the node that is in TT if collision
    // for now use depth TODO: implement better strategies
    if (e.key == 0 || e.depth_left < depth_left || (e.depth_left == depth_left && node_type == EXACT))
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

    // allow shalower nodes to influence ordering but not cutoff
    if (outMove.is_null() && !e.bestMove.is_null() && e.depth_left >= depth_left - 1)
    outMove = e.bestMove;

    // if entry has been searched for less depth than we currently have left we disregard it
    if (e.depth_left < depth_left)
        return false;

    // add / remove the current ply / depth if dealing with a mate score
    int ttScore = e.score;
    if (std::abs(ttScore) >= MATE_SCORE - MAX_PLY) {
        if (ttScore > 0) ttScore -= ply;
        else             ttScore += ply;
    }

    if (e.node_type == EXACT) {
        outScore = ttScore;
        return true;
    }

    // only apply bound refutations if the bounds are good (high enough search depth)
    if (e.node_type == LOWER_BOUND && ttScore >= beta) {
        outScore = ttScore;
        return true; // fail-high
    }
    if (e.node_type == UPPER_BOUND && ttScore <= alpha) {
        outScore = ttScore;
        return true; // fail-low
    }

    return false;
}