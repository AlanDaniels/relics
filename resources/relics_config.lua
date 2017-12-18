-- Our main config file.
-- All through this file, we use human measurements: Meters, and Seconds.

-- The world itself.
world = {
    file_name = 'worlds/small_cube.world'
}

-- Debugging.
debug = {
    opengl = true,
    check_for_leaks = false,

    show_camera = false,
    show_game_clock = false,
    show_framerate = true,
    show_hit_test = false,
    show_mouse_pos = false,

    show_memory_usage = true,
    show_render_stats = true,
    show_eval_region = false,
    show_chunk_stats = false,

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
        vert_shader   = 'shaders/landscape_PNT.vert',
        frag_shader   = 'shaders/landscape_PNT.frag',

        grass_texture   = 'grass.png',
        dirt_texture    = 'dirt.png',
        stone_texture   = 'stone.png',
        bedrock_texture = 'bedrock.png'
    },

    sky = {
        vert_shader   = 'shaders/skybox_P.vert',
        frag_shader   = 'shaders/skybox_P.frag',

        north_texture  = 'skyboxes/nice_skybox_N.png',
        south_texture  = 'skyboxes/nice_skybox_S.png',
        east_texture   = 'skyboxes/nice_skybox_E.png',
        west_texture   = 'skyboxes/nice_skybox_W.png',
        top_texture    = 'skyboxes/nice_skybox_T.png',
        bottom_texture = 'skyboxes/nice_skybox_B.png'
    },

    hit_test = {
        vert_shader   = 'shaders/hit_test_PT.vert',
        frag_shader   = 'shaders/hit_test_PT.frag'
    }
}

-- Game logic.
-- Now, *this* will. Mess with at your own peril.
logic = {
    eval_blocks       = 3,
    hit_test_distance = 25.0
}

