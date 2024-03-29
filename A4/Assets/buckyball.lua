-- A simple scene with some miscellaneous geometry.

mat3 = gr.material({0.1, 0.7, 0.1}, {0.0, 0.0, 0.0}, 5)

scene_root = gr.node('root')

-- A buckyball

buckyball = gr.mesh( 'buckyball', 'buckyball.obj' )
buckyball:set_material(mat3)
scene_root:add_child(buckyball)

white_light = gr.light({0.0, 50.0, -1.0}, {0.9, 0.9, 0.9}, {1, 0, 0})

gr.render(scene_root, 'buckyball.png', 512, 512,
	  {0, 0, 10}, {0, 0, -1}, {0, 1, 0}, 50,
	  {0.3, 0.3, 0.3}, {white_light})