#include <skeleton/actions.h>
#include <skeleton/constants.h>
#include <skeleton/runner.h>
#include <skeleton/states.h>
#include <time.h>
#include <random>
#include <string>
#include <vector>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
#include "OMPEval-master/omp/HandEvaluator.h"

using namespace pokerbots::skeleton;
using namespace std;
using namespace __gnu_pbds;
using namespace omp;

typedef long double ld;

int evaluate(vector<int> &cards){
    HandEvaluator eval;
    if (cards.size() == 7){
        Hand h = Hand::empty();
        for (auto u: cards)
            h += Hand(u);
        return eval.evaluate(h);
    }
    int ans = 0;
    for (int i=0; i<8; i++){
        Hand h = Hand::empty();
        for (int j=0; j<8; j++) if (j != i)
            h += Hand(cards[j]);
        ans = max(ans, (int)eval.evaluate(h));
    }
    return ans;
}

int string_to_int_card (string &card){
    int ans = 0;
    switch(card[0]){
        case 'A':
            ans = 12;
            break;
        case 'K':
            ans = 11;
            break;
        case 'Q':
            ans = 10;
            break;
        case 'J':
            ans = 9;
            break;
        case 'T':
            ans = 8;
            break;
        case '9':
            ans = 7;
            break;
        case '8':
            ans = 6;
            break;
        case '7':
            ans = 5;
            break;
        case '6':
            ans = 4;
            break;
        case '5':
            ans = 3;
            break;
        case '4':
            ans = 2;
            break;
        case '3':
            ans = 1;
            break;
        case '2':
            ans = 0;
            break;
        default:
            break;
    }
    ans *= 4;
    switch (card[1])
    {
    case 'd':
        ans += 3;
        break;
    case 'c':
        ans += 2;
        break;
    case 'h':
        ans += 1;
        break;
    case 's':
        ans += 0;
        break;
    default:
        break;
    }
    return ans;
}

vector<vector<int>> combinations(vector<int> &arr, int k){
    /*
    Calculates all subsest of size k.
    */

    vector<vector<int>> ans = {};

    if (k == 0){
        vector<int> empty_array;
        ans.push_back(empty_array);
        return ans;
    }

    int n = arr.size();

    vector<int> indices = {};
    for (int i=0; i<k; i++)
        indices.push_back(i);

    vector<int> solution = {};
    for (auto i: indices)
        solution.push_back(arr[i]);
    ans.push_back(solution);

    while (1){
        int idx = 0;
        for (int i=k-1; i>=0; i--){
            if (indices[i] != i + n - k){
                idx = i;
                break;
            }
        }
        if (idx == 0 && indices[0] == n - k)
            break;
        indices[idx] += 1;
        for (int j=idx+1; j<k; j++)
            indices[j] = indices[idx] + j - idx;
        for (int j=0; j<k; j++)
            solution[j] = arr[indices[j]];
        ans.push_back(solution);
    }
    return ans;
}

long double probability_after_auction(array<string, 3> &hole, array<string, 5> &board, int opp_size, int iter = 1000){
    /*
    Finds the probability of winning with a certain hole and board cards

    Arguments:
    hole: a list of the hole cards.
    board: a list of the board cards.
    opp_size: the size of the opponents hand

    Returns:
    probability of winning, tying and losing.
    */

    int strength = 0, total = iter;
    bool deck[52] = {};

    vector<int> hole_int = {};
    for (auto s: hole){
        if (s.size() == 2){
            int x = string_to_int_card(s);
            hole_int.push_back(x);
            deck[x] = 1;
        }
    }

    vector<int> board_int = {};
    for (auto s: board){
        if (s.size() == 2){
            int x = string_to_int_card(s);
            board_int.push_back(x);
            deck[x] = 1;
        }
    }

    vector<int> remaining_deck;
    for (int i=0; i<52; i++)
        if (!deck[i])
            remaining_deck.push_back(i);

    for (int t=0; t<total; t++){
        random_shuffle(remaining_deck.begin(), remaining_deck.end());
        vector<int> my_hand = hole_int;
        vector<int> opp_hand = board_int;
        for (auto u: board_int)
            my_hand.push_back(u);
        for (int i=0; i<5-board_int.size(); i++)
            my_hand.push_back(remaining_deck[i]);
        for (int i=0; i<opp_size+5-board_int.size(); i++)
            opp_hand.push_back(remaining_deck[i]);
        int my_value = evaluate(my_hand);
        int opp_value = evaluate(opp_hand);
        if (my_value > opp_value)
            strength += 2;
        if (my_value == opp_value)
            strength ++;
    }

    return (ld)(strength)/(2 * total);
}

