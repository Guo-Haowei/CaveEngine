from PIL import Image

# Load images
imgs = [Image.open(path) for path in ["entrance.png", "exit.png"]]

# Assuming same size
w, h = imgs[0].size

# Horizontal concat
result = Image.new("RGBA", (w * len(imgs), h))
for i, img in enumerate(imgs):
    result.paste(img, (i * w, 0))
result.save("output_horizontal.png")