#!/usr/bin/env python3
# combines clang -MJ fragments into a compile_commands.json database.
# usage: gen_compdb.py [-o compile_commands.json] [-s pattern] [--strip-pch] dir [dir ...]
#
# each fragment is a single JSON object with a trailing comma, as emitted by
# clang's -MJ flag. this script finds all .compdb files under the given
# directories, strips trailing commas, and wraps them in a JSON array.

import argparse
import json
import os
import re
import sys


def find_fragments(dirs, suffix=".compdb"):
    for d in dirs:
        for root, _, files in os.walk(d):
            for f in files:
                if f.endswith(suffix):
                    yield os.path.join(root, f)


def parse_fragment(path):
    with open(path) as f:
        text = f.read().rstrip().rstrip(",")
    return json.loads(text)


def strip_pch_args(entry):
    """remove -include pch.h pairs from arguments (same as the sed in the bear path)"""
    args = entry.get("arguments")
    if not args:
        return entry
    filtered = []
    skip_next = False
    for i, arg in enumerate(args):
        if skip_next:
            skip_next = False
            continue
        if arg == "-include" and i + 1 < len(args) and args[i + 1].endswith("/pch.h"):
            skip_next = True
            continue
        filtered.append(arg)
    entry["arguments"] = filtered
    return entry


def strip_mj_args(entry):
    """remove the -MJ flag itself from the recorded arguments"""
    args = entry.get("arguments")
    if not args:
        return entry
    entry["arguments"] = [a for a in args if not a.startswith("-MJ")]
    return entry


def main():
    parser = argparse.ArgumentParser(description="combine clang -MJ fragments into compile_commands.json")
    parser.add_argument("dirs", nargs="+", help="directories to search for .compdb fragments")
    parser.add_argument("-o", "--output", default="compile_commands.json", help="output file (default: compile_commands.json)")
    parser.add_argument("-s", "--suffix", default=".compdb", help="fragment file suffix (default: .compdb)")
    parser.add_argument("--strip-pch", action="store_true", help="strip -include pch.h from arguments")
    parser.add_argument("--delete", action="store_true", help="delete fragment files after combining")
    args = parser.parse_args()

    fragments = sorted(find_fragments(args.dirs, args.suffix))
    if not fragments:
        print(f"warning: no {args.suffix} fragments found in {args.dirs}", file=sys.stderr)
        with open(args.output, "w") as f:
            f.write("[]\n")
        return

    entries = []
    for path in fragments:
        try:
            entry = parse_fragment(path)
            entry = strip_mj_args(entry)
            if args.strip_pch:
                entry = strip_pch_args(entry)
            entries.append(entry)
        except (json.JSONDecodeError, OSError) as e:
            print(f"warning: skipping {path}: {e}", file=sys.stderr)

    with open(args.output, "w") as f:
        json.dump(entries, f, indent=2)
        f.write("\n")

    if args.delete:
        for path in fragments:
            os.remove(path)

    print(f"wrote {len(entries)} entries to {args.output}", file=sys.stderr)


if __name__ == "__main__":
    main()
