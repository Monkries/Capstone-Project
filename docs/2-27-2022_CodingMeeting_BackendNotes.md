From meeting on 2-27 with Karson and Hampton

```c++

  // Scenario 1: Thread, Hardinge Mode, 1/2"-13
  // Step 1: gearbox setup (from control panel)
  ELS.configureGearing(13, "tpi");
  ELS.configureClutch(false, true, cfg_rapidStepRate);
  ELS.configureMotorBraking(true); // TODO: get from cpanel switch
  // Step 2: Feed lever input
  int feedLeverPosition = 0; //temporary stand-in for cpanel
  if (feedLeverPosition == 1) {
    // If feed lever is in the LEFT position
    ELS.engageZFeedLeft();
  }
  else if (feedLeverPosition == -1) {
    // If feed lever is in the RIGHT position
    ELS.engageZFeedRight();
  }
  else {
    // If feed lever is in the center OFF position
    ELS.disengageZFeed();
  }
  // Step 3: Cycle the backend
  ELS.cycle();
  // Step 3b: Update RPM display
  cPanel.displayRPM(ELS.getSpindleRPM());

  // Scenario 2: Power Feed at 0.003" per revolution
  // Step 1: Gearbox setup
  ELS.configureGearing(0.003, "in/rev");
  ELS.configureClutch(false, false, cfg_rapidStepRate);
  ELS.configureMotorBraking(true);
  // Step 2: Feed lever input
    int feedLeverPosition = 0; //temporary stand-in for cpanel
  if (feedLeverPosition == 1) {
    // If feed lever is in the LEFT position
    ELS.engageZFeedLeft();
  }
  else if (feedLeverPosition == -1) {
    // If feed lever is in the RIGHT position
    ELS.engageZFeedRight();
  }
  else {
    // If feed lever is in the center OFF position
    ELS.disengageZFeed();
  }
  // Step 3: Cycle backend
  ELS.cycle();
  // Step 3b: Update RPM display
  cPanel.displayRPM(ELS.getSpindleRPM());

  ```