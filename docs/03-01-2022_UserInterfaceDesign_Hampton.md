### Layout

This is just a rough layout, not to scale, and not accounting for font size.

To the left, I've indicated the physical device that corresponds to each area of the screen.

```
{MODE NAME}      | 
                 |
{FEED or PITCH}  | --- Encoder
                 |
{UNITS TOGGLE}   | --- Button
{OTHER SETTING}  | --- Button
{OTHER SETTING}  | --- Button
```

### Mode 1: Threading

Example of cutting an English right-hand thread, with "Hardinge mode" (Rapid right) enabled
```
    THREADING    | 
                 |
      20TPI      | --- Encoder
                 |
      mm/in      | --- Button  (This changes the line above between TPI and mm, for inch or metric threads, respectively)
   RAPID RIGHT   | --- Button  (Options are RAPID RIGHT, RAPID LEFT, RAPID OFF)
                 | --- Button
```

Example of cutting a metric thread, with "Hardinge mode" disabled (no rapids in either direction)
```
    THREADING    | 
                 |
      1.75mm     | --- Encoder
                 |
      mm/in      | --- Button  (This changes the line above between TPI and mm, for inch or metric threads, respectively)
    RAPID OFF    | --- Button  (Options are RAPID RIGHT, RAPID LEFT, RAPID OFF)
                 | --- Button
```

### Mode 2: Power Feed

Example of power feed in inches per revolution
```
  Z POWER FEED   | 
                 |
   0.001"/REV    | --- Encoder
                 |
      mm/in      | --- Button  (This changes the line above between X"/REV and Xmm/REV)
                 | --- Button
                 | --- Button
```

Example of power feed in mm per revolution
```
  Z POWER FEED   | 
                 |
    0.2mm/REV    | --- Encoder
                 |
      mm/in      | --- Button  (This changes the line above between X"/REV and Xmm/REV)
                 | --- Button
                 | --- Button
```