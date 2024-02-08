

# Compilation

I used std=c++17 instead of 14. Program should be run from the A2 directory. Nothing else was changed. I tested my assignment on the VirtualBox VM.


# Manual

Viewport should work correctly, but clipping and perspective projections do not. I was hoping I'd understand the content in time for the deadline, but unfortunately not.

I will continue to work on this after the due date to see it finally working.

I made lots of helper functions and a Transform class to hold transformations.

I also assumed that by the eye "rotating about its local axis", that means the eye will not be orbitting the world origin, but rather literally rotating about its axes. This means that if the eye rotates about its y-axis, it 
should be able to rotate "behind" the camera, with nothing in view.

Thank you!
