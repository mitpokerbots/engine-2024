'''
Simple example pokerbot, written in Python.
'''
from skeleton.actions import FoldAction, CallAction, CheckAction, RaiseAction, BidAction
from skeleton.states import GameState, TerminalState, RoundState
from skeleton.states import NUM_ROUNDS, STARTING_STACK, BIG_BLIND, SMALL_BLIND
from skeleton.bot import Bot
from skeleton.runner import parse_args, run_bot
import random
import eval7


class Player(Bot):
    '''
    A pokerbot.
    '''

    def __init__(self):
        '''
        Called when a new game starts. Called exactly once.

        Arguments:
        Nothing.

        Returns:
        Nothing.
        '''
        pass

    def handle_new_round(self, game_state, round_state, active):
        '''
        Called when a new round starts. Called NUM_ROUNDS times.

        Arguments:
        game_state: the GameState object.
        round_state: the RoundState object.
        active: your player's index.

        Returns:
        Nothing.
        '''
        my_bankroll = game_state.bankroll  # the total number of chips you've gained or lost from the beginning of the game to the start of this round
        game_clock = game_state.game_clock  # the total number of seconds your bot has left to play this game
        round_num = game_state.round_num  # the round number from 1 to NUM_ROUNDS
        my_cards = round_state.hands[active]  # your cards
        big_blind = bool(active)  # True if you are the big blind


        monte_carlo_iters = 100
        game_clock = game_state.game_clock
        round_num = game_state.round_num

        self.strength = self.calculate_strength(my_cards, monte_carlo_iters)

        #cnan use this to see number of iterations we can squeeze in
        if round_num == NUM_ROUNDS:
            print(game_clock)


        pass

    def calculate_strength(self, hole, iters):
        deck = eval7.Deck()
        hole_cards = [eval7.Card(card) for card in hole]
        for card in hole_cards:
            deck.cards.remove(card)

        score_with_card = 0
        score_without_card = 0

        
        for _ in range(iters):
            deck.shuffle()

            com = 5
            opp = 3
            draw = deck.peek(com + opp)

            opp_hole = draw[:opp]
            community_cards = draw[opp:]
            our_hand = hole_cards + community_cards
            opp_hand = opp_hole + community_cards
            our_hand_value = eval7.evaluate(our_hand)
            opp_hand_value = eval7.evaluate(opp_hand)

            if our_hand_value > opp_hand_value:
                    #we win
                score_without_card += 2
            elif our_hand_value == opp_hand_value:
                #we tie
                score_without_card += 1
            else:
                #we lost
                score_without_card += 0

        for _ in range(iters):
            deck.shuffle()

            ours = 1
            com = 5
            opp = 2
            draw = deck.peek(com + opp + ours)

            opp_hole = draw[:opp]
            community_cards = draw[opp:opp+com]
            our_draw = draw[opp+com:]

            our_hand = hole_cards + our_draw + community_cards
            opp_hand = opp_hole + community_cards
            our_hand_value = eval7.evaluate(our_hand)
            opp_hand_value = eval7.evaluate(opp_hand)

            if our_hand_value > opp_hand_value:
                #we win
                score_with_card += 2
            elif our_hand_value == opp_hand_value:
                #we tie
                score_with_card += 1
            else:
                #we lost
                score_with_card += 0

        strength_without_card = score_without_card/(2*iters)
        strength_with_card = score_with_card/(2*iters)

        avg_strength = (strength_with_card + strength_without_card)/2

        print(avg_strength)
        return avg_strength



    def handle_round_over(self, game_state, terminal_state, active):
        '''
        Called when a round ends. Called NUM_ROUNDS times.

        Arguments:
        game_state: the GameState object.
        terminal_state: the TerminalState object.
        active: your player's index.

        Returns:
        Nothing.
        '''

        #my_delta = terminal_state.deltas[active]  # your bankroll change from this round
        #previous_state = terminal_state.previous_state  # RoundState before payoffs
        #street = previous_state.street  # 0, 3, 4, or 5 representing when this round ended
        #my_cards = previous_state.hands[active]  # your cards
        #opp_cards = previous_state.hands[1-active]  # opponent's cards or [] if not revealed
        pass

    def get_action(self, game_state, round_state, active):
        '''
        Where the magic happens - your code should implement this function.
        Called any time the engine needs an action from your bot.

        Arguments:
        game_state: the GameState object.
        round_state: the RoundState object.
        active: your player's index.

        Returns:
        Your action.
        '''
        # May be useful, but you may choose to not use.
        legal_actions = round_state.legal_actions()  # the actions you are allowed to take
        street = round_state.street  # 0, 3, 4, or 5 representing pre-flop, flop, turn, or river respectively
        my_cards = round_state.hands[active]  # your cards
        board_cards = round_state.deck[:street]  # the board cards
        my_pip = round_state.pips[active]  # the number of chips you have contributed to the pot this round of betting
        opp_pip = round_state.pips[1-active]  # the number of chips your opponent has contributed to the pot this round of betting
        my_stack = round_state.stacks[active]  # the number of chips you have remaining
        opp_stack = round_state.stacks[1-active]  # the number of chips your opponent has remaining
        my_bid = round_state.bids[active]  # How much you bid previously (available only after auction)
        opp_bid = round_state.bids[1-active]  # How much opponent bid previously (available only after auction)
        continue_cost = opp_pip - my_pip  # the number of chips needed to stay in the pot
        my_contribution = STARTING_STACK - my_stack  # the number of chips you have contributed to the pot
        opp_contribution = STARTING_STACK - opp_stack  # the number of chips your opponent has contributed to the pot
        pot = my_contribution + opp_contribution
       
        strength = self.strength

        
        min_raise, max_raise = round_state.raise_bounds()  # the smallest and largest numbers of chips for a legal bet/raise

        if street < 3:
            raise_amt = int(my_pip + continue_cost + (0.4)*(pot+continue_cost))
        else:
            raise_amt = int(my_pip + continue_cost + (0.4)*(pot+continue_cost))

        raise_amt = max(min_raise,raise_amt)
        raise_amt = min(max_raise, raise_amt)
        raise_cost = raise_amt - my_pip

        if RaiseAction in legal_actions and raise_cost <= my_stack:
            commit_action = RaiseAction(raise_amt)
                        
        elif CallAction in legal_actions:
            commit_action = CallAction()
        else:
            commit_action = CheckAction()

        if continue_cost > 0:
            pot_odds = continue_cost/(pot + continue_cost)

            if strength >= pot_odds:
                if strength > 0.5 and random.random() < strength:
                    my_action = commit_action
                else:
                    my_action = CallAction()
                
            else:
                if random.random() < strength:
                        my_action = commit_action
                else:
                     my_action = CheckAction()
        else: # board continue cost == 0, so its our action
            if random.random() < strength:
                my_action = commit_action
            else:
                my_action = CheckAction()

        return my_action


        # if RaiseAction in legal_actions:
        #    min_raise, max_raise = round_state.raise_bounds()  # the smallest and largest numbers of chips for a legal bet/raise
        #    min_cost = min_raise - my_pip  # the cost of a minimum bet/raise
        #    max_cost = max_raise - my_pip  # the cost of a maximum bet/raise
        
        # if RaiseAction in legal_actions and random.random() < 0.3:
        #     return RaiseAction(random.randint(min_raise, max_raise))
        # if CheckAction in legal_actions:
        #     return CheckAction()
        # elif BidAction in legal_actions:
        #     return BidAction(my_stack) # random bid between 0 and our stack
        # return CallAction()


if __name__ == '__main__':
    run_bot(Player(), parse_args())