pair<long double, long double> probability_before_auction(array<string, 3> &hole, array<string, 5> &board, int iter = 1000){
    /*
    Finds the probability of winning with a certain hole and board cards

    Arguments:
    hole: a list of the hole cards.
    board: a list of the board cards.
    opp_size: the size of the opponents hand

    Returns:
    probability of winning, tying and losing.
    */

    int strength_w_auction = 0, strength_wo_auction = 0, total = iter;
    bool deck[52] = {};

    vector<int> hole_int = {};
    for (auto s: hole){
        if (s.size() == 2){
            int x = string_to_int_card(s);
            hole_int.push_back(x);
            deck[x] = 1;
        }
    }

    vector<int> board_int = {};
    for (auto s: board){
        if (s.size() == 2){
            int x = string_to_int_card(s);
            board_int.push_back(x);
            deck[x] = 1;
        }
    }

    vector<int> remaining_deck;
    for (int i=0; i<52; i++)
        if (!deck[i])
            remaining_deck.push_back(i);

    for (int t=0; t<total; t++){
        random_shuffle(remaining_deck.begin(), remaining_deck.end());
        vector<int> my_hand = hole_int;
        vector<int> opp_hand = board_int;
        for (auto u: board_int)
            my_hand.push_back(u);
        for (int i=0; i<6-board_int.size(); i++)
            my_hand.push_back(remaining_deck[i]);
        for (int i=1; i<3+5-board_int.size(); i++)
            opp_hand.push_back(remaining_deck[i]);
        int my_value = evaluate(my_hand);
        int opp_value = evaluate(opp_hand);
        if (my_value > opp_value)
            strength_w_auction += 2;
        if (my_value == opp_value)
            strength_w_auction ++;
    }

    for (int t=0; t<total; t++){
        random_shuffle(remaining_deck.begin(), remaining_deck.end());
        vector<int> my_hand = hole_int;
        vector<int> opp_hand = board_int;
        for (auto u: board_int)
            my_hand.push_back(u);
        for (int i=0; i<5-board_int.size(); i++)
            my_hand.push_back(remaining_deck[i]);
        for (int i=0; i<3+5-board_int.size(); i++)
            opp_hand.push_back(remaining_deck[i]);
        int my_value = evaluate(my_hand);
        int opp_value = evaluate(opp_hand);
        if (my_value > opp_value)
            strength_wo_auction += 2;
        if (my_value == opp_value)
            strength_wo_auction ++;
    }
    return make_pair((ld)(strength_w_auction)/(2*total), (ld)(strength_wo_auction)/(2*total));
}

