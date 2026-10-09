#!/usr/bin/env python3
"""Live GNUS supply on Ethereum, Polygon, Base and BSC (no dependencies)."""
import json
import urllib.request
from decimal import Decimal

CHAINS = {
    "Ethereum": ("https://ethereum-rpc.publicnode.com", "0x614577036f0a024dbc1c88ba616b394dd65d105a"),
    "Polygon": ("https://polygon-bor-rpc.publicnode.com", "0x127e47aba094a9a87d084a3a93732909ff031419"),
    "Base": ("https://base-rpc.publicnode.com", "0x614577036f0a024dbc1c88ba616b394dd65d105a"),
    "BSC": ("https://bsc-rpc.publicnode.com", "0x614577036f0a024dbc1c88ba616b394dd65d105a"),
}

def call(url, contract, selector):
    body = json.dumps({"jsonrpc":"2.0","id":1,"method":"eth_call",
                       "params":[{"to":contract,"data":selector},"latest"]}).encode()
    req = urllib.request.Request(url, body, {"Content-Type":"application/json","User-Agent":"gnus-supply/1.0"})
    with urllib.request.urlopen(req, timeout=20) as resp:
        result = json.load(resp)
    if "error" in result or not result.get("result"):
        raise RuntimeError(f"{url}: {result}")
    return int(result["result"], 16)

total = Decimal(0)
for name, (url, contract) in CHAINS.items():
    decimals = call(url, contract, "0x313ce567")
    supply = Decimal(call(url, contract, "0x18160ddd")) / Decimal(10**decimals)
    total += supply
    print(f"{name:9} {supply:,.18f} GNUS (decimals={decimals})")
print(f"TOTAL     {total:,.18f} GNUS")
