"""
Plot the scenarios written by the C++ simulation (data/scenarios.json).

Every scenario gets data/scenarios/<name>_data.png
  - distance to the curb over time, velocity arrows, walk signal

Scenarios that Viterbi has already run on (they have "predicted_intent") also get
data/scenarios/<name>_intent.png
  - the same distance plot, with the true intent vs the Viterbi path stacked underneath
  - uses true_states, predicted_intent and accuracy (recomputed if missing)
  - viterbi_log_prob, when present, is shown next to the accuracy

Each image has a figure title, the scenario name, and a caption area for the scenario description
(space is kept for it even when the description is empty).

Usage (from the repo root):
    python3 data/plot_scenarios.py                  # every scenario in data/scenarios.json
    python3 data/plot_scenarios.py scenario_1       # only these scenarios
    python3 data/plot_scenarios.py --json other.json --out some/dir
"""

import argparse
import json
import math
import textwrap
from pathlib import Path

import matplotlib
matplotlib.use("Agg")   # write files only, no window needed
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
from matplotlib.patches import Patch
from matplotlib.transforms import offset_copy

DATA_DIR = Path(__file__).resolve().parent

DATA_TITLE = "Simulated pedestrian approaching the curb"
INTENT_TITLE = "Testing Viterbi on simulated pedestrian intent"

CROSS_COLOR = "#0ca30c"       # green
NOT_CROSS_COLOR = "#d03b3b"   # red
VITERBI_COLOR = "#2a78d6"     # blue, so the dotted path stands apart from the green/red points
GO_ALPHA = 0.14               # light transparent green behind GO periods
INK = "#52514e"               # arrows, ruler, secondary text
GRID = "#e1e0d9"

STATE_COLORS = {"CROSS": CROSS_COLOR, "NOT_CROSS": NOT_CROSS_COLOR}
STATE_ROWS = {"CROSS": 1, "NOT_CROSS": 0}     # y position in the intent panel

ARROW_MAX_PT = 40             # longest velocity arrow, in points
ARROW_GAP_PT = 6              # space between a data point and its arrow
CAPTION_LINES = 2             # caption area is always at least this tall
CAPTION_WIDTH = 100           # characters per caption line


def load_scenarios(path):
    try:
        with open(path) as f:
            return json.load(f)["scenarios"]
    except json.JSONDecodeError as e:
        raise SystemExit(f"{path} is not valid JSON ({e}). Generate it by running the simulation (build/main), "
                         "or pass another file with --json")


def go_runs(signal, dt):
    """Merge consecutive GO timesteps into (start, end) spans, one timestep centred on t."""
    runs, start = [], None
    for i, go in enumerate(signal + [False]):
        if go and start is None:
            start = i
        elif not go and start is not None:
            runs.append(((start - 0.5) * dt, (i - 0.5) * dt))
            start = None
    return runs


