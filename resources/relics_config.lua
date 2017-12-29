-- Our main config file.
-- We always use human measurements here, such as meters and seconds,
-- rather than game ticks or game units, etc.

-- The world itself.
world = {
    file_name = 'worlds/small_cube.world'
}

-- Debugging.
debug = {
    opengl = true,
    check_for_leaks = false,
    draw_transitions = true,

    hud_camera = false,
    hud_game_clock = false,
    hud_framerate = true,
    hud_hit_test = false,
    hud_mouse_pos = false,

    hud_memory_usage = true,
    hud_render_stats = true,
    hud_eval_region = false,
    hud_chunk_stats = false,

    print_draw_state = false,
    print_window_context = false,
    
    noclip_flight_speed = 5.0
}

-- Window details.
-- NOTE: This should not affect the game logic in any way.
window = {
    width  = 1024,
    height = 768,
    fullscreen = false,
    vertical_sync = true,
    mouse_degrees_per_pixel = 0.1
}

-- Rendering.
-- NOTE: This should not affect the game logic in any way.
render = {
    cull_backfaces = true,

    field_of_view  = 90.0,
    near_plane     = 0.1,
    far_plane      = 1000.0,
    fade_distance  = 5.0,

    landscape = {
        vert_shader = 'shaders/landscape_PNT.vert',
        frag_shader = 'shaders/landscape_PNT.frag',

        grass_texture   = 'grass.png',
        dirt_texture    = 'dirt.png',
        stone_texture   = 'stone.png',
        coal_texture    = 'cartoon_dirt.jpg', -- TODO: Find some coal.
        bedrock_texture = 'bedrock.png'
    },

    sky = {
        vert_shader = 'shaders/skybox_P.vert',
        frag_shader = 'shaders/skybox_P.frag',

        north_texture  = 'skyboxes/nice_skybox_N.png',
        south_texture  = 'skyboxes/nice_skybox_S.png',
        east_texture   = 'skyboxes/nice_skybox_E.png',
        west_texture   = 'skyboxes/nice_skybox_W.png',
        top_texture    = 'skyboxes/nice_skybox_T.png',
        bottom_texture = 'skyboxes/nice_skybox_B.png'
    },

    hit_test = {
        vert_shader = 'shaders/hit_test_PT.vert',
        frag_shader = 'shaders/hit_test_PT.frag'
    }
}

-- Game logic.
-- Now, *this* will. Mess with at your own peril.
logic = {
    eval_blocks       = 3,
    hit_test_distance = 25.0
}

