-- Puppet A3:
--------------------------------------------------------------------------------
-- Create materials here
bullMaterial = gr.material({0.29, 0.22, 0.17}, {0.8, 0.8, 0.8}, 5.0) -- Brown colour
wingMaterial = gr.material({0.8, 0.8, 0.8}, {0.8, 0.8, 0.8}, 10.0)
hairMaterial = gr.material({0.1, 0.1, 0.1}, {0.8, 0.8, 0.8}, 5.0)
skinMaterial = gr.material({0.75, 0.56, 0.38}, {0.8, 0.8, 0.8}, 5.0)
hatMaterial = gr.material({0.85, 0.65, 0.13}, {1, 1, 1}, 30.0)
boneMaterial = gr.material({0.89, 0.85, 0.79}, {0.8, 0.8, 0.8}, 10.0)


--------------------------------------------------------------------------------
-- ALL TRANSLATION VALUES TAKEN FROM BLENDER --
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
-- Create the top level root node named 'root'.
rootNode = gr.node('root')
rootNode:translate(0.0, 0.0, -8.0)

--------------------------------------------------------------------------------
-- Torso
torsoMesh = gr.mesh('torso', 'torso')
torsoMesh:set_material(bullMaterial)

rootNode:add_child(torsoMesh)

--------------------------------------------------------------------------------
-- Wings

rightWingJoint = gr.joint('right_wing_joint', {0, 0, 90}, {0, 0, 0})
rootNode:add_child(rightWingJoint)

rightWingMesh = gr.mesh('wing', 'right_wing')
rightWingMesh:translate(-0.47, 2.08, 0.5)
rightWingMesh:set_material(wingMaterial)

rightWingJoint:add_child(rightWingMesh)

leftWingJoint = gr.joint('left_wing_joint', {-90, 0, 0}, {0, 0, 0})
rootNode:add_child(leftWingJoint)

leftWingMesh = gr.mesh('wing', 'left_wing')
leftWingMesh:translate(-0.47, 2.08, -0.5)
leftWingMesh:set_material(wingMaterial)

leftWingJoint:add_child(leftWingMesh)

--------------------------------------------------------------------------------
-- Tail

tailJoint = gr.joint('tail_joint', {-90, 0, 90}, {0, 0, 0})
rootNode:add_child(tailJoint)

tailMesh = gr.mesh("tail", "tail")
tailMesh:translate(-2.8, 0.0, 0.0)
tailMesh:set_material(bullMaterial)

tailJoint:add_child(tailMesh)

--------------------------------------------------------------------------------
-- Tail hair

tailHairMesh = gr.mesh("tail_hair", "tail_hair")
tailHairMesh:translate(-0.17, -1.04, 0.0)
tailHairMesh:set_material(hairMaterial)

tailMesh:add_child(tailHairMesh)

--------------------------------------------------------------------------------
-- Neck

neckMesh = gr.mesh('neck', 'neck')
neckMesh:translate(2.38, 1.24, 0.0)
neckMesh:set_material(bullMaterial)

torsoMesh:add_child(neckMesh)
--------------------------------------------------------------------------------
-- Head

headJoint = gr.mesh("headJoint", {0, 0, 0}, {-45, 0, 45})
rootNode:add_child(headJoint)

headMesh = gr.mesh("head", "head")
headMesh:translate(0.20, 0.60, 0.0)
headMesh:set_material(skinMaterial)

headJoint:add_child(headMesh)

--------------------------------------------------------------------------------
-- Beard
beardMesh = gr.mesh("beard", "beard")
beardMesh:translate(0.27, -0.36, 0.0)
beardMesh:set_material(hairMaterial)

headMesh:add_child(beardMesh)

--------------------------------------------------------------------------------
-- Lips
lipsMesh = gr.mesh("lips", "lips")
lipsMesh:translate(0.44,-0.31, 0.0)
lipsMesh:set_material(skinMaterial)

headMesh:add_child(lipsMesh)

--------------------------------------------------------------------------------
-- Eyebrows
eyebrowsMesh = gr.mesh("eyebrows", "eyebrows")
eyebrowsMesh:translate(0.36, 0.20, 0.0)
eyebrowsMesh:set_material(hairMaterial)

