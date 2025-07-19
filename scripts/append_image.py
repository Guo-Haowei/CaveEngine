from PIL import Image

# Load images
images = [Image.open(path) for path in [
    'spr_player_walk.png',
    'spr_player_jump.png',
    'spr_player_damage.png',
    'spr_player_grab.png',
    'spr_player_grab_damaged.png',
    'spr_player_idle.png',
]]

widths = [img.width for img in images]
height = images[0].height  # assuming all are 1px tall

# Step 1: Sort images by width descending
sorted_images = sorted(images, key=lambda im: im.width, reverse=True)

# Step 2: Determine final width
max_width = sorted_images[0].width

# Step 3: Pack images into rows
rows = []  # each row is a list of (image, x_offset)
current_rows = []

for img in sorted_images:
    placed = False
    for row in current_rows:
        used_width = sum(im.width for im, _ in row)
        if used_width + img.width <= max_width:
            row.append((img, used_width))
            placed = True
            break
    if not placed:
        # Start a new row
        current_rows.append([(img, 0)])

# Step 4: Create final image
total_height = height * len(current_rows)
result = Image.new("RGBA", (max_width, total_height), (0, 0, 0, 0))

# Step 5: Paste images
for row_idx, row in enumerate(current_rows):
    y = row_idx * height
    for img, x in row:
        result.paste(img, (x, y))

result.save("packed.png")
