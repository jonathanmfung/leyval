import json

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.animation as animation

DATA_FILE = "../data/pretty.json"
IMG_DIR = "img/"

FIGSIZE = (8, 4.5)
DPI = 200

class LeyvalPlotter:
    def __init__(self, data_file):
        self.data_file = data_file
        self._agents_raw = []
        self._book_raw = []
        self.agents_clean: pd.DataFrame
        self.book_clean: pd.DataFrame

    @staticmethod
    def _read_agents(agents_json):
        return pd.json_normalize(agents_json)

    @staticmethod
    def _read_book(book_json):
        bids = pd.Series([x[1] for x in book_json['bid_counts']],
                         index = [x[0][0] for x in book_json['bid_counts']],
                         name='bids')
        asks = pd.Series([x[1] for x in book_json['ask_counts']],
                         index = [x[0][0] for x in book_json['ask_counts']],
                         name='asks')
        return pd.concat([bids, asks], keys=['bids', 'asks'])

    # TODO: Split read_raw and clean_data for agents/book
    def read_raw(self):
        with open(self.data_file, 'r') as f:
            raw_json = json.load(f)

        for run_tick in raw_json:
            self._agents_raw.append(self._read_agents(run_tick['agents']))
            self._book_raw.append(self._read_book(run_tick['order_book']))
        print("RAW DATA READ")

    def clean_data(self):
        self.agents_clean = pd.concat(self._agents_raw,
                                      keys = range(len(self._agents_raw)))\
                              .drop(columns='id')
        self.agents_clean.index = self.agents_clean.index.set_names(['time', 'id'])

        self.book_clean = pd.concat(self._book_raw, axis=1)\
                            .T.fillna(0).astype("Int64")
        self.book_clean.index = self.book_clean.index.set_names('time')
        self.book_clean = self.book_clean.sort_index(ascending=[False, True], axis=1)
        print("DATA CLEANED")

    def plot_book(self):
        fig, ax = plt.subplots(figsize=FIGSIZE, dpi=DPI, layout='constrained')

        bids_hist = ax.bar(x=lp.book_clean.loc[0]['bids'].index, height=lp.book_clean.loc[0]['bids'],
                           color='green', linewidth=0, label='Bids', animated=True)
        asks_hist = ax.bar(x=lp.book_clean.loc[0]['asks'].index, height=lp.book_clean.loc[0]['asks'],
                           color='red', linewidth=0, label='Asks', animated=True)
        title = ax.text(x=0.5, y=0.8, s="Tick #0", ha='center', transform=ax.transAxes, animated=True)

        # TODO: Format x-axis labels as decimal (10020 -> 100.20)

        # TODO: Add proper title (Simulation Params: n_providers, n_takers)
        def init():
            ax.set(ylim=(0, lp.book_clean.max().max() * 1.1),
                   xlabel='Price ($)', ylabel='Quantity')
            # TODO: Make y-axis integers
            ax.legend()
            return [r for r in bids_hist] + [r for r in asks_hist] + [title]

        def update(frame_num):
            title.set_text(f"Tick #{frame_num}")
            for rect, quantity in zip(bids_hist.patches, lp.book_clean.loc[frame_num]['bids']):
                rect.set_height(quantity)
            for rect, quantity in zip(asks_hist.patches, lp.book_clean.loc[frame_num]['asks']):
                rect.set_height(quantity)
            return [r for r in bids_hist] + [r for r in asks_hist] + [title]

        ani = animation.FuncAnimation(fig=fig, func=update, init_func=init,
                                      frames=len(lp.book_clean), interval=200, blit=True)
        # TODO: Switch to ImageMagickWriter or FFMpegWriter for performance
        #       https://matplotlib.org/stable/api/animation_api.html#writer-classes
        ani.save(IMG_DIR + "book.gif")
        print("PLOT ANIMATION SAVED")


# TODO: Add main block
lp = LeyvalPlotter(DATA_FILE)
lp.read_raw()
lp.clean_data()
lp.plot_book()

## plot agents
fig, ax = plt.subplots(figsize=FIGSIZE, dpi=DPI, layout='constrained')
title = ax.text(x=0.5, y=0.8, s="Agents: Tick #0", ha='center', transform=ax.transAxes, animated=True)

# TODO: Plot seperate capital and shares
# Option 1: up-down bar grophs, top/bottom
# Option 2: combined
# #shares | capital
#    5    |-----
#   12    |--
#    7    |----
#    5    |-
# Option 3: 2 plots of histograms/density curves for captial and shares
#   group by agent_type (so ~3-6 groups)
#   no connection from capital and share, just looking at behavior
# Option 4: Grid of cells, to help refer to agents (a1,j10)
#   both capital/share: text + heat map based on stdev?
#   can be modular, easy grouping of agent_type

# NOTE: Also want to group by type of trader (market taker, market provider, ...)
# NOTE: May want to scale #shares by current best sell price?
#
agents = data_agents[2]
x = agents['id']

fig, ax = plt.subplots(figsize=(15, 7))

width = 0.25  # the width of the bars
multiplier = 0.5
for row in agents.itertuples():
    offset = width * multiplier
    capitals = ax.bar(row.id + offset , row.capital, width, label=row.id, color='green')
    ax.bar_label(capitals, padding=3)

    shares = ax.bar(row.id + offset + width , row.shares, width, label=row.id, color ='blue')
    ax.bar_label(shares, padding=3)
    # multiplier += 1

ax.set_title('Agents')
ax.set_xticks(x + width, x)

plt.tight_layout()
plt.savefig(IMG_DIR + "agents.png")

## Book FuncAnimation
