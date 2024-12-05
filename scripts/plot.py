import json

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.animation as animation

DATA_FILE = "../data/pretty.json"
IMG_DIR = "img/"

with open(DATA_FILE, 'r') as f:
    raw_json = json.load(f)

def read_agents(agents):
    return pd.json_normalize(agents)

def read_book(book):
    bids = pd.Series([x[1] for x in book['bid_counts']], index = [x[0] for x in book['bid_counts']], name='bids')
    asks = pd.Series([x[1] for x in book['ask_counts']], index = [x[0] for x in book['ask_counts']], name='asks')
    return pd.concat([bids, asks], keys=['bids', 'asks'])

data_agents = []
data_book = []
for run_tick in raw_json:
    data_agents.append(read_agents(run_tick['agents']))
    data_book.append(read_book(run_tick['order_book']))

## Clean
clean_agents = pd.concat(data_agents, keys = range(len(data_agents))).drop(columns='id')
clean_agents.index = clean_agents.index.set_names(['time', 'id'])

clean_book = pd.concat(data_book, axis=1).T.fillna(0).astype("Int64")
clean_book.index = clean_book.index.set_names('time')

## plot agents
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
fig, ax = plt.subplots(figsize=(8, 4.5), dpi=200, layout='constrained')

bids_hist = ax.bar(x=clean_book.loc[0]['bids'].index, height=clean_book.loc[0]['bids'],
                   color='green', label='Bids', animated=True)
asks_hist = ax.bar(x=clean_book.loc[0]['asks'].index, height=clean_book.loc[0]['asks'],
                   color='red', label='Asks', animated=True)
title = ax.text(x=0.5, y=0.8, s="Tick #0", ha='center', transform=ax.transAxes, animated=True)

def init():
    ax.set(ylim=(0, clean_book.max().max() * 1.1),
       xlabel='Price ($)', ylabel='Quantity')
    ax.legend()
    return [r for r in bids_hist] + [r for r in asks_hist] + [title]

def update(frame_num):
    title.set_text(f"Tick #{frame_num}")
    for rect, quantity in zip(bids_hist.patches, clean_book.loc[frame_num]['bids']):
        rect.set_height(quantity)
    for rect, quantity in zip(asks_hist.patches, clean_book.loc[frame_num]['asks']):
        rect.set_height(quantity)
    return [r for r in bids_hist] + [r for r in asks_hist] + [title]

ani = animation.FuncAnimation(fig=fig, func=update, init_func=init,
                              frames=len(clean_book), interval=200, blit=True)
# TODO: Switch to ImageMagickWriter or FFMpegWriter for performance
#       https://matplotlib.org/stable/api/animation_api.html#writer-classes
ani.save(IMG_DIR + "book.gif")