headMesh:add_child(eyebrowsMesh)

--------------------------------------------------------------------------------
-- Hat with connectors
hatWithConnectorsMesh = gr.mesh("hat_with_connectors", "hat_with_connectors")
hatWithConnectorsMesh:translate(-0.17, 0.96, 0.0)
hatWithConnectorsMesh:set_material(hatMaterial)

headMesh:add_child(hatWithConnectorsMesh)

--------------------------------------------------------------------------------
-- Hair balls next to hat
leftHairballMesh = gr.mesh("hairball", "left_hairball")
leftHairballMesh:translate(0.13, -1.2, -0.56)
leftHairballMesh:set_material(hairMaterial)

hatWithConnectorsMesh:add_child(leftHairballMesh)

rightHairballMesh = gr.mesh("hairball", "right_hairball")
rightHairballMesh:translate(0.13, -1.2, 0.56)
rightHairballMesh:set_material(hairMaterial)

hatWithConnectorsMesh:add_child(rightHairballMesh)

--------------------------------------------------------------------------------
-- Eyes
rightEyeballMesh = gr.mesh("eyeball", "right_eyeball")
rightEyeballMesh:translate(0.26, 0.10, 0.17)
rightEyeballMesh:set_material(wingMaterial)

headMesh:add_child(rightEyeballMesh)

leftEyeballMesh = gr.mesh("eyeball", "left_eyeball")
leftEyeballMesh:translate(0.26, 0.10, -0.17)
leftEyeballMesh:set_material(wingMaterial)

headMesh:add_child(leftEyeballMesh)

--------------------------------------------------------------------------------
-- Right Front Leg
upperRightFrontLegBallMesh = gr.mesh("leg_socket", "upper_right_front_leg_ball")
upperRightFrontLegBallMesh:translate(1.6, -0.36, 0.81)
upperRightFrontLegBallMesh:set_material(bullMaterial)

torsoMesh:add_child(upperRightFrontLegBallMesh)

upperRightFrontLegJoint = gr.joint('upper_right_front_leg_joint', {0, 0, 90}, {0, 0, 90})
rootNode:add_child(upperRightFrontLegJoint)

upperRightFrontLegMesh = gr.mesh("upper_leg", "upper_right_front_leg")
upperRightFrontLegMesh:translate(0.0, -0.61, 0.0)
upperRightFrontLegMesh:set_material(bullMaterial)

upperRightFrontLegJoint:add_child(upperRightFrontLegMesh)

lowerRightFrontLegJoint = gr.joint('lower_right_front_leg_joint', {0, 0, 90}, {0, 0, 90})
upperRightFrontLegJoint:add_child(lowerRightFrontLegJoint)

lowerRightFrontLegMesh = gr.mesh("lower_leg", "lower_right_front_leg")
lowerRightFrontLegMesh:translate(0.0, -1.1, 0.0)
lowerRightFrontLegMesh:set_material(bullMaterial)

lowerRightFrontLegJoint:add_child(lowerRightFrontLegMesh)

rightFrontHoofJoint = gr.joint('right_front_hoof_joint', {0, 0, 90}, {0, 0, 90})
lowerRightFrontLegJoint:add_child(rightFrontHoofJoint)

rightFrontHoofMesh = gr.mesh("hoof", "right_front_hoof")
rightFrontHoofMesh:translate(0.0, -0.53, 0.0)
rightFrontHoofMesh:set_material(boneMaterial)

rightFrontHoofJoint:add_child(rightFrontHoofMesh)

--------------------------------------------------------------------------------
-- Left Front Leg
upperLeftFrontLegBallMesh = gr.mesh("leg_socket", "upper_left_front_leg_ball")
upperLeftFrontLegBallMesh:translate(1.6, -0.36, -0.81)
upperLeftFrontLegBallMesh:set_material(bullMaterial)

torsoMesh:add_child(upperLeftFrontLegBallMesh)

upperLeftFrontLegJoint = gr.joint("upper_left_front_leg_joint", {0, 0, 90}, {0, 0, 0})
rootNode:add_child(upperLeftFrontLegJoint)