def draw_distance(ax, scenario):
    """Distance to the curb (reversed, curb at the top) with velocity arrows. Returns legend handles."""
    fig = ax.figure
    params = scenario["params"]
    obs = scenario["observations"]
    dt = params["dt"]

    t = [i * dt for i in range(len(obs))]
    dist = [o["distance_to_curb"] for o in obs]
    vel = [o["velocity"] for o in obs]
    signal = [o["walk_signal"] for o in obs]
    colors = [STATE_COLORS[s] for s in scenario["true_states"]]

    # Arrows show velocity relative to the starting velocity: up = faster, down = slower
    v0 = params["init_vel"]
    dv = [v - v0 for v in vel]
    scale = ARROW_MAX_PT / max(0.5, max(abs(x) for x in dv))    # points per m/s

    def shifted(base, pts):
        # base transform moved up by `pts` points, so arrow lengths do not depend on the axis scale
        return offset_copy(base, fig=fig, y=pts, units="points")

    # Walk signal GO
    for start, end in go_runs(signal, dt):
        ax.axvspan(start, end, color=CROSS_COLOR, alpha=GO_ALPHA, lw=0)

    # Distance, reversed so approaching the curb (0) moves up. Arrows are sized in points, so work out
    # how many metres of headroom the longest up/down arrow needs at this axes height
    top = max(dist)
    height_pt = ax.get_position().height * fig.get_figheight() * 72
    up_pt = ARROW_GAP_PT + max(0, max(dv)) * scale + 8
    down_pt = ARROW_GAP_PT + max(0, -min(dv)) * scale + 8
    span = top / (1 - (up_pt + down_pt) / height_pt)
    ax.set_ylim(top + max(down_pt * span / height_pt, 0.04 * span), -max(up_pt * span / height_pt, 0.04 * span))
    ax.set_xlim(-0.5 * dt, t[-1] + 0.5 * dt)
    step = 2 if top <= 16 else 5
    ax.set_yticks(list(range(0, int(top) + step, step)))
    ax.axhline(0, color="#898781", lw=1.5, ls=(0, (5, 4)))
    ax.text(-0.4 * dt, 0, "curb", ha="left", va="bottom", fontsize=9, color=INK,
            transform=shifted(ax.transData, 2))
    ax.plot(t, dist, color="#898781", lw=1.5, alpha=0.7, zorder=2)

    # Velocity arrows
    for ti, di, d in zip(t, dist, dv):
        length = abs(d) * scale
        if length < 3:      # about the same speed as at the start
            continue
        sign = 1 if d > 0 else -1
        ax.annotate("", xy=(ti, di), xycoords=shifted(ax.transData, sign * (ARROW_GAP_PT + length)),
                    xytext=(ti, di), textcoords=shifted(ax.transData, sign * ARROW_GAP_PT),
                    arrowprops=dict(arrowstyle="-|>,head_width=0.18,head_length=0.35",
                                    color=INK, lw=1.3, shrinkA=0, shrinkB=0),
                    zorder=3)

    # Points coloured by the true state
    ax.scatter(t, dist, c=colors, s=46, edgecolors="white", linewidths=1.2, zorder=4)

    # Velocity ruler on the right: its own scale, in m/s relative to the start
    x = 1.07
    lo = -int(max(0, -min(dv)) * 2 + 0.999) / 2
    hi = int(max(0, max(dv)) * 2 + 0.999) / 2
    ticks = [lo + 0.5 * k for k in range(int((hi - lo) / 0.5) + 1)]
    mid = 0.62
    ax.annotate("", xy=(x, mid), xycoords=shifted(ax.transAxes, hi * scale),
                xytext=(x, mid), textcoords=shifted(ax.transAxes, lo * scale),
                arrowprops=dict(arrowstyle="-", color=INK, lw=1, shrinkA=0, shrinkB=0),
                annotation_clip=False)
    for q in ticks:
        ax.annotate("", xy=(x + 0.01, mid), xycoords=shifted(ax.transAxes, q * scale),
                    xytext=(x, mid), textcoords=shifted(ax.transAxes, q * scale),
                    arrowprops=dict(arrowstyle="-", color=INK, lw=1, shrinkA=0, shrinkB=0),
                    annotation_clip=False)
        ax.text(x + 0.02, mid, f"{q:+.1f}" if q else "0.0", transform=shifted(ax.transAxes, q * scale),
                va="center", fontsize=8, color=INK, clip_on=False)
    ax.text(x, mid, "Δ velocity\nvs. start (m/s)", transform=shifted(ax.transAxes, hi * scale + 14),
            ha="center", va="bottom", fontsize=8, color=INK, clip_on=False)
    ax.text(x, mid, f"start = {v0:g} m/s", transform=shifted(ax.transAxes, lo * scale - 12),
            ha="center", va="top", fontsize=8, color=INK, clip_on=False)

    # Labels, grid, frame
    ax.set_xlabel("time (s)", color=INK)
    ax.set_ylabel("distance to curb (m)", color=INK)
    ax.grid(axis="y", color=GRID, lw=1)
    ax.set_axisbelow(True)
    ax.tick_params(colors=INK)
    for side in ("top", "right"):
        ax.spines[side].set_visible(False)

    return [
        Line2D([], [], marker="o", ls="", color=CROSS_COLOR, markeredgecolor="white", label="CROSS"),
        Line2D([], [], marker="o", ls="", color=NOT_CROSS_COLOR, markeredgecolor="white", label="NOT_CROSS"),
        Patch(facecolor=CROSS_COLOR, alpha=GO_ALPHA, label="signal GO"),
    ]


def draw_intent(ax, scenario):
    """True intent (solid) vs the Viterbi path (dotted). Returns (legend handles, accuracy text)."""
    dt = scenario["params"]["dt"]
    truth = scenario["true_states"]
    predicted = scenario["predicted_intent"]
    n = len(truth)
    if len(predicted) != n:
        raise SystemExit(f"{scenario['name']}: predicted_intent has {len(predicted)} entries, expected {n}")

    t = [i * dt for i in range(n)]

    # One point per state per timestep: solid on the true path, faint elsewhere to show the rows
    for state, y in STATE_ROWS.items():
        ax.scatter(t, [y] * n, s=70, lw=0, zorder=3, color=STATE_COLORS[state],
                   alpha=[1.0 if s == state else 0.22 for s in truth])

    ax.plot(t, [STATE_ROWS[s] for s in truth], color="black", lw=3, alpha=0.7,
            solid_capstyle="round", solid_joinstyle="round", zorder=2)
    ax.plot(t, [STATE_ROWS[s] for s in predicted], color=VITERBI_COLOR, lw=2.5, ls=(0, (1, 1.6)),
            dash_capstyle="round", zorder=4)

    ax.set_yticks(list(STATE_ROWS.values()))
    ax.set_yticklabels(list(STATE_ROWS))
    ax.set_ylim(-0.5, 1.5)
    ax.set_xlim(-0.5 * dt, t[-1] + 0.5 * dt)
    ax.set_xlabel("time (s)", color=INK)
    ax.tick_params(colors=INK, left=False)
    for side in ("top", "right", "left"):
        ax.spines[side].set_visible(False)

    correct = sum(a == b for a, b in zip(truth, predicted))
    accuracy = scenario.get("accuracy", correct / n)
    handles = [
        Line2D([], [], color="black", lw=3, alpha=0.7, label="true intent"),
        Line2D([], [], color=VITERBI_COLOR, lw=2.5, ls=(0, (1, 1.6)), dash_capstyle="round", label="Viterbi path"),
    ]
    note = f"Viterbi accuracy {accuracy:.1%} ({correct}/{n} timesteps)"

    # Log probability of the whole path grows more negative with every timestep, so also show
    # the per-step average as a probability, which is comparable across scenarios of any length
    log_prob = scenario.get("viterbi_log_prob")
    if log_prob is not None:
        note += f"  ·  path log-prob {log_prob:.2f} (≈ {math.exp(log_prob / n):.2f} per step)"
    return handles, note


