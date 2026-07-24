#!/bin/bash
# watches a log file live, pulls out only the ERROR lines, shows them and saves
# them to a separate report at the same time.

log="app.log"
report="errors_report.log"

: > "$log"          # start the log empty so each run is clean and repeatable
: > "$report"       # empty the report too

# make some log lines in the background so there's something to watch live.
# INFO every loop, ERROR on the even ones.
( for i in 1 2 3 4 5; do
    echo "$(date +%T) INFO  request $i handled" >> "$log"
    if [ $((i % 2)) -eq 0 ]; then
        echo "$(date +%T) ERROR request $i failed" >> "$log"
    fi
    sleep 1
  done ) &

# tail -n0 -f shows only NEW lines from here on. without -n0 it first dumps the
# last 10 old lines and would re-report old errors, so -n0 keeps it to genuinely
# new entries. 2>/dev/null on tail drops its stderr (like the warning if the file
# gets rotated while following) so it doesn't clutter the screen. the pipe feeds
# the live lines into grep which keeps only the ERROR ones, and tee shows them and
# appends them to the report. timeout stops tail after 7s.
timeout 7 tail -n0 -f "$log" 2>/dev/null | grep --line-buffered ERROR | tee -a "$report"

echo "--- errors saved to $report ---"
cat "$report"
