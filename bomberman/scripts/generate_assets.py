#!/usr/bin/env python3
"""Generate CC0-style arcade sprites (爆爆王風格參考，非官方素材)."""
from pathlib import Path

try:
    from PIL import Image, ImageDraw
except ImportError:
    raise SystemExit("pip install pillow") from None

ROOT = Path(__file__).resolve().parents[1] / "assets" / "sprites"


def save(path: Path, img: Image.Image) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    img.save(path, "PNG")
    print("wrote", path.relative_to(ROOT.parent.parent))


def tile_grass() -> Image.Image:
    img = Image.new("RGBA", (64, 64), (118, 198, 98, 255))
    d = ImageDraw.Draw(img)
    for i in range(0, 64, 8):
        d.line([(i, 0), (i, 64)], fill=(100, 175, 85, 90))
        d.line([(0, i), (64, i)], fill=(100, 175, 85, 90))
    d.rectangle([2, 2, 61, 61], outline=(85, 155, 75, 120), width=1)
  # light highlight
    d.ellipse([6, 6, 18, 16], fill=(200, 255, 200, 40))
    return img


def tile_crate() -> Image.Image:
    """可破壞：木箱"""
    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rounded_rectangle([6, 8, 58, 58], radius=6, fill=(168, 110, 55, 255))
    d.rounded_rectangle([8, 10, 56, 56], radius=5, outline=(110, 65, 30, 255), width=2)
    for y in (22, 38):
        d.line([(10, y), (54, y)], fill=(120, 70, 35, 255), width=3)
    for x in (22, 38):
        d.line([(x, 12), (x, 54)], fill=(120, 70, 35, 255), width=3)
    return img


def tile_bush() -> Image.Image:
    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.ellipse([8, 20, 56, 58], fill=(55, 140, 65, 255))
    d.ellipse([4, 28, 36, 56], fill=(45, 125, 58, 255))
    d.ellipse([28, 26, 60, 54], fill=(50, 130, 62, 255))
    for x, y in ((18, 34), (40, 36), (30, 44)):
        d.ellipse([x, y, x + 5, y + 5], fill=(230, 60, 70, 255))
    return img


def tile_stone() -> Image.Image:
    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rounded_rectangle([4, 4, 59, 59], radius=8, fill=(145, 150, 165, 255))
    d.rounded_rectangle([10, 10, 53, 53], radius=6, fill=(175, 180, 195, 255))
    d.arc([18, 18, 46, 46], 20, 300, fill=(95, 100, 115, 255), width=3)
    return img


def player_frame(facing: int, step: int, body=(70, 150, 255, 255)) -> Image.Image:
    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    bob = 2 if step else 0
    d.ellipse([14, 44 + bob, 50, 58 + bob], fill=(40, 40, 50, 120))
    d.rounded_rectangle([20, 28 + bob, 44, 50 + bob], radius=8, fill=body)
    d.ellipse([18, 8 + bob, 46, 34 + bob], fill=(255, 220, 190, 255))
    d.ellipse([18, 8 + bob, 46, 28 + bob], fill=(80, 140, 255, 255))
    eye = 28 + (facing % 2) * 4
    d.ellipse([eye, 18 + bob, eye + 8, 26 + bob], fill=(30, 30, 50, 255))
    d.ellipse([eye + 2, 20 + bob, eye + 5, 23 + bob], fill=(255, 255, 255, 255))
    return img


def player_sheet(body=(70, 150, 255, 255)) -> Image.Image:
    sheet = Image.new("RGBA", (64 * 8, 64), (0, 0, 0, 0))
    for dir_i in range(4):
        for step in range(2):
            frame = player_frame(dir_i, step, body)
            sheet.paste(frame, (64 * (dir_i * 2 + step), 0))
    return sheet


def enemy_sprite() -> Image.Image:
    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.ellipse([14, 44, 50, 58], fill=(40, 40, 50, 100))
    d.rounded_rectangle([18, 26, 46, 52], radius=10, fill=(240, 70, 85, 255))
    d.ellipse([16, 8, 48, 36], fill=(255, 200, 200, 255))
    d.polygon([(22, 20), (30, 28), (22, 28)], fill=(50, 20, 30, 255))
    d.polygon([(42, 20), (34, 28), (42, 28)], fill=(50, 20, 30, 255))
    d.arc([24, 30, 40, 38], 10, 170, fill=(180, 50, 60, 255), width=2)
    return img


