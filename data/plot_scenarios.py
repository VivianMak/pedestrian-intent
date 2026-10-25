"""
Plot the scenarios written by the C++ simulation (data/scenarios.json).

One image per scenario, saved to data/scenarios/<name>_data.png
  - title   = scenario name
  - caption = scenario description (skipped when empty)

Later, the plot comparing intent will be saved as data/scenarios/<name>_intent.png

Usage (from the repo root):
    python3 data/plot_scenarios.py                  # every scenario in data/scenarios.json
    python3 data/plot_scenarios.py scenario_1       # only these scenarios
    python3 data/plot_scenarios.py --json other.json --out some/dir
"""

import argparse
import json
import textwrap
from pathlib import Path

import matplotlib
matplotlib.use("Agg")   # write files only, no window needed
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
from matplotlib.patches import Patch
from matplotlib.transforms import offset_copy

DATA_DIR = Path(__file__).resolve().parent

CROSS_COLOR = "#0ca30c"       # green
NOT_CROSS_COLOR = "#d03b3b"   # red
GO_ALPHA = 0.14               # light transparent green behind GO periods
INK = "#52514e"               # arrows, ruler, secondary text
GRID = "#e1e0d9"

ARROW_MAX_PT = 40             # longest velocity arrow, in points
ARROW_GAP_PT = 6              # space between a data point and its arrow


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


def plot_distance(scenario, out_dir):
    params = scenario["params"]
    obs = scenario["observations"]
    dt = params["dt"]

    t = [i * dt for i in range(len(obs))]
    dist = [o["distance_to_curb"] for o in obs]
    vel = [o["velocity"] for o in obs]
    signal = [o["walk_signal"] for o in obs]
    colors = [CROSS_COLOR if s == "CROSS" else NOT_CROSS_COLOR for s in scenario["true_states"]]

    # Arrows show velocity relative to the starting velocity: up = faster, down = slower
    v0 = params["init_vel"]
    dv = [v - v0 for v in vel]
    scale = ARROW_MAX_PT / max(0.5, max(abs(x) for x in dv))    # points per m/s

    fig, ax = plt.subplots(figsize=(9, 5.6), dpi=150)

    def shifted(base, pts):
        # base transform moved up by `pts` points, so arrow lengths do not depend on the axis scale
        return offset_copy(base, fig=fig, y=pts, units="points")

    # Walk signal GO
    for start, end in go_runs(signal, dt):
        ax.axvspan(start, end, color=CROSS_COLOR, alpha=GO_ALPHA, lw=0)

    # Distance, reversed so approaching the curb (0) moves up. The extra room above/below
    # the data keeps the arrows inside the frame
    top = max(dist)
    ax.set_ylim(top * 1.12, -top * 0.22)
    ax.set_xlim(-0.5 * dt, t[-1] + 0.5 * dt)
    ax.set_yticks([y for y in range(0, int(top) + 2, 2)])
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

    ax.legend(handles=[
        Line2D([], [], marker="o", ls="", color=CROSS_COLOR, markeredgecolor="white", label="CROSS"),
        Line2D([], [], marker="o", ls="", color=NOT_CROSS_COLOR, markeredgecolor="white", label="NOT_CROSS"),
        Patch(facecolor=CROSS_COLOR, alpha=GO_ALPHA, label="signal GO"),
    ], loc="lower left", ncol=3, frameon=False, fontsize=9, bbox_to_anchor=(0, 1.0))

    ax.set_title(scenario["name"], loc="right", fontsize=13, pad=10)

    description = scenario.get("description", "")
    if description:
        ax.text(0.5, -0.14, "\n".join(textwrap.wrap(description, 100)), transform=ax.transAxes,
                ha="center", va="top", fontsize=9, color=INK, style="italic")

    out_dir.mkdir(parents=True, exist_ok=True)
    path = out_dir / f"{scenario['name']}_data.png"
    fig.savefig(path, bbox_inches="tight", pad_inches=0.2)
    plt.close(fig)
    return path


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


if __name__ == "__main__":
    main()
