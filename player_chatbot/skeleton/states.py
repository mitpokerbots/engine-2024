'''
Encapsulates game and round state information for the player.
'''
from collections import namedtuple
from .actions import FoldAction, CallAction, CheckAction, RaiseAction, BidAction

GameState = namedtuple('GameState', ['bankroll', 'game_clock', 'round_num'])
TerminalState = namedtuple('TerminalState', ['deltas', 'bids', 'previous_state'])

NUM_ROUNDS = 1000
STARTING_STACK = 400
BIG_BLIND = 2
SMALL_BLIND = 1


class RoundState(namedtuple('_RoundState', ['button', 'street', 'auction', 'bids', 'pips', 'stacks', 'hands', 'deck', 'previous_state'])):
    '''
    Encodes the game tree for one round of poker.
    '''

    def showdown(self):
        '''
        Compares the players' hands and computes payoffs.
        '''
        return TerminalState([0, 0], self.bids, self)

    def legal_actions(self):
        if self.auction:
            return {BidAction}
        '''
        Returns a set which corresponds to the active player's legal moves.
        '''
        active = self.button % 2
        continue_cost = self.pips[1-active] - self.pips[active]
        if continue_cost == 0:
            # we can only raise the stakes if both players can afford it
            bets_forbidden = (self.stacks[0] == 0 or self.stacks[1] == 0)
            return {CheckAction} if bets_forbidden else {CheckAction, RaiseAction}
        # continue_cost > 0
        # similarly, re-raising is only allowed if both players can afford it
        raises_forbidden = (continue_cost >= self.stacks[active] or self.stacks[1-active] == 0)
        return {FoldAction, CallAction} if raises_forbidden else {FoldAction, CallAction, RaiseAction}

    def raise_bounds(self):
        '''
        Returns a tuple of the minimum and maximum legal raises.
        '''
        active = self.button % 2
        continue_cost = self.pips[1-active] - self.pips[active]
        max_contribution = min(self.stacks[active], self.stacks[1-active] + continue_cost)
        min_contribution = min(max_contribution, continue_cost + max(continue_cost, BIG_BLIND))
        return (self.pips[active] + min_contribution, self.pips[active] + max_contribution)

    def proceed_street(self):
        '''
        Resets the players' pips and advances the game tree to the next round of betting.
        '''
        if self.street == 5:
            return self.showdown()
        if self.street == 0:
            return RoundState(1, 3, True, self.bids, [0, 0], self.stacks, self.hands, self.deck, self)
        return RoundState(1, self.street + 1, False, self.bids, [0, 0], self.stacks, self.hands, self.deck, self)


    def proceed(self, action):
        '''
        Advances the game tree by one action performed by the active player.
        '''
        active = self.button % 2
        if isinstance(action, FoldAction):
            delta = self.stacks[0] - STARTING_STACK if active == 0 else STARTING_STACK - self.stacks[1]
            return TerminalState([delta, -delta], self.bids, self)
        if isinstance(action, CallAction):
            if self.button == 0:  # sb calls bb
                return RoundState(1, 0, self.auction, self.bids, [BIG_BLIND] * 2, [STARTING_STACK - BIG_BLIND] * 2, self.hands, self.deck, self)
            # both players acted
            new_pips = list(self.pips)
            new_stacks = list(self.stacks)
            contribution = new_pips[1-active] - new_pips[active]
            new_stacks[active] -= contribution
            new_pips[active] += contribution
            state = RoundState(self.button + 1, self.street, self.auction, self.bids, new_pips, new_stacks, self.hands, self.deck, self)
            return state.proceed_street()
        if isinstance(action, CheckAction):
            if (self.street == 0 and self.button > 0) or self.button > 1:  # both players acted
                return self.proceed_street()
            # let opponent act
            return RoundState(self.button + 1, self.street, self.auction, self.bids, self.pips, self.stacks, self.hands, self.deck, self)
        if isinstance(action, BidAction):
            self.bids[active] = action.amount
            if None not in self.bids:       # both players have submitted bids and we deal the extra card
                # case in which bids are equal, both players receive card
                if self.bids[0] == self.bids[1]:
                    new_stacks = list(self.stacks)
                    new_stacks[0] -= self.bids[0]
                    new_stacks[1] -= self.bids[1]
                    # print(new_stacks)
                    state = RoundState(1, self.street, False, self.bids, self.pips, new_stacks, self.hands, self.deck, self)
                else:
                # case in which bids are not equal
                    winner = self.bids.index(max(self.bids))
                    new_stacks = list(self.stacks)
                    new_stacks[winner] -= self.bids[1 - winner]
                    state = RoundState(1, self.street, False, self.bids, self.pips, new_stacks, self.hands, self.deck, self)
                return state
            else:
                return RoundState(self.button + 1, self.street, True, self.bids, self.pips, self.stacks, self.hands, self.deck, self)
        # isinstance(action, RaiseAction)
        new_pips = list(self.pips)
        new_stacks = list(self.stacks)
        contribution = action.amount - new_pips[active]
        new_stacks[active] -= contribution
        new_pips[active] += contribution
        return RoundState(self.button + 1, self.street, self.auction, self.bids, new_pips, new_stacks, self.hands, self.deck, self)