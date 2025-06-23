#!/bin/bash

# Path to your compiled QtZenity executable
QTZENITY="./qtzenity"

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

# Access values
echo ""
echo "Parsed values:"
echo "Name: ${results[input]}"
echo "Calendar date: ${results[calendar]}"
echo "Option A selected: ${results[option_a]}"
echo "Option B selected: ${results[option_b]}"
echo "Volume slider: ${results[slider]}"
echo "Speed dial: ${results[dial]}"
echo "Selected date/time: ${results[datetime]}"
echo "Radio mode: ${results[radio]}"
echo "Clock time: ${results[clock]}"
