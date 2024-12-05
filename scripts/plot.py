import json

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

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

## plot book

h_bids, edges_bids = data_book[0]['bids'].index, data_book[0]['bids'].values
h_asks, edges_asks = data_book[0]['asks'].index, data_book[0]['asks'].values

fig, ax = plt.subplots(figsize=(15, 7))
ax.bar(h_bids, edges_bids, label='bids', color='green')
ax.bar(h_asks, edges_asks, label='asks', color='red')

ax.legend()
plt.savefig(IMG_DIR + "book.png")

## plot book - animation (plt.pause)
fig, ax = plt.subplots(figsize=(7, 15))

for t, book in enumerate(data_book):
    ax.clear()

    h_bids, edges_bids = book['bids'].index, book['bids'].values
    h_asks, edges_asks = book['asks'].index, book['asks'].values

    ax.bar(h_bids, edges_bids, label='bids', color='green')
    ax.bar(h_asks, edges_asks, label='asks', color='red')

    ax.set_title(f"frame {t}")
    ax.legend()

    plt.pause(0.1)
