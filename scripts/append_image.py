from pathlib import Path
from PIL import Image


def append_images_horizontal(
    left_path: str | Path,
    right_path: str | Path,
    output_path: str | Path,
) -> None:
    left = Image.open(left_path).convert("RGBA")
    right = Image.open(right_path).convert("RGBA")

    if left.height != right.height:
        raise ValueError(
            f"Image heights must match: {left.height} != {right.height}"
        )

    output = Image.new(
        "RGBA",
        (left.width + right.width, left.height),
        (0, 0, 0, 0),
    )

    output.paste(left, (0, 0))
    output.paste(right, (left.width, 0))
    output.save(output_path)


append_images_horizontal(
    "spr_1.png",
    "spr_2.png",
    "combine.png",
)