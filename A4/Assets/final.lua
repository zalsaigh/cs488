-- A simple scene with some miscellaneous geometry.

mat1 = gr.material({0.7, 1.0, 0.7}, {0.5, 0.7, 0.5}, 25)
mat2 = gr.material({0.5, 0.5, 0.5}, {0.5, 0.7, 0.5}, 25)
mat3 = gr.material({1.0, 0.6, 0.1}, {0.5, 0.7, 0.5}, 25)
mat4 = gr.material({0.7, 0.6, 1.0}, {0.5, 0.4, 0.8}, 25)

basketballTexture = gr.texture("BasketballColor.png", {0.1, 0.1, 0.1}, 5)
basketballCourtTexture = gr.texture("BasketballCourtColor.png", {0.8, 0.8, 0.8}, 25)

scene_root = gr.node('root')

s1 = gr.nh_sphere('s1', {0, 0, 0}, 100)
scene_root:add_child(s1)
-- s1:set_material(mat1)
s1:set_texture(basketballTexture)

s2 = gr.nh_sphere('s2', {700, 400, -2000}, 400)
scene_root:add_child(s2)
s2:set_material(mat3)

-- s3 = gr.nh_sphere('s3', {0, -1200, -500}, 1000)
s3 = gr.nh_box_nosize('s3', {-1000, -300, -600}, {1000, -295, 600})
-- s3 = gr.mesh('s3', 'plane copy.obj')
scene_root:add_child(s3)
-- s3:set_material(mat2)
s3:set_texture(basketballCourtTexture)

-- b1 = gr.nh_box('b1', {-500, -225, -800}, 100)
b1 = gr.nh_box('b1', {-500, -225, -800}, 100)
scene_root:add_child(b1)
b1:set_material(mat4)

-- s4 = gr.nh_sphere('s4', {-100, 25, -300}, 50)
s4 = gr.nh_sphere('s4', {-150, 50, 225}, 100)
scene_root:add_child(s4)
s4:set_material(mat3)

s5 = gr.nh_sphere('s5', {400, 50, 225}, 125)
scene_root:add_child(s5)
s5:set_material(mat1)

-- A small stellated dodecahedron.

-- steldodec = gr.mesh( 'dodec', 'smstdodeca.obj' )
-- steldodec:set_material(mat3)
-- scene_root:add_child(steldodec)

-- white_light = gr.light({-100.0, 150.0, 400.0}, {0.9, 0.9, 0.9}, {1, 0, 0})
skylight = gr.light({0, 500, 1200}, {0.53, 0.81, 0.92}, {1.0, 0.0, 0.0})
-- magenta_light = gr.light({400.0, 100.0, 150.0}, {0.7, 0.0, 0.7}, {1, 0, 0})

gr.render(scene_root, 'final', 512, 512,
	  {0, 300, 800}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {skylight})
