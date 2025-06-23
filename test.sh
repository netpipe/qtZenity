#!/bin/bash

# Path to your compiled QtZenity executable
QTZENITY="/Applications/qtZenity.app/contents/MacOS/qtZenity"

# Run with multiple widgets and debug mode to get key=value pairs
OUTPUT=$("$QTZENITY" \
  --entry "Enter your name" \
  --multi-checkbox "Option A,Option B" \
  --calendar "Pick a date" \
  --slider "Volume,0,100,50" \
  --dial "Speed,0,10,5" \
  --datetime "Pick time" \
  --radio "Mode,Easy,Hard,Expert" \
  --clock \
  --window-size 600,700 \
  --debug \
)

# Check if user canceled (non-zero exit code)
if [ $? -ne 0 ]; then
  echo "Dialog canceled."
  exit 1
fi

echo "Raw output:"
echo "$OUTPUT"

# Parse CSV into associative array
declare -A results

# Convert to key=value lines
IFS=',' read -ra parts <<< "$OUTPUT"
for kv in "${parts[@]}"; do
  key="${kv%%=*}"
  val="${kv#*=}"
  results["$key"]="$val"
done

# Set initial input line
line="$OUTPUT"

# Initialize arrays
i=0

# Parse CSV key=value string
while [ -n "$line" ]; do
  case "$line" in
    *,*)
      pair="${line%%,*}"
      line="${line#*,}"
      ;;
    *)
      pair="$line"
      line=""
      ;;
  esac

  key="${pair%%=*}"
  val="${pair#*=}"
  keys[$i]="$key"
  vals[$i]="$val"
  i=$((i + 1))
done

# Print output in original order
echo "=== Ordered Output ==="
j=0
while [ $j -lt $i ]; do
  echo "${keys[$j]} = ${vals[$j]}"
  j=$((j + 1))
done
