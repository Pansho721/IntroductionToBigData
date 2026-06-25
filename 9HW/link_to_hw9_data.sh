#!/usr/bin/env bash
set -e
[[ $# -ne 2 ]] && { echo "Usage: $0 <src_dir> <dst_dir>"; exit 1; }

SRC=$(realpath "$1")          # 确保绝对
DST=$2
mkdir -p "$DST"
HERE=$PWD
cd "$DST"

for f in "$SRC"/block-* "$SRC"/row* "$SRC"/column* "$SRC"/meta; do
  [[ -e $f ]] || continue
  ln -s "$f" .                # 这里直接用绝对路径
done

cd "$HERE"