upperLeftFrontLegMesh = gr.mesh("upper_leg", "upper_left_front_leg")
upperLeftFrontLegMesh:translate(0.0, -0.61, 0.0)
upperLeftFrontLegMesh:set_material(bullMaterial)

upperLeftFrontLegJoint:add_child(upperLeftFrontLegMesh)

lowerLeftFrontLegJoint = gr.joint("lower_left_front_leg_joint", {0, 0, 90}, {0, 0, 0})
upperLeftFrontLegJoint:add_child(lowerLeftFrontLegJoint)

lowerLeftFrontLegMesh = gr.mesh("lower_leg", "lower_left_front_leg")
lowerLeftFrontLegMesh:translate(0.0, -1.1, 0.0)
lowerLeftFrontLegMesh:set_material(bullMaterial)

lowerLeftFrontLegJoint:add_child(lowerLeftFrontLegMesh)

leftFrontHoofJoint = gr.joint("left_front_hoof_joint", {0, 0, 90}, {0, 0, 0})
lowerLeftFrontLegJoint:add_child(leftFrontHoofJoint)

leftFrontHoofMesh = gr.mesh("hoof", "left_front_hoof")
leftFrontHoofMesh:translate(0.0, -0.53, 0.0)
leftFrontHoofMesh:set_material(boneMaterial)

leftFrontHoofJoint:add_child(leftFrontHoofMesh)

--------------------------------------------------------------------------------
-- Right Back Leg
upperRightBackLegBallMesh = gr.mesh("leg_socket", "upper_right_back_leg_ball")
upperRightBackLegBallMesh:translate(-1.9, -0.36, 0.81)
upperRightBackLegBallMesh:set_material(bullMaterial)

torsoMesh:add_child(upperRightBackLegBallMesh)

upperRightBackLegMesh = gr.mesh("upper_leg", "upper_right_back_leg")
upperRightBackLegMesh:translate(0.0, -0.61, 0.0)
upperRightBackLegMesh:set_material(bullMaterial)

upperRightBackLegBallMesh:add_child(upperRightBackLegMesh)

lowerRightBackLegMesh = gr.mesh("lower_leg", "lower_right_back_leg")
lowerRightBackLegMesh:translate(0.0, -1.1, 0.0)
lowerRightBackLegMesh:set_material(bullMaterial)

upperRightBackLegMesh:add_child(lowerRightBackLegMesh)

rightBackHoofMesh = gr.mesh("hoof", "right_back_hoof")
rightBackHoofMesh:translate(0.0, -0.53, 0.0)
rightBackHoofMesh:set_material(boneMaterial)

lowerRightBackLegMesh:add_child(rightBackHoofMesh)

--------------------------------------------------------------------------------
-- Left Back Leg
upperLeftBackLegBallMesh = gr.mesh("leg_socket", "upper_left_back_leg_ball")
upperLeftBackLegBallMesh:translate(-1.9, -0.36, -0.81)
upperLeftBackLegBallMesh:set_material(bullMaterial)

torsoMesh:add_child(upperLeftBackLegBallMesh)

upperLeftBackLegMesh = gr.mesh("upper_leg", "upper_left_back_leg")
upperLeftBackLegMesh:translate(0.0, -0.61, 0.0)
upperLeftBackLegMesh:set_material(bullMaterial)

upperLeftBackLegBallMesh:add_child(upperLeftBackLegMesh)

lowerLeftBackLegMesh = gr.mesh("lower_leg", "lower_left_back_leg")
lowerLeftBackLegMesh:translate(0.0, -1.1, 0.0)
lowerLeftBackLegMesh:set_material(bullMaterial)

upperLeftBackLegMesh:add_child(lowerLeftBackLegMesh)

leftBackHoofMesh = gr.mesh("hoof", "left_back_hoof")
leftBackHoofMesh:translate(0.0, -0.53, 0.0)
leftBackHoofMesh:set_material(boneMaterial)

lowerLeftBackLegMesh:add_child(leftBackHoofMesh)

-- Return the root with all of it's childern.  The SceneNode A3::m_rootNode will be set
-- equal to the return value from this Lua script.
return rootNode
