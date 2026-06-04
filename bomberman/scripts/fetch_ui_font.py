#!/usr/bin/env python3
"""下載台北黑體 Regular（SIL OFL 1.1，翰字鑄造 JT Foundry）。

來源鏡像：https://github.com/xlfont/taipei-sans-tc
官方說明：https://sites.google.com/view/jtfoundry/zh-tw/downloads
"""
from __future__ import annotations

import urllib.request
from pathlib import Path

URL = (
    "https://raw.githubusercontent.com/xlfont/taipei-sans-tc/master/src/"
    "TaipeiSansTCBeta-Regular.ttf"
)
OUT = Path(__file__).resolve().parent.parent / "assets" / "fonts" / "TaipeiSansTCBeta-Regular.ttf"
MIN_BYTES = 1_000_000


def main() -> None:
    OUT.parent.mkdir(parents=True, exist_ok=True)
    if OUT.is_file() and OUT.stat().st_size >= MIN_BYTES:
        print(f"already present: {OUT} ({OUT.stat().st_size} bytes)")
        return
    print(f"downloading {URL} ...")
    urllib.request.urlretrieve(URL, OUT)
    size = OUT.stat().st_size
    if size < MIN_BYTES:
        raise SystemExit(f"download too small ({size} bytes); check network or URL")
    print(f"wrote {OUT} ({size} bytes)")


if __name__ == "__main__":
    main()
