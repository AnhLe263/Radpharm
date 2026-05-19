#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Merge Geant4 reaction tally files.

Features
--------
1. Merge all reaction_thread_*.txt files
2. Sum counts of identical reactions
3. Ignore gamma in products:
      A -> B + gamma
      A -> B
   are treated as the same reaction
4. Sort by descending count
5. Save merged result

Usage
-----
python merge_reactions.py

Optional:
python merge_reactions.py output.txt
"""

import glob
import re
import sys
from collections import defaultdict


# --------------------------------------------------------
# CONFIG
# --------------------------------------------------------

# words considered gamma
GAMMA_NAMES = {
    "gamma",
    "Gamma",
    "photon"
}


# --------------------------------------------------------
# REMOVE GAMMA
# --------------------------------------------------------

def normalize_reaction(reaction_line):
    """
    Convert:
        proton + Ni64 -> Cu64 + neutron + gamma
    into:
        proton + Ni64 -> Cu64 + neutron
    """

    reaction_line = reaction_line.strip()

    if "->" not in reaction_line:
        return reaction_line

    left, right = reaction_line.split("->")

    left = left.strip()
    right = right.strip()

    products = [
        normalize_particle_name(x.strip())
        for x in right.split("+")
        ]

    # remove gamma
    products = [
        p for p in products
        if p not in GAMMA_NAMES
    ]

    # remove duplicates/spaces
    products = [p for p in products if p]

    # sort for canonical form
    products.sort()

    if len(products) == 0:
        right_new = "nothing"
    else:
        right_new = " + ".join(products)

    return f"{left} -> {right_new}"

# --------------------------------------------------------
# REMOVE ELASTIC QUASI-ELASTIC
# --------------------------------------------------------

def is_trivial_reaction(reaction):
    """
    Remove reactions where products are identical
    to initial particles.

    Example:
        proton + O16 -> proton + O16

    returns True
    """

    if "->" not in reaction:
        return False

    left, right = reaction.split("->")

    left_parts = sorted([
        x.strip()
        for x in left.split("+")
        if x.strip()
    ])

    right_parts = sorted([
        x.strip()
        for x in right.split("+")
        if x.strip()
    ])

    return left_parts == right_parts

# --------------------------------------------------------
# MERGE Cu-64 states
# --------------------------------------------------------
def normalize_particle_name(name):
    """
    Only normalize Cu64 excited states.

    Examples:
        Cu64[1594.19000] -> Cu64
        Cu64[0.0] -> Cu64
    """

    name = name.strip()

    # only for Cu64 excited states
    name = re.sub(r"^Cu64\[.*?\]$", "Cu64", name)

    return name


# --------------------------------------------------------
# PARSE FILE
# --------------------------------------------------------

def parse_file(filename, tally_dict):

    with open(filename, "r") as f:

        for line in f:

            line = line.strip()

            if not line:
                continue

            if line.startswith("#"):
                continue

            # expected format:
            # 12345    proton + Ni64 -> Cu64 + neutron

            m = re.match(r"^\s*(\d+)\s+(.*)$", line)

            if not m:
                continue

            count = int(m.group(1))
            reaction = m.group(2).strip()

            reaction = normalize_reaction(reaction)
            # skip elastic-like channels
            if is_trivial_reaction(reaction):
                continue
            tally_dict[reaction] += count


# --------------------------------------------------------
# MAIN
# --------------------------------------------------------

def main():

    output_file = "merged_reactions.txt"

    if len(sys.argv) > 1:
        output_file = sys.argv[1]

    files = sorted(glob.glob("reaction_thread_*.txt"))

    if not files:
        print("No reaction_thread_*.txt files found")
        return

    print(f"Found {len(files)} files")

    tally = defaultdict(int)

    for f in files:
        print(f"Reading: {f}")
        parse_file(f, tally)

    # sort descending by count
    sorted_items = sorted(
        tally.items(),
        key=lambda x: x[1],
        reverse=True
    )

    with open(output_file, "w") as out:

        out.write("# Merged reaction tally\n")
        out.write("# Gamma removed from products\n\n")

        for reaction, count in sorted_items:

            out.write(f"{count:12d}    {reaction}\n")

    print()
    print(f"Merged reactions: {len(sorted_items)}")
    print(f"Output written to: {output_file}")


# --------------------------------------------------------

if __name__ == "__main__":
    main()