long double probability_brute_force(array<string, 3> &hole, array<string, 5> &board, int opp_size){
    /*
    Finds the probability of winning with a certain hole and board cards

    Arguments:
    hole: a list of the hole cards.
    board: a list of the board cards.
    opp_size: the size of the opponents hand

    Returns:
    probability of winning and losing.
    */

    int win = 0, tie = 0, lose = 0;
    bool deck[52] = {};

    vector<int> hole_int = {};
    for (auto s: hole){
        if (s.size() == 2){
            int x = string_to_int_card(s);
            hole_int.push_back(x);
            deck[x] = 1;
        }
    }

    vector<int> board_int = {};
    for (auto s: board){
        if (s.size() == 2){
            int x = string_to_int_card(s);
            board_int.push_back(x);
            deck[x] = 1;
        }
    }

    vector<int> remaining_deck;
    for (int i=0; i<52; i++)
        if (!deck[i])
            remaining_deck.push_back(i);

    int board_size_left = 5 - board_int.size();
    vector<vector<int>> remaining_combinations = combinations(remaining_deck, board_size_left + opp_size);
    for (auto comb: remaining_combinations){
        vector<vector<int>> board_combinations = combinations(comb, board_size_left);
        for (auto board_comb: board_combinations){
            vector<int> my_hand = hole_int;
            vector<int> opp_hand = board_int;
            for (auto u: board_int)
                my_hand.push_back(u);
            for (auto u: board_comb)
                my_hand.push_back(u);
            for (auto u: comb)
                opp_hand.push_back(u);
            int my_value = evaluate(my_hand);
            int opp_value = evaluate(opp_hand);
            if (my_value > opp_value)
                win ++;
            if (my_value == opp_value)
                tie ++;
            if (my_value < opp_value)
                lose ++;
        }
    }
    return (ld)(win)/(win + tie + lose);
}

struct Bot {
    /*
        Called when a new round starts. Called NUM_ROUNDS times.

        @param gameState The GameState object.
        @param roundState The RoundState object.
        @param active Your player's index.
    */
    
    long double min_win_to_raise = 0.5;
    long double MAX_RAISE = 0.8;
    int action_number = 1;
    long double weak_to_mid = 0.25;
    long double mid_to_strong = 0.7;
    long double bluff_probability = 0.05;
    string last_action = "none"; // "raise" for raise, "call" for call, etc... "none" by default.
    int opp_weakfolds = 0;
    long double intimidation = 0.15;
    bool already_won = false;
    bool almost_won = false;
    int won_at_round = -1;
    tree<int, null_type, less_equal<int>, rb_tree_tag, tree_order_statistics_node_update> opp_bids;
    int last_pot = 0;
    long double last_strength = 0;
    int opp_fold_small_blind_preflop = 0;
    bool last_action_is_bluff = false; // True if our last action was a bluff
    long double raise_weight = 1; // Weight of raise, depends on opp behavior
    int opp_call_counter = 0;
    int opp_raise_counter = 0;
    bool big_blind = false;
    int my_bankroll = 0;
    int round_num = 0;
    int opp_size = 0;
    int my_size = 0;
    optional<int> my_bid = 0;
    optional<int> opp_bid = 0;
    int num_raise = 0;
    int opp_raise = 0;
    bool acted = false;
    bool strong_hole = false;
    bool mid_hole = false;
    bool weak_hole = false;
    int street = 0;
    int my_pip = 0;
    int opp_pip = 0;
    int continue_cost = 0;
    int min_raise = 0;
    int max_raise = 0;
    int pot = 0;
    mt19937 generator;
    normal_distribution<long double> normalDistribution{1.0, 0.1};
    uniform_real_distribution<long double> uniformDistribution{0.0, 1.0};

    int OnlyFold(){
        /*
        Calculates the amount of money lost if we fold every time.
        */
        int remaining_rounds = NUM_ROUNDS - round_num + 1;
        int fold_amount = (remaining_rounds / 2) * (SMALL_BLIND + BIG_BLIND);
        if ((remaining_rounds % 2) == 1){
            if (big_blind)
                fold_amount += BIG_BLIND;
            else
                fold_amount += SMALL_BLIND;
        }
        return fold_amount;
    }

    int AlmostWon(){
        /*
        Assumes opponent keeps folding in small blind and checks if we almost won
        */
        if (my_bankroll < 200 || round_num <= 2)
            return 100000;
        int remaining_rounds = NUM_ROUNDS - round_num + 1;
        int remaining_small = remaining_rounds / 2;
        int remaining_big = remaining_rounds / 2;
        if ((remaining_rounds % 2) == 1){
            if (big_blind)
                remaining_big ++;
            else
                remaining_small ++;
        }
        long double opp_prob_fold_small = (ld) opp_fold_small_blind_preflop / (NUM_ROUNDS / 2 - remaining_big);
        return (ld)SMALL_BLIND * remaining_small + (ld)BIG_BLIND * remaining_big * (1 - (ld)opp_prob_fold_small) - (ld)SMALL_BLIND * remaining_big * opp_prob_fold_small;
    }

