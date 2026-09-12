from __future__ import annotations

import argparse
import math
import xml.etree.ElementTree as ET
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np


SVG_WIDTH = 920
SVG_HEIGHT = 470
COVER_WIDTH = 800
COVER_HEIGHT = 800
BACKGROUND = "#fcfcfb"
COVER_BACKGROUND = "#111315"
SWITCHING_COLOR = "#eb6834"
RIPPLE_COLOR = "#1baf7a"
TOTAL_COLOR = "#2a78d6"


def build_model(samples: int = 401) -> dict[str, object]:
    p0 = 0.25
    k1 = 0.38
    k2 = 0.20
    frequency = np.linspace(0.45, 2.80, samples)
    p_sw = k1 * frequency
    p_ripple = k2 / np.square(frequency)
    p_total = p0 + p_sw + p_ripple

    return {
        "p0": p0,
        "k1": k1,
        "k2": k2,
        "f_opt": math.pow(2.0 * k2 / k1, 1.0 / 3.0),
        "frequency": frequency,
        "p_sw": p_sw,
        "p_ripple": p_ripple,
        "p_total": p_total,
    }


def render_svg(output_path: Path) -> None:
    model = build_model()
    frequency = model["frequency"]
    p_sw = model["p_sw"]
    p_ripple = model["p_ripple"]
    p_total = model["p_total"]
    f_opt = model["f_opt"]
    p_opt = model["p0"] + model["k1"] * f_opt + model["k2"] / f_opt**2

    plt.rcParams.update(
        {
            "font.family": "sans-serif",
            "font.sans-serif": [
                "Microsoft YaHei",
                "SimHei",
                "Noto Sans CJK SC",
                "DejaVu Sans",
            ],
            "svg.fonttype": "none",
            "svg.hashsalt": "pwm-carrier-frequency-power-loss-model",
            "axes.unicode_minus": False,
        }
    )

    figure = plt.figure(
        figsize=(SVG_WIDTH / 72.0, SVG_HEIGHT / 72.0),
        facecolor=BACKGROUND,
    )
    axes = figure.add_axes((0.09, 0.23, 0.84, 0.60), facecolor=BACKGROUND)

    axes.plot(
        frequency,
        p_sw,
        color=SWITCHING_COLOR,
        linewidth=2.4,
        label="开关损耗：P_sw = k1 f",
    )
    axes.plot(
        frequency,
        p_ripple,
        color=RIPPLE_COLOR,
        linewidth=2.4,
        label="纹波铜损：P_ripple = k2/f^2",
    )
    axes.plot(
        frequency,
        p_total,
        color=TOTAL_COLOR,
        linewidth=3.0,
        label="总损耗：P_total = P0 + P_sw + P_ripple",
    )

    axes.vlines(
        f_opt,
        0.0,
        p_opt,
        color=TOTAL_COLOR,
        linewidth=1.2,
        linestyles=(0, (3, 3)),
    )
    axes.scatter(
        [f_opt],
        [p_opt],
        s=50,
        facecolor=BACKGROUND,
        edgecolor=TOTAL_COLOR,
        linewidth=2.0,
        zorder=5,
    )
    axes.annotate(
        "模型最低点",
        xy=(f_opt, p_opt),
        xytext=(0, 12),
        textcoords="offset points",
        ha="center",
        color=TOTAL_COLOR,
        fontsize=11,
        fontweight="bold",
    )

    axes.set_xlim(float(frequency[0]), float(frequency[-1]))
    axes.set_ylim(0.0, float(max(p_total)) * 1.08)
    axes.set_xlabel("PWM 载波频率 f（示意）", color="#52514e", labelpad=12)
    axes.set_ylabel("相对损耗", color="#52514e", labelpad=16)
    axes.set_xticks([])
    axes.set_yticklabels([])
    axes.tick_params(length=0)
    axes.grid(axis="y", color="#00000018", linewidth=1.0)
    for spine in axes.spines.values():
        spine.set_visible(False)
    legend = axes.legend(
        loc="upper right",
        frameon=True,
        fancybox=False,
        framealpha=1.0,
        fontsize=10,
        handlelength=2.8,
    )
    legend.get_frame().set_facecolor(BACKGROUND)
    legend.get_frame().set_edgecolor("none")

    figure.text(
        0.5,
        0.935,
        "开关损耗与纹波铜损的理想化模型",
        ha="center",
        fontsize=15,
        fontweight="bold",
        color="#0b0b0b",
    )
    figure.text(
        0.5,
        0.895,
        "固定平均电流、连续电流条件下的趋势图，不是本次扫频的损耗分解",
        ha="center",
        fontsize=10,
        color="#8a8985",
    )
    figure.text(
        0.5,
        0.105,
        "P_total = P0 + k1 f + k2/f^2",
        ha="center",
        fontsize=11,
        color="#52514e",
    )
    figure.text(
        0.5,
        0.055,
        "f* = (2k2/k1)^(1/3)；参数取决于电机、驱动器和工况",
        ha="center",
        fontsize=10,
        color="#52514e",
    )

    output_path.parent.mkdir(parents=True, exist_ok=True)
    figure.savefig(output_path, format="svg", metadata={"Date": None})
    plt.close(figure)

    ET.register_namespace("", "http://www.w3.org/2000/svg")
    ET.register_namespace("xlink", "http://www.w3.org/1999/xlink")
    tree = ET.parse(output_path)
    root = tree.getroot()
    root.set("width", "100%")
    root.attrib.pop("height", None)
    root.set("viewBox", f"0 0 {SVG_WIDTH} {SVG_HEIGHT}")
    root.set("role", "img")
    root.set(
        "aria-label",
        "开关损耗随频率上升、纹波铜损随频率平方下降，总损耗由两者与基线损耗逐点相加得到的理想化模型",
    )
    tree.write(output_path, encoding="utf-8", xml_declaration=True)


