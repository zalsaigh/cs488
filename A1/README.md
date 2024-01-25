

# Compilation

I used std=c++17 instead of 14. Program should be run from the A1 directory. Nothing else was changed. I tested my assignment on the VirtualBox VM.


# Manual

I assumed that walls of height 0 means that only the floor should show. I also assumed that this was a visual thing only (i.e. if you try to walk through a height=0 maze, you will still be stopped by invisible
blocks because the maze itself has not been deleted).

Since this was my first time making a sphere, I used an online resource for the math needed to generate the spheres. It is http://www.songho.ca/opengl/gl_sphere.html.

I assumed there should be a human error threshold for persistence to occur. This threshold should be very small and not noticeable, but just in case - be intentional with trying to cause persistence please :)

I made a bunch of helper functions and variables to help me out with the cleanliness of the code.

That's it! :D  