    void handleNewRound(GameInfoPtr gameState, RoundStatePtr roundState, int active) {

        cout << "----------ROUND " << gameState->roundNum << " -----------\n";

        my_bankroll = gameState->bankroll;  // the total number of chips you've gained or lost from the beginning of the game to the start of this round
        long double game_clock = gameState->gameClock;  // the total number of seconds your bot has left to play this game
        round_num = gameState->roundNum;  // the round number from 1 to NUM_ROUNDS
        auto my_cards = roundState->hands[active];  // your cards
        big_blind = (active == 1);  // True if you are the big blind
        opp_size = 2;
        my_bid = 0;
        opp_bid = 0;
        num_raise = 0;
        opp_raise = 0;
        last_pot = BIG_BLIND + SMALL_BLIND;
        last_strength = 0;
        acted = false;
        action_number = 1; // resets action number

        string card1 = my_cards[0];
        string card2 = my_cards[1];

        char rank1 = card1[0]; // "Ad", "9c", "Th" -> "A", "9", "T"
        char suit1 = card1[1]; // "d", "c", "h", etc.
        char rank2 = card2[0];
        char suit2 = card2[1];

        strong_hole = false;
        mid_hole = false;
        weak_hole = false;
        string strong_rank = "AKQJT9";
        string mid_rank = "AKQJT98";
        if (rank1 == rank2 || (strong_rank.find(rank1) != string::npos && strong_rank.find(rank2) != string::npos))
            strong_hole = true;
        else{
            if (suit1 == suit2 && (mid_rank.find(rank1) != string::npos || mid_rank.find(rank2) != string::npos))
                mid_hole = true;
            else
                weak_hole = true;
        }

        if (!already_won && my_bankroll > OnlyFold()){
            already_won = true;
            won_at_round = round_num - 1;
        }

        almost_won = (my_bankroll > AlmostWon());
    }

    /*
        Called when a round ends. Called NUM_ROUNDS times.

        @param gameState The GameState object.
        @param terminalState The TerminalState object.
        @param active Your player's index.
    */
    void handleRoundOver(GameInfoPtr gameState, TerminalStatePtr terminalState, int active) {

        cout << "------------END OF ROUND------------------\n";

        int my_delta = terminalState->deltas[active];  // your bankroll change from this round
        auto previous_state = static_pointer_cast<const RoundState>(terminalState->previousState);  // RoundState before payoffs
        street = previous_state->street;  // 0, 3, 4, or 5 representing when this round ended
        auto my_cards = previous_state->hands[active];  // your cards
        auto opp_cards = previous_state->hands[1-active];  // opponent's cards or [] if not revealed

        bool change_bluff = false;

        if (last_action == "raise" && last_pot == 2 * STARTING_STACK - previous_state->stacks[0] - previous_state->stacks[1] && action_number > 2){ // In case the opponent folded
            opp_weakfolds ++;
            if (num_raise == 1){
                change_bluff = true;
                bluff_probability += 0.001 * normalDistribution(generator);
            }
            else if (num_raise > 1){
                change_bluff = true;
                bluff_probability -= 0.001 * normalDistribution(generator);
            }
            if (!last_action_is_bluff){ // Opponent folded when we did not bluff
                raise_weight -= 0.04 * normalDistribution(generator);
                cout << "Opp Folded No Bluff\n";
            }
        }
        else if (num_raise >= 1){
            change_bluff = true;
            bluff_probability -= 0.001 * normalDistribution(generator);
        }

        if (last_strength > mid_to_strong){
            if (my_delta > 0)
                mid_to_strong -= 0.005 * normalDistribution(generator);
            if (my_delta < 0)
                mid_to_strong += 0.025 * normalDistribution(generator);
        }

        bluff_probability = min((ld)0.01, bluff_probability);
        bluff_probability = max(bluff_probability, (ld)0.001);
        mid_to_strong = min((ld)0.8, mid_to_strong);
        mid_to_strong = max((ld)0.7, mid_to_strong);

        intimidation += (0.5 - opp_raise) * 0.001 * normalDistribution(generator);
        intimidation = min(intimidation, (ld)0.3);
        intimidation = max(intimidation, (ld)0.05);

        raise_weight = min(raise_weight, (ld)1.1);
        raise_weight = max(raise_weight, (ld)0.85);

        if (opp_bid.has_value())
            opp_bids.insert(opp_bid.value());

        if (!acted)
            opp_fold_small_blind_preflop += 1;

        last_action = "none";
        if (change_bluff)
            cout << "Bluff changed\n";
        cout << "Intervals: " << weak_to_mid << ' ' << mid_to_strong << '\n';
        cout << "Intimidation: " << intimidation << '\n';
        cout << "Bluff: " << bluff_probability << '\n';
        cout << "Opp Folds: " << opp_weakfolds << '\n';
        cout << "Raise Weight: " << raise_weight << '\n';
        cout << '\n';
        if (gameState->roundNum == NUM_ROUNDS){
            cout << "Time left: " << gameState->gameClock << '\n';
            cout << "Won at round: " << won_at_round << '\n';
        }
    }
    