def render_cover_svg(output_path: Path) -> None:
    model = build_model()
    frequency = model["frequency"]
    p_sw = model["p_sw"]
    p_ripple = model["p_ripple"]
    p_total = model["p_total"]
    f_opt = model["f_opt"]
    p_opt = model["p0"] + model["k1"] * f_opt + model["k2"] / f_opt**2

    plt.rcParams.update(
        {
            "font.family": "sans-serif",
            "font.sans-serif": [
                "Microsoft YaHei",
                "SimHei",
                "Noto Sans CJK SC",
                "DejaVu Sans",
            ],
            "svg.fonttype": "none",
            "svg.hashsalt": "pwm-carrier-frequency-power-cover",
            "axes.unicode_minus": False,
        }
    )

    figure = plt.figure(
        figsize=(COVER_WIDTH / 72.0, COVER_HEIGHT / 72.0),
        facecolor=COVER_BACKGROUND,
    )
    axes = figure.add_axes((0.07, 0.06, 0.86, 0.49), facecolor=COVER_BACKGROUND)
    axes.plot(frequency, p_sw, color="#ff8a3d", linewidth=12)
    axes.plot(frequency, p_ripple, color="#36d399", linewidth=12)
    axes.plot(frequency, p_total, color="#45b8ff", linewidth=15)
    axes.vlines(
        f_opt,
        0.0,
        p_opt,
        color="#45b8ff",
        linewidth=5,
        linestyles=(0, (4, 4)),
        alpha=0.72,
    )
    axes.scatter(
        [f_opt],
        [p_opt],
        s=380,
        facecolor=COVER_BACKGROUND,
        edgecolor="#45b8ff",
        linewidth=8,
        zorder=5,
    )
    axes.set_xlim(float(frequency[0]), float(frequency[-1]))
    axes.set_ylim(0.0, float(max(p_total)) * 1.05)
    axes.axis("off")

    figure.text(
        0.5,
        0.84,
        "PWM 载波频率",
        ha="center",
        va="center",
        fontsize=96,
        fontweight="bold",
        color="#f8fafc",
    )
    figure.text(
        0.5,
        0.69,
        "损耗优化",
        ha="center",
        va="center",
        fontsize=122,
        fontweight="bold",
        color="#f8fafc",
    )

    output_path.parent.mkdir(parents=True, exist_ok=True)
    figure.savefig(output_path, format="svg", metadata={"Date": None})
    plt.close(figure)

    ET.register_namespace("", "http://www.w3.org/2000/svg")
    ET.register_namespace("xlink", "http://www.w3.org/1999/xlink")
    tree = ET.parse(output_path)
    root = tree.getroot()
    root.set("width", str(COVER_WIDTH))
    root.set("height", str(COVER_HEIGHT))
    root.set("viewBox", f"0 0 {COVER_WIDTH} {COVER_HEIGHT}")
    root.set("role", "img")
    root.set("aria-label", "PWM 载波频率优化封面，三条粗线展示 U 型损耗模型")
    tree.write(output_path, encoding="utf-8", xml_declaration=True)


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate the PWM loss model SVG.")
    parser.add_argument(
        "--output",
        type=Path,
        default=Path(__file__).with_name("loss-model.svg"),
    )
    parser.add_argument(
        "--cover-output",
        type=Path,
        default=Path(__file__).with_name("cover.svg"),
    )
    args = parser.parse_args()
    render_svg(args.output)
    render_cover_svg(args.cover_output)


if __name__ == "__main__":
    main()
