# Compilation

Run this program with ./Assets/puppet.lua. I tested this on the VBox VM. I used c++17 instead of 14.

# Manual/Assumptions

I changed the background colour to purple.

The trackball demo example in the code shows that outside the trackball, the model rotates about its OWN z-axis, not the view's. However the assignment asks for a rotation about the view's z-axis. Thus, I assumed that's what was correct. To this end, you will find that if you translate the model away from the view's z-axis, and then perform a trackball rotation outside the trackball's bounds, the model will "orbit" (rotate around) the view's z-axis.

Furthermore, when a rotation occurs on the view's z-axis, all subsequent model translations and rotations are also rotated, because we just rotated the view's z-axis. I asked on a private post on Piazza and was told this is acceptable. See post @115.

In picking mode, you can click on joints to have them render in a red color to indicate they've been picked. I didn't render in false colors for all nodes once Joints mode is selected because it looks confusing with how the model is structured (it's not humanoid, really). I was assured this was okay by a TA and piazza post @100.

I also assumed that the head can be rotated side to side with the middle mouse button (if it's picked). Thus, the only difference between a middle click and a right click (in Joints mode) is that right click rotates the head if it's selected, while middle click rotates all selected joints (including the head).

Rotating the head with middle and right button at the same time will only apply the rotation to the head once (specifically, on the right click). This has absolutely no effect on the "being able to rotate joints with middle and right click at the same time" requirement, because it's only being applied to the head. This stops the head from being rotated 2x the angle that it should be rotated.

By "Reset Position", I assumed this meant "reset translations", NOT rotations (so the model will still look rotated if it's been rotated). Crucially, this also means that this button does not reset view rotations, so if you did view rotations, then the model might look like it's not in the right position until you also do a "Reset Orientation".

By "Reset Orientation", I assumed this meant "reset rotations", NOT translations (so the model will still be translated). This also includes resetting the view rotations.

Since the assignment didn't mention how to implement it, I did not implement proper support for undoing/redoing WHILE performing a joint rotation (eg: should an undo clear the in-progress rotation? Should it clear the prior one? Should it even do anything?).

I used Blender to create my model. Every mesh is mine except the head/face. For that, I used this mesh: https://www.turbosquid.com/3d-models/nextgen-head-3d-obj/525420

I read up on trackball rotations from https://www.xarg.org/2021/07/trackball-rotation-using-quaternions/

The model I used is (a simplified) "Lamassu". I'm of Iraqi origin and I wanted to portray this creature known widely from Assyrian mythology <3 Enjoy! https://en.wikipedia.org/wiki/Lamassu

# Data Structure Changes

I changed SceneNode to include custom rotate functions that do not rely on a character axis (also a rotate function for the view). I also gave it "local" transformation data. I also implemented most of selection in SceneNode - the selection map comes from there, and so does the function to convert node ID to RGB. I finally also gave it initial transformation data for reset to work properly.

For picking, I changed up scene_lua.cpp and GeometryNode to include the original material of the geometry node so I can switch back to it when the node is deselected. I also added z_axis joint rotation support to the scene_lua.cpp and hpp files.

I added a custom joint_rotate() function to Joint Node, and I also created the JointRotationCommand class in JointNode's files for undoing and redoing joint node rotations.

# High level Overview of Hierarchical Model

My model has 16 degrees of freedom. These are:

<ol>
    <li>Head (side-to-side rotation, click on the actual head to "pick" this)</li>
    <li>Tail (z-axis rotation)</li>
    <li>4x Upper leg (z-axis rotation. IMPORTANT: do NOT click on the balls/sockets. These count as part of the torso. Click on the legs themselves.)</li>
    <li>4x Lower leg (z-axis rotation)</li>
    <li>4x Hoof (z-axis rotation)</li>
    <li>2x Wings (x-axis rotation, flapping up and down)</li>
</ol>

<pre>
root 
├── Torso Mesh
│   ├── All four leg socket/ball meshes
│   └── Neck Mesh
├── Head Joint
│   └── Head Mesh
│       └── Eyes, beard, hat, hairballs on the sides of the face, lips, nose, etc. (meshes)
├── Left Wing Joint
│   └── Left Wing Mesh
├── Right Wing Joint
│   └── Right Wing Mesh
├── Tail Joint
│   └── Tail Mesh
│       └── Tail Hair Mesh
└── All four upper leg joints (condensed into this written node for brevity of writing)
    ├── All four upper leg meshes
    └── All four lower leg joints
        ├── All four lower leg meshes
        └── All four hoof joints
            └── All four hoof meshes
</pre>