    /*
        Where the magic happens - your code should implement this function.
        Called any time the engine needs an action from your bot.

        @param gameState The GameState object.
        @param roundState The RoundState object.
        @param active Your player's index.
        @return Your action.
    */

    Action Act(string action, int amount=0){
        /*
        Returns a valid action object and resets bot's last action.

        Argument:
        action: string ("raise" for RaiseAction, "call" for CallAction, "check" for CheckAction", "bid" for BidAction and "fold" for FoldAction)
        amount: (Optional) integer to bid/raise in case of RaiseAction / BidAction.

        Returns:
        Desired action.
        */

        cout << "Action : " << action << "/ Stage " << action_number << '\n';

        last_action = action;
        if (action == "raise"){
            num_raise += 1;
            last_pot += amount - my_pip;
            return {Action::Type::RAISE, amount};
        }
        if (action == "bid")
            return {Action::Type::BID, amount};
        if (action == "fold")
            return {Action::Type::FOLD};
        if (action == "call"){
            last_pot += continue_cost;
            return {Action::Type::CALL};
        }
        return {Action::Type::CHECK};
    }

    int CalculateRaise(long double strength_for_raise, bool bluff = false){
        long double amount_prob = (strength_for_raise - mid_to_strong) / ((ld)1 - mid_to_strong);
        long double raise_ammt = ((ld)min_raise + ((ld)max_raise*MAX_RAISE-(ld)min_raise)*amount_prob)*normalDistribution(generator);
        if (bluff)
            raise_ammt = max(raise_ammt, (ld)my_pip + (ld)pot * 0.55 * normalDistribution(generator));
        else
            raise_ammt = raise_weight * min(raise_ammt, (ld)my_pip + (ld)pot * 0.45 * normalDistribution(generator));
        int raise_ammt_int = round(raise_ammt);
        raise_ammt_int = min(raise_ammt_int, max_raise);
        raise_ammt_int = max(raise_ammt_int, min_raise);
        return raise_ammt_int;
    }

    void CalcIntimidation(){
        if (action_number == 3 && opp_raise_counter > 0)
            intimidation += 0.003*opp_raise_counter;
        if (action_number == 4 && opp_raise_counter > 0)
            intimidation += 0.001*opp_raise_counter;
        if (action_number == 5 && opp_raise_counter > 0)
            intimidation += 0.0005 *opp_raise_counter;
    }

