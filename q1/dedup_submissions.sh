#!/bin/bash
# finds duplicate submissions by their contents, backs up the unique ones
# and prints a small report. errors go to their own file.

src="submissions"
backup="backup"
report="report.txt"
errors="errors.log"

mkdir -p "$backup"      # make the backup folder if it isn't there
: > "$errors"           # empty the error file each time i run this

processed=0
duplicates=0
backed_up=0

declare -A seen         # like a python dict, remembers hashes i've seen (had to google declare -A)

for file in "$src"/*; do
    [ -f "$file" ] || continue      # skip it if it's not a file

    hash=$(md5sum "$file" 2>>"$errors" | cut -d ' ' -f1)   # fingerprint of the contents

    if [ -z "$hash" ]; then         # no hash means i couldn't read it
        echo "skipped (could not read): $file" >> "$errors"
        continue
    fi

    processed=$((processed + 1))

    if [ -n "${seen[$hash]}" ]; then
        duplicates=$((duplicates + 1))      # seen this hash already, so it's a copy
    else
        seen[$hash]="$file"
        cp "$file" "$backup"/ 2>>"$errors"  # first time, back it up
        backed_up=$((backed_up + 1))
    fi
done

echo "files processed: $processed" > "$report"
echo "duplicates found: $duplicates" >> "$report"
echo "backed up: $backed_up" >> "$report"

cat "$report"