def legend_above(ax, handles):
    ax.legend(handles=handles, loc="lower left", ncol=len(handles), frameon=False, fontsize=9,
              bbox_to_anchor=(0, 1.0))


def finish(fig, top_ax, bottom_ax, scenario, title, path, note=None):
    """Figure title, scenario name, optional note under the plot, caption area, then save."""
    fig.suptitle(title, x=top_ax.get_position().x0, ha="left", fontsize=14, y=0.99)
    top_ax.set_title(scenario["name"], loc="right", fontsize=11, color=INK, pad=10)

    # Text under the bottom axes is placed in points below its x label, so it stays put
    # whatever the axes height
    below = -34
    if note:
        bottom_ax.annotate(note, xy=(1, 0), xycoords="axes fraction", xytext=(0, below),
                           textcoords="offset points", ha="right", va="top", fontsize=9, color=INK)
        below -= 20

    # Caption: always reserve CAPTION_LINES lines, so the layout is the same with or without a description
    description = scenario.get("description", "").strip()
    lines = textwrap.wrap(description, CAPTION_WIDTH) if description else []
    caption = "\n".join(lines + [" "] * (CAPTION_LINES - len(lines)))
    bottom_ax.annotate(caption, xy=(0.5, 0), xycoords="axes fraction", xytext=(0, below),
                       textcoords="offset points", ha="center", va="top", fontsize=9, color=INK,
                       style="italic", linespacing=1.4)
    # An invisible column of CAPTION_LINES lines makes sure bbox_inches="tight" keeps the empty caption space
    bottom_ax.annotate("\n".join(["M"] * CAPTION_LINES), xy=(0.5, 0), xycoords="axes fraction",
                       xytext=(0, below), textcoords="offset points", ha="center", va="top", fontsize=9,
                       linespacing=1.4, alpha=0)

    path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(path, bbox_inches="tight", pad_inches=0.25)
    plt.close(fig)
    return path


def plot_distance(scenario, out_dir):
    fig, ax = plt.subplots(figsize=(9, 5.6), dpi=150)
    legend_above(ax, draw_distance(ax, scenario))
    return finish(fig, ax, ax, scenario, DATA_TITLE, out_dir / f"{scenario['name']}_data.png")


def plot_intent(scenario, out_dir):
    """Distance plot with the intent comparison stacked underneath.
    Returns None when Viterbi has not run on this scenario yet."""
    if not scenario.get("predicted_intent"):
        return None

    fig, (top, bottom) = plt.subplots(2, 1, figsize=(9, 7.6), dpi=150, sharex=True,
                                      gridspec_kw=dict(height_ratios=[3.2, 1.2], hspace=0.34))
    legend_above(top, draw_distance(top, scenario))
    top.tick_params(labelbottom=True)
    handles, accuracy = draw_intent(bottom, scenario)
    legend_above(bottom, handles)
    return finish(fig, top, bottom, scenario, INTENT_TITLE, out_dir / f"{scenario['name']}_intent.png",
                  note=accuracy)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("names", nargs="*", help="scenario names to plot (default: all)")
    parser.add_argument("--json", type=Path, default=DATA_DIR / "scenarios.json")
    parser.add_argument("--out", type=Path, default=DATA_DIR / "scenarios")
    args = parser.parse_args()

    scenarios = load_scenarios(args.json)
    names = args.names or list(scenarios)
    for name in names:
        if name not in scenarios:
            raise SystemExit(f"no scenario '{name}' in {args.json} (have: {', '.join(scenarios)})")
        print(plot_distance(scenarios[name], args.out))
        intent_path = plot_intent(scenarios[name], args.out)
        print(intent_path or f"  skipped intent plot for {name}: no predicted_intent yet (run Viterbi first)")


if __name__ == "__main__":
    main()