    Action getAction(GameInfoPtr gameState, RoundStatePtr roundState, int active) {

        auto legal_actions = roundState->legalActions();  // the actions you are allowed to take
        street = roundState->street;  // 0, 3, 4, or 5 representing pre-flop, flop, turn, or river respectively
        auto my_cards = roundState->hands[active];  // your cards
        auto board_cards = roundState->deck;  // the board cards
        my_pip = roundState->pips[active];  // the number of chips you have contributed to the pot this round of betting
        opp_pip = roundState->pips[1-active];  // the number of chips your opponent has contributed to the pot this round of betting
        int my_stack = roundState->stacks[active];  // the number of chips you have remaining
        int opp_stack = roundState->stacks[1-active];  // the number of chips your opponent has remaining
        my_bid = roundState->bids[active];  // How much you bid previously (available only after auction)
        opp_bid = roundState->bids[1-active];  // How much opponent bid previously (available only after auction)
        continue_cost = opp_pip - my_pip;  // the number of chips needed to stay in the pot
        int my_contribution = STARTING_STACK - my_stack;  // the number of chips you have contributed to the pot
        int opp_contribution = STARTING_STACK - opp_stack;  // the number of chips your opponent has contributed to the pot
        pot = my_contribution + opp_contribution;
        last_pot = pot;
        acted = true;
        bool last_action_bluff_previous = last_action_is_bluff;
        last_action_is_bluff = false;

        if (last_action == "raise" &&  !last_action_bluff_previous)
            raise_weight += 0.02 * normalDistribution(generator);

        if (legal_actions.find(Action::Type::CALL) != legal_actions.end() && pot > SMALL_BLIND + BIG_BLIND)
            opp_raise ++;

        if (legal_actions.find(Action::Type::BID) != legal_actions.end())
            action_number = 2; // Bidding stage
        else if (street > 0)
            action_number = street; // 3 for flop after bid, 4 for turn, 5 for river.

        CalcIntimidation();
        // post flop raise and call counter
        if (legal_actions.find(Action::Type::CALL) != legal_actions.end() && action_number > 2)
            opp_raise_counter ++;
        if (last_action == "raise" && action_number > 2)
            opp_call_counter ++;

        if (already_won || almost_won){
            if (legal_actions.find(Action::Type::CHECK) != legal_actions.end())
                return Act("check");
            if (legal_actions.find(Action::Type::FOLD) != legal_actions.end())
                return Act("fold");
            return Act("bid", 0);
        }

        if (legal_actions.find(Action::Type::RAISE) != legal_actions.end()){
            auto raise = roundState->raiseBounds();
            min_raise = raise[0];
            max_raise = raise[1];
            int min_cost = min_raise - my_pip;  // the cost of a minimum bet/raise
            int max_cost = max_raise - my_pip;  // the cost of a maximum bet/raise
        }

        int auction_trigger = int(action_number >= 2); // Did auction happen yet ?

        my_size = 2;
        if (my_cards[2].size() == 2)
            my_size = 3;
        opp_size = 4 + auction_trigger - my_size; // correct opponent size (assuming no tie)

        //Bot Behavior (In Round)

        if (action_number == 1){ // bet before flop
            if (legal_actions.find(Action::Type::CHECK) != legal_actions.end())
                return Act("check");
            if (!big_blind && pot == BIG_BLIND + SMALL_BLIND && legal_actions.find(Action::Type::RAISE) != legal_actions.end() && min_raise <= 2*BIG_BLIND && 2*BIG_BLIND <= max_raise){
                if (strong_hole && uniformDistribution(generator) < 0.3){
                    num_raise -= 1;
                    return Act("raise", 2*BIG_BLIND);
                }
                if (mid_hole && uniformDistribution(generator) < 0.6){
                    num_raise -= 1;
                    return Act("raise", 2*BIG_BLIND);
                }
            }
            if (strong_hole)
                return Act("call");
            return Act("fold");
        }

        if (action_number == 2){ // Auction
            pair<long double, long double> joint_strength = probability_before_auction(my_cards, board_cards);
            long double strength_w_auction = joint_strength.first;
            long double strength_wo_auction = joint_strength.second;
            if (opp_bids.size() > 10){
                if (strength_w_auction < mid_to_strong){
                    int idx = rand() % (opp_bids.size()/2);
                    int bid = *opp_bids.find_by_order(idx);
                    bid -= 5;
                    bid = max (bid, 0);
                    bid = min (bid, my_stack);
                    return Act("bid", bid);
                }
                else{
                    int idx = opp_bids.size()/2 + rand() % (opp_bids.size()/2);
                    int bid = *opp_bids.find_by_order(idx);
                    bid += 5;
                    bid = max (bid, 0);
                    bid = min (bid, my_stack);
                    return Act("bid", bid);
                }
            }
            if (strength_w_auction < mid_to_strong){
                int bid = round(12 * normalDistribution(generator));
                bid = max (bid, 0);
                bid = min (bid, my_stack);
                return Act("bid", bid);
            }
            int MAX_AUCTION = my_stack;
            long double SCALAR_AUCTION = 1.5;
            int bid = round((ld)MAX_AUCTION * (strength_w_auction - strength_wo_auction) * pow(strength_w_auction, 8) * normalDistribution(generator) * SCALAR_AUCTION);
            bid = max(0, bid);
            bid = min(bid, my_stack);
            cout << "BID " << strength_wo_auction << ' ' << strength_w_auction << ' ' << bid << '\n';
            return Act("bid", bid);
        }

        long double strength = 0.5;
        if (action_number == 5 && gameState->gameClock > 10)
            strength = probability_brute_force(my_cards, board_cards, opp_size);
        else
            strength = probability_after_auction(my_cards, board_cards, opp_size);

        last_strength = strength;

        cout << "Strength at round " << action_number << " is " << strength << '\n';

        long double raise_ammt = 0;
        if (legal_actions.find(Action::Type::RAISE) != legal_actions.end()){
            if (strength < weak_to_mid)
                raise_ammt = CalculateRaise(uniformDistribution(generator)*(1-mid_to_strong) + mid_to_strong, true);
            else
                raise_ammt = CalculateRaise(strength, false);
        }

        string commit_action = "";
        long double commit_amount = 0;
        string my_action = "";
        long double my_amount = 0;

        if (legal_actions.find(Action::Type::RAISE) != legal_actions.end()){
            commit_action = "raise";
            commit_amount = raise_ammt;
        }
        else{
            if (legal_actions.find(Action::Type::CALL) != legal_actions.end() && continue_cost <= my_stack)
                commit_action = "call";
            else
                commit_action = "fold";
        }

        if (continue_cost > 0){
            long double pot_odds = (ld)continue_cost/(continue_cost + pot);

            if ((ld)continue_cost/pot > 0.25)
                strength -= intimidation;

            if (strength >= pot_odds){
                if (uniformDistribution(generator) < strength && strength > mid_to_strong){
                    my_action = commit_action;
                    my_amount = commit_amount;
                }
                else
                    my_action = "call";
            }
            else{
                if (strength < weak_to_mid && uniformDistribution(generator) < bluff_probability && legal_actions.find(Action::Type::RAISE) != legal_actions.end()){
                    my_action = commit_action;
                    my_amount = commit_amount;
                    last_action_is_bluff = true;
                }
                else
                    my_action = "fold";
            }
        }
        else{
            if (strength > mid_to_strong && uniformDistribution(generator) < strength){
                my_action = commit_action;
                my_amount = commit_amount;
            }
            else{
                if (strength < weak_to_mid && uniformDistribution(generator) < bluff_probability && legal_actions.find(Action::Type::RAISE) != legal_actions.end()){
                    my_action = commit_action;
                    my_amount = commit_amount;
                    last_action_is_bluff = true;
                }
                else
                    my_action = "check";
            }
        }
        return Act(my_action, my_amount);
    }
};

/*
  Main program for running a C++ pokerbot.
*/
int main(int argc, char *argv[]) {
    ios_base::sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);
    srand(time(0));
    auto [host, port] = parseArgs(argc, argv);
    runBot<Bot>(host, port);
    return 0;
}
