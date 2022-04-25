This example shows how the calls that might be made to take two passes on a 13TPI thread.

First we set up the gearbox, then engage the forward feed, allowing the cutter to make its first pass.

Then (whenever the cutter gets to the end of the part, probably triggered by a user input) we disengage the clutch, and engage the reverse feed.

After allowing the cutter to return to the right end of the part, we engage the forward feed again to begin another cutting pass. There's no need to worry about synchronization in software or with a threading dial, the software clutch model takes care of it.
```cpp
  // Example 1: Cutting a 13TPI thread
  els.gearbox.configuredPitch = {13, tpi, rightHandThread_feedLeft}; // Set desired thread pitch
  els.gearbox.enableMotorBraking = true;
  // Start feeding toward the headstock
  els.clutch.engageForward(); 
  // Then return to the start for another pass
  els.clutch.disengage();
  els.clutch.engageReverse();
  els.clutch.disengage();
  // Take another pass (the system will maintain sync automatically)
  els.clutch.engageForward();
 ```
 
 Another example: Configuring the gearbox for an inch power feed rate, feeding 0.005" per revolution of the spindle.
 ```cpp
 
  // Example 2: Power Feeding 0.005" per revolution towards the headstock
  els.gearbox.configuredPitch = {0.005, in_per_rev, rightHandThread_feedLeft};
  els.gearbox.enableMotorBraking = true;
  els.clutch.engageForward();
```

Another example: Configuring the gearbox for a metric thread, 1.75mm pitch.

```cpp

  // Example 3: Cutting a metric 1.75mm pitch thread
  els.gearbox.configuredPitch = {1.75, mm, rightHandThread_feedLeft};
  els.clutch.engageForward();
```