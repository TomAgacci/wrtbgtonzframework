Formula Summary (for translator engine)
Each binary pattern maps to a slope:

Code
slope_value = pattern / (2^bits - 1)
Pulse duration is:

Code
duration_us = base_us + slope_us * slope_value
This gives you:

Tiny beeps for low‑value patterns

Longer beeps for high‑value patterns

Predictable timing curves for your quantitative modulation

A continuous slope across all binary combinations

EXAMPLES

// 00000000 → slope 0.0 → duration = base_us
// 00000001 → slope 1/255 → duration = base_us + slope_us*(1/255)
// 00001111 → slope 15/255 → duration = base_us + slope_us*(15/255)
// 01010101 → slope 85/255 → duration = base_us + slope_us*(85/255)
// 11111111 → slope 255/255 → duration = base_us + slope_us