def bomb_sprite() -> Image.Image:
    """水球彈風格"""
    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.ellipse([12, 20, 52, 58], fill=(60, 170, 255, 230))
    d.ellipse([18, 24, 46, 50], fill=(140, 220, 255, 200))
    d.ellipse([24, 30, 38, 42], fill=(220, 245, 255, 180))
    d.ellipse([28, 32, 34, 38], fill=(255, 255, 255, 220))
    d.arc([22, 28, 42, 44], 200, 340, fill=(40, 120, 200, 255), width=2)
    d.ellipse([30, 18, 36, 24], fill=(255, 255, 255, 255))
    d.ellipse([31, 19, 35, 23], fill=(30, 30, 40, 255))
    return img


def explosion_sheet() -> Image.Image:
    sheet = Image.new("RGBA", (64 * 4, 64), (0, 0, 0, 0))
    for i in range(4):
        img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
        d = ImageDraw.Draw(img)
        r = 14 + i * 7
        d.ellipse([32 - r, 32 - r, 32 + r, 32 + r], fill=(100, 200, 255, 200 - i * 25))
        d.ellipse([32 - r // 2, 32 - r // 2, 32 + r // 2, 32 + r // 2],
                  fill=(255, 255, 255, 220))
        sheet.paste(img, (64 * i, 0))
    return sheet


def item_icon(color: tuple) -> Image.Image:
    img = Image.new("RGBA", (64, 64), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.ellipse([8, 8, 56, 56], fill=color)
    d.ellipse([14, 12, 48, 44], fill=(255, 255, 255, 80))
    return img


def ui_panel() -> Image.Image:
    img = Image.new("RGBA", (128, 64), (255, 200, 80, 255))
    d = ImageDraw.Draw(img)
    d.rounded_rectangle([2, 2, 125, 61], radius=12, outline=(200, 120, 30, 255), width=3)
    return img


def menu_title() -> Image.Image:
    img = Image.new("RGBA", (480, 120), (0, 0, 0, 0))
    d = ImageDraw.Draw(img)
    d.rounded_rectangle([8, 16, 472, 104], radius=22, fill=(255, 130, 35, 255))
    d.rounded_rectangle([18, 28, 462, 92], radius=16, fill=(255, 215, 70, 255))
    return img


def main() -> None:
    tiles = ROOT / "tiles"
    save(tiles / "floor.png", tile_grass())
    save(tiles / "wall_brick.png", tile_crate())
    save(tiles / "wall_bush.png", tile_bush())
    save(tiles / "wall_metal.png", tile_stone())
    save(ROOT / "actors" / "player_sheet.png", player_sheet())
    save(ROOT / "actors" / "player2_sheet.png", player_sheet((90, 220, 120, 255)))
    save(ROOT / "actors" / "enemy.png", enemy_sprite())
    save(ROOT / "bomb.png", bomb_sprite())
    save(ROOT / "fx" / "explosion_sheet.png", explosion_sheet())
    items = ROOT / "items"
    save(items / "item_bomb.png", item_icon((255, 80, 80, 255)))
    save(items / "item_fire.png", item_icon((255, 140, 40, 255)))
    save(items / "item_speed.png", item_icon((80, 200, 255, 255)))
    ui = ROOT / "ui"
    save(ui / "button.png", ui_panel())
    save(ui / "panel.png", ui_panel())
    save(ui / "menu_title.png", menu_title())
    legacy = ROOT
    save(legacy / "floor.png", tile_grass())
    save(legacy / "wall_brick.png", tile_crate())
    save(legacy / "wall_metal.png", tile_stone())
    save(legacy / "player.png", player_frame(0, 0))
    save(legacy / "enemy.png", enemy_sprite())
    save(legacy / "explosion.png", explosion_sheet().crop((0, 0, 64, 64)))
    save(legacy / "menu_title.png", menu_title())


if __name__ == "__main__":
    main()
