# TeensyLeadscrew Library (and friends)

This is the backbone of the project, and it's designed to be extremely user-friendly, and easy to use in your own "frontends."

The TeensyLeadscrew class is the main thing you'll want to interact with, and it's meant to "feel" a lot like using traditional lathe controls (specifically patterned after Hardinge HLV-H's feed clutch system).

## TeensyLeadscrew Usage

To initialize the class, you have to pass in the hardware objects for the spindle encoder and the stepper motor, along with important specs about the lathe (leadscrew pitch, stepper motor max speed, encoder/spindle pulley ratio, etc). See the constructor for more thorough documentation.

To actually make it work, you just call ``TeensyLeadscrew::init()`` once during the ``setup()`` routine (after all your other backend hardware is ready), and then call ``TeensyLeadscrew::cycle()`` as often as possible.

To configure the virtual gearbox, make changes to the members of ``TeensyLeadscrew::gearbox``. This is where you set the pitch or power feed rate and direction, enable/disable rapid return, and whether or not you want the stepper to brake when the leadscrew isn't moving. This is analogous to setting the levers on a quick change threading gearbox.

To operate the feed clutch, use the member functions in ``TeensyLeadscrew::clutch``. There are 3: ``engageForward()``, ``engageReverse()``, and ``disengage()``. This is analogous to operating the clutch lever on a Hardinge HLV-H. If you want a more traditional non-clutched leadscrew (where you just engage/disengage the half nuts using a threading dial during threading), you can just call ``engageForward()`` once and leave the virtual clutch engaged all the time.

## Clutch Details

If you just want to use the leadscrew system as built, don't worry about this. But for developers, here's how the clutch works.

Its behavior is meant to copy the single-tooth dog clutch on Hardinge HLV-H and similar lathes. If you've never seen the threading process on these lathes, look it up, it's incredible.

This class works by modeling a handful of "virtual shafts," measuring their angular position in steps. The shaft angles are constrained between 0 and a configurable stepsPerRev number, to avoid integer overflow.

It's split into two "clutches," one for forward and one for reverse, with the ability to engage the output shaft to either one, or neither (neutral).

Here's a pseudo-controls-diagram of the model:

```
                                         --------------> [Forward Clutch Input Shaft Angle]---(X)---
                                         |                                                          |
Input (probably from spindle encoder) ---|                                                          |---> [Output Shaft Angle (stepper motor)]
                                         |                                                          |
                                         ----> [-1] ---> [Reverse Clutch Input Shaft      ]---( )---
```

The class doesn't have any direct access to hardware (such as the stepper motor). Rather, it relies on another class to log input shaft movement, and take the calculated output shaft movement.