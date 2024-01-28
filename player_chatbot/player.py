'''
Simple example pokerbot, written in Python.
'''
from skeleton.actions import FoldAction, CallAction, CheckAction, RaiseAction, BidAction
from skeleton.states import GameState, TerminalState, RoundState
from skeleton.states import NUM_ROUNDS, STARTING_STACK, BIG_BLIND, SMALL_BLIND
from skeleton.bot import Bot
from skeleton.runner import parse_args, run_bot
import random


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
        self.bid_info = False

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
        #my_bankroll = game_state.bankroll  # the total number of chips you've gained or lost from the beginning of the game to the start of this round
        game_clock = game_state.game_clock  # the total number of seconds your bot has left to play this game
        self.bid_info = True
        #round_num = game_state.round_num  # the round number from 1 to NUM_ROUNDS
        #my_cards = round_state.hands[active]  # your cards
        big_blind = bool(active)  # True if you are the big blind
        print("================================NEW ROUND===================================")
        print("You are", "big blind!" if big_blind else "small blind!")

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
        my_delta = terminal_state.deltas[active]  # your bankroll change from this round
        previous_state = terminal_state.previous_state  # RoundState before payoffs
        #street = previous_state.street  # 0, 3, 4, or 5 representing when this round ended
        #my_cards = previous_state.hands[active]  # your cards
        opp_cards = previous_state.hands[1-active]  # opponent's cards or [] if not revealed
        print()
        if opp_cards:
            print("Your opponent revealed", ', '.join(opp_cards))
        
        print("This round, your bankroll changed by", str(my_delta)+'!')
        print()
        ask = input("Press enter to continue, or q to quit!\n")
        if ask in ['q', 'quit', 'Quit']:
            exit()

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

        print()
        print("Your current cards are:", ", ".join(my_cards))
        if board_cards:
            print("The visible community cards are:", ", ".join(board_cards))
        else:
            print("There are no visible community cards.")
        print("Your current contribution to the pot is", my_contribution)
        print("Your remaining stack is", my_stack)

        if self.bid_info and None not in round_state.bids:
            self.bid_info = False
            if opp_bid > my_bid:
                print("Your opponent won the auction by bidding", opp_bid)
            elif my_bid > opp_bid:
                print("Your won the auction, your opponent bid", opp_bid)
            else:
                print("You and your opponent both bid the same amount!")

        if my_contribution != 1 and continue_cost > 0:
            print("Your opponent raised by", continue_cost)

        poss_actions = "Your legal actions are: "
        if RaiseAction in legal_actions:
            poss_actions += "Raise, "
        if FoldAction in legal_actions:
            poss_actions += "Fold, "
        if CallAction in legal_actions:
            poss_actions += "Call, "
        if CheckAction in legal_actions:
            poss_actions += "Check, "
        if BidAction in legal_actions:
            poss_actions += "Bid, "
        print(poss_actions[:-2] + '.\n')
        
        if RaiseAction in legal_actions:
           min_raise, max_raise = round_state.raise_bounds()  # the smallest and largest numbers of chips for a legal bet/raise
           min_cost = min_raise - my_pip  # the cost of a minimum bet/raise
           max_cost = max_raise - my_pip  # the cost of a maximum bet/raise
        
        active = input("Enter your move:\n")
        act = None
        while act is None:
            active = active.split(" ")
            if active[0] in ["Quit", 'quit', 'q']:
                exit()
            if (len(active) != 1 and len(active) != 2):
                active = input("Too many words. Re-enter move: \n")
            elif len(active) == 1:
                act = active[0]
                if act not in ['Check', 'Fold', 'Call']:
                    act = None
                    active = input("One-word moves are only Check, Fold and Call. Re-enter move: \n")
            else:
                act, num = active
                if act not in ['Bid', 'Raise']:
                    act = None
                    active = input("Two-word moves are only Bid and Raise. Re-enter move: \n")
                try:
                    num = int(num)
                except:
                    act = None
                    active = input("Integer not entered for Raising or Bidding. Enter new move: \n")
        
        if act == "Raise":
            return RaiseAction(num)
        elif act == "Check":
            return CheckAction()
        elif act == "Bid":
            return BidAction(num)
        elif act == "Call":
            return CallAction()
        else:
            return FoldAction()


if __name__ == '__main__':
    run_bot(Player(), parse_args())
