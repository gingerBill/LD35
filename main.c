/*	Game: Transmog
	Ludum Dare 35 - Shapeshift
	Copyright: (c) 2016 Ginger Bill

	This game was made for the Ludum Dare 35 Compo so most of this code
	is dreadful. The game would have not been possible without
	Martin Cohen' Punity Engine which allowed for a palletized display,
	audio stream, and input handling.

	The code is mainly FSMs upon FSMs and really could have been better
	done. It works (kind of)!

	This game was totally "Not to a Pokemon Clone"(TM). Most of the final
	game is missing due to time constraints and life in general.

	I hope you had fun playing the game and have "fun" reading this game.
 */
#define RELEASE_BUILD
#include "punity.c"

#define GB_IMPLEMENTATION
#include "gb.h"
#define GB_MATH_IMPLEMENTATION
#include "gb_math.h"

#include "maps.c"


#define TILE_SIZE 16

typedef enum {
	TRANSMOG_TYPE_LINE,
	TRANSMOG_TYPE_TRIANGLE,
	TRANSMOG_TYPE_SQUARE,
	TRANSMOG_TYPE_PENTAGON,
	TRANSMOG_TYPE_HEXAGON,

	TRANSMOG_TYPE_CIRCLE,
	TRANSMOG_TYPE_SPHERE,

	TRANSMOG_TYPE_COUNT,
} Transmog_Type;

typedef struct {
	Transmog_Type type;
	i32 health;
	i32 max_health;
	i32 attack_power;
	i32 shift_level;
	i32 xp; // 10xp == level up
} Transmog;

void update_transmog_stats(Transmog *mog);

gb_internal Transmog
make_transmog(Transmog_Type type) {
	Transmog mog;
	gb_zero_struct(&mog);
	mog.type = type;
	mog.shift_level = 1;

	switch (type) {
	case TRANSMOG_TYPE_LINE:
		mog.health = mog.max_health = 5;
		mog.attack_power = 15;
		break;
	case TRANSMOG_TYPE_TRIANGLE:
		mog.health = mog.max_health = 7;
		mog.attack_power = 12;
		break;

	case TRANSMOG_TYPE_SQUARE:
		mog.health = mog.max_health = 9;
		mog.attack_power = 10;
		break;
	case TRANSMOG_TYPE_PENTAGON:
		mog.health = mog.max_health = 12;
		mog.attack_power = 8;
		mog.shift_level = gb_random_range_int(2, 4);
		break;
	case TRANSMOG_TYPE_HEXAGON:
		mog.health = mog.max_health = 15;
		mog.attack_power = 7;
		mog.shift_level = gb_random_range_int(2, 4);
		break;
	case TRANSMOG_TYPE_CIRCLE:
		mog.health = mog.max_health = 18;
		mog.attack_power = 6;
		mog.shift_level = gb_random_range_int(3, 4);
		break;
	case TRANSMOG_TYPE_SPHERE:
		mog.health = mog.max_health = 30;
		mog.attack_power = 6;
		mog.shift_level = gb_random_range_int(6, 8);
		break;
	}

	update_transmog_stats(&mog);

	return mog;
}


void
update_transmog_stats(Transmog *mog) {
	mog->max_health   += (mog->shift_level-1);
	mog->attack_power += (mog->shift_level-1);
}


char const *
transmog_name(Transmog_Type type)
{
	switch (type) {
	case TRANSMOG_TYPE_LINE:     return "LINE";
	case TRANSMOG_TYPE_TRIANGLE: return "TRIANGLE";
	case TRANSMOG_TYPE_SQUARE:   return "SQUARE";
	case TRANSMOG_TYPE_PENTAGON: return "PENTAGON";
	case TRANSMOG_TYPE_HEXAGON:  return "HEXAGON";
	case TRANSMOG_TYPE_CIRCLE:   return "CIRCLE";
	case TRANSMOG_TYPE_SPHERE:   return "SPHERE";
	}
	return "UNKNOWN";
}


typedef struct {
	gbVec2 pos;
	gbVec2 vel;
	// 0 - down
	// 1 - up
	// 2 - left
	// 3 - right (-ve left)
	i32 orientation;
	b32 is_moving;
	b32 in_grass;

	Transmog my_transmog;
} Player;


typedef struct {
	char *name;
	i32 width, height;
	char **tiles;
	b32 outside;
} Map;


typedef enum {
	GAME_MODE_TITLE,
	GAME_MODE_MAP,
	GAME_MODE_BATTLE,
	GAME_MODE_END_FAIL,
	GAME_MODE_END_SUCCESS,
} Game_Mode;

typedef enum {
	BATTLE_TYPE_GRASS,
	BATTLE_TYPE_BOSS,
} Battle_Type;

typedef enum {
	BATTLE_TURN_STATE_INTRO,
	BATTLE_TURN_STATE_WHAT_TO_DO,
	BATTLE_TURN_STATE_FIGHT,
	BATTLE_TURN_STATE_SUBSTITUTE,
	BATTLE_TURN_STATE_ESPACE,
} Battle_Turn_State;

typedef enum {
	FIGHTING_STATE_NONE,
	FIGHTING_STATE_USE,
	FIGHTING_STATE_EFFECTIVE,
	FIGHTING_STATE_SUPER_EFFECTIVE,
	FIGHTING_STATE_NO_EFFECT,
	FIGHTING_STATE_NEXT_TURN,
	FIGHTING_STATE_ENEMY_DEAD,
	FIGHTING_STATE_PLAYER_DEAD,
} Fighting_State;

typedef struct {
	Battle_Type type;
	Transmog *player_mog;
	Transmog  enemy_mog;

	Battle_Turn_State state;
	u32 state_index;
	b32 is_enemies_turn;
	Fighting_State fighting_state;
	u32 attack_index;
} Battle_Mode;

typedef struct {
#define MAX_MAPS 16 // NOTE(bill): Are there numbers bigger than 16?
	Map maps[MAX_MAPS];
	i32 curr_map_index;

	Game_Mode mode;

	Player player;

	i32 show_map_name_ticks;
	i32 grass_event_cooldown;

	i32 doctor_line_index;
	b32 has_chosen_transmog;

	Battle_Mode battle_mode;

	i32 intro_cooldown;
} Game;


gb_global char *doctor_lines[] = {
	"Hello, you. I'm The Doctor.",
	"I'm doing good if I don't me saying.",
	"I guess my friend has lead you here?",
	"You might be asking why, yes?",
	"That area has Transmogs in it.",
	"They're shapes that gained...",
	"FIGHTING ABILITIES and sentience!",
	"It sounds crazy but it's true!",
	"The only way to beat geometry is...",
	"You guessed it, with Transmogs.",
	"...",
	"How old are you? Ten?!",
	"You don't need parental supervision.",
	"Transmogs won't hurt you, much.",
	"You'll need your very own Transmog.",
	"Which one will you take?",
};


typedef enum {
	PC_TRANSPARENT,
	PC_TRUE_BLACK,
	PC_WHITE,
	PC_LIGHT_GREY,
	PC_DARK_GREY,
	PC_BLACK,

	PC_COUNT,
} PaletteColor;

typedef enum {
	PS_WHITE,
	PS_BEIGE,
	PS_BLUE,
	PS_GREEN,
	PS_RED,
	PS_RED_GREEN,
	PS_PASTEL_MIX,

	PS_COUNT,
} PaletteSet;


gb_global Bitmap title_screen;
gb_global Bitmap spritesheet;
gb_global Font font;

gb_global Game game;


gb_internal void
change_map_to(i32 to_index) {
	Map *map;
	i32 i, j;
	i32 from_index;

	from_index = game.curr_map_index;
	game.curr_map_index = to_index;
	map = &game.maps[game.curr_map_index];

	for (j = 0; j < map->height; j++) {
		for (i = 0; i < map->width; i++) {
			char t = map->tiles[j][i];
			if (gb_char_is_digit(t) && (t-'0') == from_index) {
				if (j > 0 && map->tiles[j-1][i] == '.') {
					game.player.pos = gb_vec2(TILE_SIZE*i, TILE_SIZE*(j-1));
				}
				if (j <map->height-1 && map->tiles[j+1][i] == '.') {
					game.player.pos = gb_vec2(TILE_SIZE*i, TILE_SIZE*(j+1));
				}
			}
		}
	}

	game.show_map_name_ticks = 60;
}



gb_internal void
change_to_battle_mode(Battle_Type type)
{
	// TODO(bill): Change for different battle types
	game.mode = GAME_MODE_BATTLE;
	game.battle_mode.type = type;
	game.battle_mode.player_mog = &game.player.my_transmog;

	if (type == BATTLE_TYPE_GRASS) {
		game.battle_mode.enemy_mog = make_transmog(cast(Transmog_Type)gb_random_range_int(TRANSMOG_TYPE_LINE, TRANSMOG_TYPE_HEXAGON));
	} else {
		game.battle_mode.enemy_mog = make_transmog(TRANSMOG_TYPE_SPHERE);
	}

	game.battle_mode.state = BATTLE_TURN_STATE_INTRO;
	game.battle_mode.state_index = 0;
	game.battle_mode.is_enemies_turn = false;
	game.battle_mode.fighting_state = FIGHTING_STATE_NONE;
}

gb_internal void
end_battle_mode_turn(void)
{
	if (!game.battle_mode.is_enemies_turn) {
		game.battle_mode.is_enemies_turn = true;
	} else {
		game.battle_mode.is_enemies_turn = false;
	}
	game.battle_mode.fighting_state = FIGHTING_STATE_NONE;
	game.battle_mode.state_index    = 0;
	game.battle_mode.attack_index   = 0;
}



gb_internal void
set_palette_set(PaletteSet set)
{
	switch (set) {
	case PS_BEIGE:
		CORE->palette.colors[PC_WHITE]       = color_make(0xe6, 0xd6, 0x9c, 0xff);
		CORE->palette.colors[PC_LIGHT_GREY]  = color_make(0xb4, 0xa5, 0x6a, 0xff);
		CORE->palette.colors[PC_DARK_GREY]   = color_make(0x7b, 0x71, 0x62, 0xff);
		CORE->palette.colors[PC_BLACK]       = color_make(0x39, 0x38, 0x29, 0xff);
		break;
	case PS_GREEN:
		CORE->palette.colors[PC_WHITE]       = color_make(0xd7, 0xe8, 0x94, 0xff);
		CORE->palette.colors[PC_LIGHT_GREY]  = color_make(0xae, 0xc4, 0x40, 0xff);
		CORE->palette.colors[PC_DARK_GREY]   = color_make(0x52, 0x7f, 0x39, 0xff);
		CORE->palette.colors[PC_BLACK]       = color_make(0x20, 0x46, 0x31, 0xff);
		break;
	case PS_BLUE:
		CORE->palette.colors[PC_WHITE]       = color_make(0xe0, 0xf0, 0xe8, 0xff);
		CORE->palette.colors[PC_LIGHT_GREY]  = color_make(0xa8, 0xc0, 0xb0, 0xff);
		CORE->palette.colors[PC_DARK_GREY]   = color_make(0x50, 0x78, 0x68, 0xff);
		CORE->palette.colors[PC_BLACK]       = color_make(0x18, 0x30, 0x30, 0xff);
		break;
	case PS_RED:
		CORE->palette.colors[PC_WHITE]       = color_make(0xff, 0xe5, 0xe6, 0xff);
		CORE->palette.colors[PC_LIGHT_GREY]  = color_make(0xec, 0x8a, 0x8c, 0xff);
		CORE->palette.colors[PC_DARK_GREY]   = color_make(0xac, 0x26, 0x24, 0xff);
		CORE->palette.colors[PC_BLACK]       = color_make(0x63, 0x14, 0x14, 0xff);
		break;
	case PS_RED_GREEN:
		CORE->palette.colors[PC_WHITE]       = color_make(0xfc, 0xfa, 0xfc, 0xff);
		CORE->palette.colors[PC_LIGHT_GREY]  = color_make(0x04, 0xfa, 0x04, 0xff);
		CORE->palette.colors[PC_DARK_GREY]   = color_make(0xfc, 0x32, 0x04, 0xff);
		CORE->palette.colors[PC_BLACK]       = color_make(0x04, 0x02, 0x02, 0xff);
		break;
	case PS_PASTEL_MIX:
		CORE->palette.colors[PC_WHITE]       = color_make(0xfc, 0xfa, 0xac, 0xff);
		CORE->palette.colors[PC_LIGHT_GREY]  = color_make(0xec, 0x8a, 0x8c, 0xff);
		CORE->palette.colors[PC_DARK_GREY]   = color_make(0x9c, 0x92, 0xf4, 0xff);
		CORE->palette.colors[PC_BLACK]       = color_make(0x04, 0x02, 0x02, 0xff);
		break;
	case PS_WHITE:
		CORE->palette.colors[PC_WHITE]       = color_make(0xff, 0xff, 0xff, 0xff);
		CORE->palette.colors[PC_LIGHT_GREY]  = color_make(0x8f, 0x8f, 0x8f, 0xff);
		CORE->palette.colors[PC_DARK_GREY]   = color_make(0x6f, 0x6f, 0x6f, 0xff);
		CORE->palette.colors[PC_BLACK]       = color_make(0x31, 0x31, 0x31, 0xff);
		break;
	}
}

gb_internal void
init_game(void)
{
	gb_zero_struct(&game);

	{ // Register Maps
	#define REGISTER_MAP(n) \
		game.maps[n].name    = map##n##_name;    \
		game.maps[n].width   = map##n##_width;   \
		game.maps[n].height  = map##n##_height;  \
		game.maps[n].outside = map##n##_outside; \
		game.maps[n].tiles   = map##n

		REGISTER_MAP(0);
		REGISTER_MAP(1);
		REGISTER_MAP(2);
		REGISTER_MAP(3);
		REGISTER_MAP(4);
		REGISTER_MAP(5);
		REGISTER_MAP(6);
		REGISTER_MAP(7);

	#undef REGISTER_MAP
	}

	game.curr_map_index = 1;
	change_map_to(0);


	game.mode = GAME_MODE_TITLE;
	game.intro_cooldown = 60;
#if 0
	game.curr_map_index = 6;
	change_map_to(7);

	game.player.my_transmog = make_transmog(TRANSMOG_TYPE_HEXAGON);
	game.player.my_transmog.shift_level = 3;
	update_transmog_stats(&game.player.my_transmog);
#endif
}

void
init(void)
{
	CORE->palette.colors[PC_TRANSPARENT] = color_make(0x00, 0x00, 0x00, 0x00);
	CORE->palette.colors[PC_TRUE_BLACK]  = color_make(0x00, 0x00, 0x00, 0xff);
	set_palette_set(PS_WHITE); // IMPORTANT NOTE(bill): This must be white as the spritesheet is white

	CORE->palette.colors_count = PC_COUNT;
	canvas_clear(1);

	bitmap_load_resource(&spritesheet, "spritesheet.png");
	bitmap_load_resource(&title_screen, "title_screen.png");

	bitmap_load_resource(&font.bitmap, "font.png");
	font.char_width = 4;
	font.char_height = 7;


	CORE->font = &font;

	init_game();

#if 0
	set_palette_set(PS_WHITE);
#endif
}

gb_internal void
draw_sprite_size(i32 x, i32 y, i32 sx, i32 sy, i32 w, i32 h, u32 flags)
{
	Rect r = rect_make_size(sx*TILE_SIZE, sy*TILE_SIZE, w*TILE_SIZE, h*TILE_SIZE);
	bitmap_draw(x, y, 0, 0, &spritesheet, &r, flags, 0);
}

void draw_sprite(i32 x, i32 y, i32 sx, i32 sy, u32 flags) { draw_sprite_size(x, y, sx, sy, 1, 1, flags); }

gb_internal void
move_game_camera(Map *map)
{
	i32 canvas_mid_x = (CANVAS_WIDTH -TILE_SIZE)/2;
	i32 canvas_mid_y = (CANVAS_HEIGHT-TILE_SIZE)/2;
	i32 width = map->width*TILE_SIZE;
	i32 height = map->height*TILE_SIZE;
	i32 off_x = (CANVAS_WIDTH-width)/2;
	i32 off_y = (CANVAS_HEIGHT-height)/2;

	CORE->translate_x = 0;
	CORE->translate_y = 0;

	if (width <= CANVAS_WIDTH) {
		CORE->translate_x = off_x;
	} else {
		if (game.player.pos.x > canvas_mid_x)
			CORE->translate_x = -game.player.pos.x + canvas_mid_x;

		if (game.player.pos.x + canvas_mid_x + TILE_SIZE >= width)
			CORE->translate_x = -(width-TILE_SIZE - 2.0f*canvas_mid_x);
	}

	if (height <= CANVAS_HEIGHT) {
		CORE->translate_y = off_y;
	} else {
		if (game.player.pos.y > canvas_mid_y)
			CORE->translate_y = -game.player.pos.y + canvas_mid_y;

		if (game.player.pos.y + canvas_mid_y + TILE_SIZE >= height)
			CORE->translate_y = -(height-TILE_SIZE - 2.0f*canvas_mid_y);
	}
}

void
draw_message_box(char *line1, char *line2, char *line3)
{
	i32 i, w = CANVAS_WIDTH, h = CANVAS_HEIGHT;
	draw_sprite(0, h-1-2.0f*TILE_SIZE, 0, 4, 0);
	draw_sprite(0, h-1-1.0f*TILE_SIZE, 0, 5, 0);
	draw_sprite(w-1-TILE_SIZE, h-1-2.0f*TILE_SIZE, 0, 4, DrawFlags_FlipH);
	draw_sprite(w-1-TILE_SIZE, h-1-1.0f*TILE_SIZE, 0, 5, DrawFlags_FlipH);

	for (i = TILE_SIZE; i < w-1-TILE_SIZE; i += TILE_SIZE) {
		draw_sprite(i, h-1-2.0f*TILE_SIZE, 1, 4, 0);
		draw_sprite(i, h-1-1.0f*TILE_SIZE, 1, 5, 0);
	}

	rect_draw(rect_make_size(5, h-1-27, w-1-10, 22), PC_WHITE);

	if (line1) text_draw(8, 116+0*font.char_height, line1, PC_BLACK);
	if (line2) text_draw(8, 116+1*font.char_height, line2, PC_BLACK);
	if (line3) text_draw(8, 116+2*font.char_height, line3, PC_BLACK);
}


gb_global b32 draw_tooltip = false;
gb_global char *tooltip_messages[3] = {0};

gb_internal void
set_tooltip(i32 x, i32 y, f32 prox, char *line1, char *line2, char *line3)
{
	f32 dx = game.player.pos.x - x;
	f32 dy = game.player.pos.y - y;
	if (dx*dx+dy*dy < square(prox*TILE_SIZE)) {
		draw_tooltip = true;
		if (line1) tooltip_messages[0] = line1;
		if (line2) tooltip_messages[1] = line2;
		if (line3) tooltip_messages[2] = line3;
	}
}


gb_internal void
draw_big_transmog(i32 x, i32 y, Transmog_Type type)
{
	i32 sx = 0, sy = 0;

	switch (type) {
	case TRANSMOG_TYPE_LINE:     sx =  0; sy =  8; break;
	case TRANSMOG_TYPE_TRIANGLE: sx =  0; sy = 12; break;
	case TRANSMOG_TYPE_SQUARE:   sx =  4; sy =  8; break;
	case TRANSMOG_TYPE_PENTAGON: sx =  4; sy = 12; break;
	case TRANSMOG_TYPE_HEXAGON:  sx =  8; sy =  8; break;
	case TRANSMOG_TYPE_CIRCLE:   sx =  8; sy = 12; break;
	case TRANSMOG_TYPE_SPHERE:   sx = 12; sy =  8; break;
	default: GB_PANIC("UNKNOWN Transmog Type");    break;
	}

	draw_sprite_size(x, y, sx, sy, 4, 4, 0);
}


void
step(void)
{


	if (key_pressed(KEY_ESCAPE)) {
		CORE->running = 0;
		return;
	}

	     if (key_pressed(KEY_1)) set_palette_set(PS_WHITE);
	else if (key_pressed(KEY_2)) set_palette_set(PS_GREEN);
	else if (key_pressed(KEY_3)) set_palette_set(PS_BEIGE);
	else if (key_pressed(KEY_4)) set_palette_set(PS_BLUE);
	else if (key_pressed(KEY_5)) set_palette_set(PS_RED);
	else if (key_pressed(KEY_6)) set_palette_set(PS_PASTEL_MIX);
	else if (key_pressed(KEY_7)) set_palette_set(PS_RED_GREEN);




	switch (game.mode) {
	case GAME_MODE_TITLE: {
		if (game.intro_cooldown --> 0) {
			canvas_clear(PC_TRANSPARENT);
			bitmap_draw(0, 0, 0, 0, &title_screen, NULL, 0, 0);
		} else {
			game.mode = GAME_MODE_MAP;
		}
	} break;

	case GAME_MODE_MAP: {
		Map *map = &game.maps[game.curr_map_index];
		i32 i, j, x, y;
		gbVec2 old_player_pos = game.player.pos;
		b32 render_doctor_text = false;


		canvas_clear(PC_TRANSPARENT);

		game.player.vel = gb_vec2_zero();
		if (key_down(KEY_UP))    { game.player.vel.y -= 1.0f; }
		if (key_down(KEY_DOWN))  { game.player.vel.y += 1.0f; }
		if (key_down(KEY_LEFT))  { game.player.vel.x -= 1.0f; }
		if (key_down(KEY_RIGHT)) { game.player.vel.x += 1.0f; }
		gb_vec2_norm0(&game.player.vel, game.player.vel);
		gb_vec2_muleq(&game.player.vel, game.player.in_grass ? 2.5f : 3.5f);
		gb_vec2_addeq(&game.player.pos, game.player.vel);
		game.player.in_grass = false;

		game.player.is_moving = (gb_vec2_mag(game.player.vel) > 0);

		if (game.player.vel.x < 0) game.player.orientation = 2;
		if (game.player.vel.x > 0) game.player.orientation = 3;
		if (game.player.vel.y > 0) game.player.orientation = 0;
		if (game.player.vel.y < 0) game.player.orientation = 1;

		game.player.pos.x = gb_clamp(game.player.pos.x, 0, (map->width-1) *TILE_SIZE);
		game.player.pos.y = gb_clamp(game.player.pos.y, 0, (map->height-1)*TILE_SIZE);


		game.grass_event_cooldown--;
		if (game.grass_event_cooldown < 0)
			game.grass_event_cooldown = 0;

		draw_tooltip = false;
		gb_zero_array(tooltip_messages, gb_count_of(tooltip_messages));


		{ // Handle Interactions
			Rect player_rect = rect_make_size(game.player.pos.x+2, game.player.pos.y+12,
			                                  12, 4);
			for (j = 0; j < map->height; j++) {
				y = j*TILE_SIZE;
				for (i = 0; i < map->width; i++) {
					Rect tile_rect;
					char t = map->tiles[j][i];
					x = i*TILE_SIZE;
					tile_rect = rect_make_size(x, y, TILE_SIZE, TILE_SIZE);

					if (gb_char_is_digit(t)) {
						if (rect_collides(player_rect, tile_rect, NULL)) {
							change_map_to(t-'0');
							return;
						}
					}
					if ((t != ' ' && t != '.' && t != 'g')) {
						Rect inter;
						b32 do_collision = true;
						if (t == 'L') { // NOTE(bill): Lab Chemicals Top
							tile_rect = rect_make_size(x, y+7, TILE_SIZE, TILE_SIZE-7);
						}

						if (t == 'Q' && game.has_chosen_transmog) {
							do_collision = false;
						}
						if (do_collision && rect_collides(player_rect, tile_rect, &inter)) {
							game.player.pos = old_player_pos;
							game.player.is_moving = false;
						}

					}


					if (t == 'g' && game.player.is_moving && rect_collides(player_rect, tile_rect, NULL)) {
						i32 random = gb_random_range_int(0, 100);
						game.player.in_grass = true;
						if ((random < 7) && (game.grass_event_cooldown <= 0)) {
							game.grass_event_cooldown = 50;
#if 1
							change_to_battle_mode(BATTLE_TYPE_GRASS);
#endif
							return;
						}
					}

					if (t == 'D') {
						f32 dx = game.player.pos.x - (x + TILE_SIZE/2);
						f32 dy = game.player.pos.y - (y + TILE_SIZE/2);
						if (dx*dx+dy*dy < square(2*TILE_SIZE)) render_doctor_text = true;
					}

					if (t == 'Y') {
						set_tooltip(x, y, 1.5f,
						            "Hello! I'm your worst nightmare!",
						            "My name is Yugi the Boss.",
						            "Want to battle [z]?");

						if (key_pressed(KEY_Z)) {
							change_to_battle_mode(BATTLE_TYPE_BOSS);
							return;
						}
					}

					if (t == 'Q' && !game.has_chosen_transmog) {
						set_tooltip(x, y, 1.5f,
						            "It's dangerous to go alone",
						            "in there! You might get hurt.",
						            "Go see The Doctor in The Lab.");
					}

					if (t == 'L' || t == 'l') {
						set_tooltip(x, y, 1.0f,
						            "It appears to be weird chemicals.", NULL, NULL);
					}

					if (t == 'm' || t == 'c') {
						set_tooltip(x, y, 1.0f,
						            "State of the art computers",
						            "The new Pear Bartlett!",
						            NULL);
					}
					if (t == 'P') {
						set_tooltip(x, y, 0.6f,
						            "Such a lovely picture!", NULL, NULL);
					}

					if (t == 'B' || t == 'b') {
						set_tooltip(x, y, 1.2f,
						            "It is not time to sleep.", NULL, NULL);
					}

					if (t == 'W') {
						set_tooltip(x, y, 0.6f,
						            "It's a beautiful view outside.",
						            "Why not go outside and explore?", NULL);
					}
				}
			}
		}

		move_game_camera(map);


		// Render 1x1 Sprites
		for (j = 0; j < map->height; j++) {
			y = j*TILE_SIZE;
			for (i = 0; i < map->width; i++) {
				char t = map->tiles[j][i];
				x = i*TILE_SIZE;
				switch (t) {
				case ' ':
				case '.':
				case 'g':
				case 'Q':
				case 'D':
				case 'Y': {
					i32 sx = 0;
					i32 sy = map->outside;
					if (map->outside) {
						sx = (i+j)%2;
					}
					draw_sprite(x, y, sx, sy, 0);

					if (t == 'g') draw_sprite(x, y, 2, 1, 0); // grass
					if (t == 'Q' && !game.has_chosen_transmog)
						draw_sprite(x, y, 0, 3, 0); // Person in the way
					if (t == 'D') draw_sprite(x, y, 1, 3, 0); // Dr.
					if (t == 'Y') draw_sprite(x, y, 2, 3, 0); // Yugi
				} break;
				case 'x': draw_sprite(x, y,  1, 0, 0); break; // Wall
				case 'P': draw_sprite(x, y,  2, 0, 0); break; // Hanging Picture
				case 'W': draw_sprite(x, y,  3, 0, 0); break; // Window
				case 'T': draw_sprite(x, y,  3, 1, 0); break; // Thick Grass
				case 't': draw_sprite(x, y,  4, 1, 0); break; // Tree trunk
				case 'B': draw_sprite(x, y,  5, 0, 0); break; // Bed Top
				case 'b': draw_sprite(x, y,  5, 1, 0); break; // Bed bottom
				case 'M': draw_sprite(x, y,  6, 0, 0); break; // Computer monitor
				case 'm': draw_sprite(x, y,  6, 1, 0); break; // Computer keyboard
				case 'C': draw_sprite(x, y,  7, 0, 0); break; // Computer case
				case 'c': draw_sprite(x, y,  7, 1, 0); break; // Computer mouse
				case 'L': draw_sprite(x, y,  8, 0, 0); break; // Lap Chemicals top
				case 'l': draw_sprite(x, y,  8, 1, 0); break; // Lap Chemicals bottom

				default: {
					if (gb_char_is_digit(t)) draw_sprite(x, y, 4, 0, 0);
				} break;
				}
			}
		}

		// Render NxM Sprites
		// TODO(bill): Be more efficient
		for (j = 0; j < map->height; j++) {
			y = j*TILE_SIZE;
			for (i = 0; i < map->width; i++) {
				char t = map->tiles[j][i];
				x = i*TILE_SIZE;
				switch (t) {
				case 'h': draw_sprite_size(x, y, 12, 2, 4, 4, 0); break;
				case 'd': draw_sprite_size(x, y,  8, 2, 4, 4, 0); break;
				}
			}
		}

		{ // Render player and particles
			i32 sx = 0;
			u32 flags = 0;
			// 0 - down
			// 1 - up
			// 2 - left
			// 3 - right (-ve left)
			if (game.player.orientation == 0) sx = 0;
			if (game.player.orientation == 1) sx = 1;
			if (game.player.orientation == 2) sx = 2;
			if (game.player.orientation == 3) { sx = 2; flags |= DrawFlags_FlipH; }
			if (game.player.is_moving) {
				u32 flip = 10.0f*CORE->perf_frame.stamp;
				b32 use_flip = true;
				if (game.player.orientation == 0) {
					if (use_flip) {
						sx = 3;
						flags |= DrawFlags_FlipH * (flip%2);
					}
				} else if (game.player.orientation == 1) {
					if (use_flip) {
						sx = 4;
						flags |= DrawFlags_FlipH * (flip%2);
					}
				} else {
					sx += 3 * (flip%2);
				}
			}
			draw_sprite(game.player.pos.x, game.player.pos.y, sx, 2, flags);
		}

		{ // Render Text
			CORE->translate_x = 0;
			CORE->translate_y = 0;

			if (game.show_map_name_ticks-- > 0) {
				if (game.show_map_name_ticks < 0)
					game.show_map_name_ticks = 0;

				{
					char *name = game.maps[game.curr_map_index].name;
					i32 name_len = gb_strlen(name);
					i32 w = name_len*font.char_width+6;
					i32 h = font.char_height+6;

					rect_draw(rect_make_size(1, 1, w, h), PC_WHITE);
					rect_draw(rect_make_size(1,   0, w, 1), PC_DARK_GREY);
					rect_draw(rect_make_size(1, h+1, w, 1), PC_DARK_GREY);
					rect_draw(rect_make_size(0,   1, 1, h), PC_DARK_GREY);
					rect_draw(rect_make_size(w+1, 1, 1, h), PC_DARK_GREY);


					text_draw(4, 4, name, PC_BLACK);
				}
			}

			if (render_doctor_text) {
				if (game.doctor_line_index < gb_count_of(doctor_lines)) {
					draw_message_box(doctor_lines[game.doctor_line_index], "", "[Z] to continue");

					if (key_pressed(KEY_Z)) game.doctor_line_index++;
				} else if (!game.has_chosen_transmog) {
					draw_message_box("Line          [X]",
					                 "Triangle      [C]",
					                 "Square        [V]");
					if (key_pressed(KEY_X)) {
						game.player.my_transmog = make_transmog(TRANSMOG_TYPE_LINE);
						game.has_chosen_transmog = true;
					} else if (key_pressed(KEY_C)) {
						game.player.my_transmog = make_transmog(TRANSMOG_TYPE_TRIANGLE);
						game.has_chosen_transmog = true;
					} else if (key_pressed(KEY_V)) {
						game.player.my_transmog = make_transmog(TRANSMOG_TYPE_SQUARE);
						game.has_chosen_transmog = true;
					}

				} else {
					draw_message_box("Go now and find Transmogs!", "", "");
				}
			} else if (draw_tooltip) {
				draw_message_box(tooltip_messages[0], tooltip_messages[1], tooltip_messages[2]);
			}
		}
	} break;

	case GAME_MODE_BATTLE: {
		i32 i, w = CANVAS_WIDTH, h = CANVAS_HEIGHT;
		Transmog *enemy_mog = &game.battle_mode.enemy_mog;
		Transmog *player_mog = game.battle_mode.player_mog;
		char name_buf[32] = "";
		char *attack_names[3] = {
			"TRANSLATE",
			"ROTATE",
			"DIVIDE",
		};
		i32 level_diff = enemy_mog->shift_level - player_mog->shift_level;


		CORE->translate_x = 0;
		CORE->translate_y = 0;

		canvas_clear(PC_WHITE);

		// Enemy Transmog
		gb_snprintf(name_buf, gb_size_of(name_buf),
		            "%s LVL%d", transmog_name(enemy_mog->type), enemy_mog->shift_level);
		text_draw(8, 5, name_buf, PC_BLACK);
		draw_sprite_size(8, 12,
		                 2, 4,
		                 3, 1, 0);
		draw_sprite_size(24, 15,
		                 3, 5,
		                 2, 1, 0);
		{
			f32 hp_percent = cast(f32)enemy_mog->health/cast(f32)enemy_mog->max_health;
			rect_draw(rect_make_size(25, 16, hp_percent*30, 1), PC_LIGHT_GREY);
			draw_big_transmog(84, 4, enemy_mog->type);
		}


		// Player Transmog
		gb_snprintf(name_buf, gb_size_of(name_buf),
		            "%s LVL%d", transmog_name(player_mog->type), player_mog->shift_level);
		text_draw(100, 80, name_buf, PC_BLACK);
		draw_sprite_size(104, 92,
		                 5, 4,
		                 3, 1, 0);
		draw_sprite_size(117, 95,
		                 3, 5,
		                 2, 1, 0);
		{
			f32 hp_percent = cast(f32)player_mog->health/cast(f32)player_mog->max_health;
			char buf[16] = {0};
			gb_snprintf(buf, gb_size_of(buf),
			            "%03d/%03d", player_mog->health, player_mog->max_health);
			rect_draw(rect_make_size(118, 96, hp_percent*30, 1), PC_LIGHT_GREY);
			text_draw(118, 99, buf, PC_BLACK);

			draw_big_transmog(4, 54, player_mog->type);
		}

		if (!game.battle_mode.is_enemies_turn) {
			switch (game.battle_mode.state) {
			case BATTLE_TURN_STATE_INTRO: {
				char buf[32] = {0};
				gb_snprintf(buf, gb_size_of(buf),
				            "A wild %s appears!", transmog_name(enemy_mog->type));
				draw_message_box(buf, "", "[Z] to continue");

				if (key_pressed(KEY_Z)) {
					game.battle_mode.state = BATTLE_TURN_STATE_WHAT_TO_DO;
					return;
				}
			} break;
			case BATTLE_TURN_STATE_WHAT_TO_DO: {
				u32 hot_item = 0;
				if (game.battle_mode.type == BATTLE_TYPE_GRASS) {
					game.battle_mode.state_index %= 3;
					hot_item = game.battle_mode.state_index;
	 				draw_message_box("What to do?           Fight     ",
					                 "                      Substitute",
					                 "                      Escape    ");
					draw_sprite(70, 111, 0, 4, 0);
					draw_sprite(70, 127, 0, 5, 0);
					draw_sprite(90, 117 + hot_item*font.char_height, 5, 5, 0);

					if (key_pressed(KEY_DOWN)) {
						game.battle_mode.state_index++;
					}
					if (key_pressed(KEY_UP)) {
						if (game.battle_mode.state_index == 0)
							game.battle_mode.state_index = 2;
						else
							game.battle_mode.state_index--;
					}

					if (key_pressed(KEY_Z)) {
						switch (hot_item) {
						case 0:
							game.battle_mode.state = BATTLE_TURN_STATE_FIGHT;
							game.battle_mode.state_index = 0;
							game.battle_mode.fighting_state = FIGHTING_STATE_NONE;
							return;
						case 1:
							game.battle_mode.state = BATTLE_TURN_STATE_SUBSTITUTE;
							game.battle_mode.state_index = 0;
						case 2:
							game.battle_mode.state = BATTLE_TURN_STATE_ESPACE;
							game.battle_mode.state_index = 0;
							return;
						}
					}
				} else {
					game.battle_mode.state_index %= 2;
					hot_item = game.battle_mode.state_index;
	 				draw_message_box("What to do?           Fight     ",
					                 "                      Escape    ",
					                 "                                ");
					draw_sprite(70, 111, 0, 4, 0);
					draw_sprite(70, 127, 0, 5, 0);
					draw_sprite(90, 117 + hot_item*font.char_height, 5, 5, 0);

					if (key_pressed(KEY_DOWN)) {
						game.battle_mode.state_index++;
					}
					if (key_pressed(KEY_UP)) {
						if (game.battle_mode.state_index == 0)
							game.battle_mode.state_index = 1;
						else
							game.battle_mode.state_index--;
					}

					if (key_pressed(KEY_Z)) {
						switch (hot_item) {
						case 0:
							game.battle_mode.state = BATTLE_TURN_STATE_FIGHT;
							game.battle_mode.state_index = 0;
							game.battle_mode.fighting_state = FIGHTING_STATE_NONE;
							return;
						case 1:
							game.battle_mode.state = BATTLE_TURN_STATE_ESPACE;
							game.battle_mode.state_index = 0;
							return;
						}
					}
				}


			} break;
			case BATTLE_TURN_STATE_FIGHT: {
				char buf[32] = "";
				switch (game.battle_mode.fighting_state) {
				case FIGHTING_STATE_NONE: {
					u32 hot_item = game.battle_mode.state_index;
					draw_message_box("Which attack?         Translate ",
					                 "                      Rotate    ",
					                 "                      Divide    ");
					draw_sprite(70, 111, 0, 4, 0);
					draw_sprite(70, 127, 0, 5, 0);
					draw_sprite(90, 117 + hot_item*font.char_height, 5, 5, 0);

					if (key_pressed(KEY_DOWN)) {
						game.battle_mode.state_index++;
						game.battle_mode.state_index %= 3;
					}
					if (key_pressed(KEY_UP)) {
						if (game.battle_mode.state_index == 0)
							game.battle_mode.state_index = 2;
						else
							game.battle_mode.state_index--;
					}

					if (key_pressed(KEY_X)) {
						game.battle_mode.state_index = 0;
						game.battle_mode.state = BATTLE_TURN_STATE_WHAT_TO_DO;
						return;
					}

					if (key_pressed(KEY_Z)) {
						game.battle_mode.attack_index = hot_item;
						game.battle_mode.state_index = 0;
						game.battle_mode.fighting_state = FIGHTING_STATE_USE;
						return;
					}
				} break;
				case FIGHTING_STATE_USE: {
					gb_snprintf(buf, gb_size_of(buf),
					            "Your %s used %s",
					            transmog_name(enemy_mog->type),
					            attack_names[game.battle_mode.attack_index]);
					draw_message_box(buf, NULL, NULL);

					if (key_pressed(KEY_Z)) {
					if (-level_diff < 0) { // Higher level enemy
						game.battle_mode.fighting_state = FIGHTING_STATE_SUPER_EFFECTIVE;
					} else {
						if (gb_random_range_int(0, -level_diff+1) == 0) {
							game.battle_mode.fighting_state = FIGHTING_STATE_SUPER_EFFECTIVE;
						} else {
							if (gb_random_range_int(0, -level_diff/2 + 1) == 0)
								game.battle_mode.fighting_state = FIGHTING_STATE_EFFECTIVE;
							else
								game.battle_mode.fighting_state = FIGHTING_STATE_NO_EFFECT;
						}
					}
					return;
				}
				} break;
				case FIGHTING_STATE_EFFECTIVE: {
					gb_snprintf(buf, gb_size_of(buf),
					            "%s was effective",
					            attack_names[game.battle_mode.attack_index]);
					draw_message_box(buf, NULL, NULL);

					if (key_pressed(KEY_Z)) {
						i32 damage = gb_clamp(0, player_mog->attack_power/3 + level_diff/2, 1000);
						enemy_mog->health -= damage;

						game.battle_mode.fighting_state = FIGHTING_STATE_NEXT_TURN;
					}
				} break;
				case FIGHTING_STATE_SUPER_EFFECTIVE: {
					gb_snprintf(buf, gb_size_of(buf),
					            "%s was super effective",
					            attack_names[game.battle_mode.attack_index]);
					draw_message_box(buf, NULL, NULL);

					if (key_pressed(KEY_Z)) {
						i32 damage = gb_clamp(0, player_mog->attack_power/1 + level_diff/2, 1000);
						enemy_mog->health -= damage;
						game.battle_mode.fighting_state = FIGHTING_STATE_NEXT_TURN;
						game.battle_mode.state_index = 0;
					}
				} break;
				case FIGHTING_STATE_NO_EFFECT: {
					gb_snprintf(buf, gb_size_of(buf),
					            "%s had no effect",
					            attack_names[game.battle_mode.attack_index]);
					draw_message_box(buf, NULL, NULL);

					if (key_pressed(KEY_Z))
						game.battle_mode.fighting_state = FIGHTING_STATE_NEXT_TURN;
				} break;
				case FIGHTING_STATE_NEXT_TURN: {
					char *end_message = "";
					if (enemy_mog->health <= 0 &&
					    !(game.battle_mode.state_index & 1)) {
						enemy_mog->health = 0;
						player_mog->xp += clamp(1, 4 + level_diff, 1000);

						while (player_mog->xp >= 10) {
							player_mog->xp -= 10;
							player_mog->shift_level++;
							update_transmog_stats(player_mog);
							player_mog->health = player_mog->max_health;
							game.battle_mode.state_index |= 2;
						}

						game.battle_mode.state_index |= 1;
					}
					if (game.battle_mode.state_index & 1) {
						if (game.battle_mode.type == BATTLE_TYPE_GRASS) {
							if (game.battle_mode.state_index & 2)
								end_message = "Your Transmog has levelled up!";
							draw_message_box("You win this battle!", end_message, "Exit the battle.");
							if (key_pressed(KEY_Z)) {
								game.mode = GAME_MODE_MAP;
								game.battle_mode.state_index = 0;
								return;
							}
						} else {
							canvas_clear(PC_WHITE);
							draw_sprite(72, 40,
							            2, 3, 0);

							draw_message_box("You defeated me?! HOW?!",
							                 "Well it's my first battle...",
							                 "Well done and thank you!");

							if (key_pressed(KEY_Z)) {
								game.mode = GAME_MODE_END_SUCCESS;
								return;
							}
						}
					} else {
						draw_message_box("End turn.", NULL, NULL);
						if (key_pressed(KEY_Z)) {
							end_battle_mode_turn();
							return;
						}
					}
				} break;
				}
			} break;
			case BATTLE_TURN_STATE_SUBSTITUTE: {
				i32 level_diff = enemy_mog->shift_level - player_mog->shift_level;

				if (level_diff <= 0) {
					draw_message_box("Failed to substitute",
					                 "",
					                 "End turn.");

					if (key_pressed(KEY_Z)) {
						end_battle_mode_turn();
						return;
					}

				} else {
					if (game.battle_mode.state_index == 0) {
						if (gb_random_range_int(0, level_diff) != 0)
							game.battle_mode.state_index = 1; // Success
						else
							game.battle_mode.state_index = 2; // Failure
					}

					if (game.battle_mode.state_index == 1) {
						draw_message_box("Well done!",
						                 "You successfully substituted.",
						                 "Exit to world");

						if (key_pressed(KEY_Z)) {
							// Change it!
							game.player.my_transmog = *enemy_mog;
							game.mode = GAME_MODE_MAP;
							return;
						}
					} else {
						draw_message_box("Substitution failed!",
						                 "",
						                 "End turn.");

						if (key_pressed(KEY_Z)) {
							end_battle_mode_turn();
							return;
						}
					}
				}
			} break;
			case BATTLE_TURN_STATE_ESPACE: {
				if (game.battle_mode.state_index == 0) {
					if (gb_random_range_int(0, 100) < 90) {
						game.battle_mode.state_index = 1;
					} else {
						game.battle_mode.state_index = 2;
					}
				}

				if (game.battle_mode.state_index == 1) {
					draw_message_box("You escaped from this battle!",
					                 "",
					                 "[Z] to return to the world.");

					if (key_pressed(KEY_Z)) {
						game.mode = GAME_MODE_MAP;
						return;
					}
				} else {
					draw_message_box("You could not escape!",
					                 "You'll have to try again!",
					                 "[Z] to end turn.");
					end_battle_mode_turn();
					return;
				}
			} break;
			}
		} else {
		// NOTE(bill): Enemies Turn
			char buf[32] = {0};

			switch (game.battle_mode.fighting_state) {
			case FIGHTING_STATE_NONE: {
				game.battle_mode.attack_index = gb_random_range_int(0, 2);
				game.battle_mode.fighting_state = FIGHTING_STATE_USE;
				return;
			} break;
			case FIGHTING_STATE_USE: {
				gb_snprintf(buf, gb_size_of(buf),
				            "Enemy %s used %s",
				            transmog_name(enemy_mog->type),
				            attack_names[game.battle_mode.attack_index]);
				draw_message_box(buf, NULL, NULL);

				if (key_pressed(KEY_Z)) {
					if (level_diff < 0) { // Higher level enemy
						game.battle_mode.fighting_state = FIGHTING_STATE_SUPER_EFFECTIVE;
					} else {
						if (gb_random_range_int(0, -level_diff+3) == 0) {
							game.battle_mode.fighting_state = FIGHTING_STATE_SUPER_EFFECTIVE;
						} else {
							if (gb_random_range_int(0, -level_diff/2 + 3) == 0)
								game.battle_mode.fighting_state = FIGHTING_STATE_EFFECTIVE;
							else
								game.battle_mode.fighting_state = FIGHTING_STATE_NO_EFFECT;
						}
					}
					return;
				}
			} break;
			case FIGHTING_STATE_EFFECTIVE: {
				gb_snprintf(buf, gb_size_of(buf),
				            "%s was effective",
				            attack_names[game.battle_mode.attack_index]);
				draw_message_box(buf, NULL, NULL);

				if (key_pressed(KEY_Z)) {
					i32 damage = gb_clamp(0, enemy_mog->attack_power/6.0f - level_diff/3.0f, 1000);

					player_mog->health -= damage;

					game.battle_mode.fighting_state = FIGHTING_STATE_NEXT_TURN;
				}

			} break;
			case FIGHTING_STATE_SUPER_EFFECTIVE: {
				gb_snprintf(buf, gb_size_of(buf),
				            "%s was super effective",
				            attack_names[game.battle_mode.attack_index]);
				draw_message_box(buf, NULL, NULL);

				if (key_pressed(KEY_Z)) {
					i32 damage = gb_clamp(0, enemy_mog->attack_power/3.0f - level_diff/2.0f, 1000);

					player_mog->health -= damage;

					game.battle_mode.fighting_state = FIGHTING_STATE_NEXT_TURN;
				}
			} break;
			case FIGHTING_STATE_NO_EFFECT: {
				gb_snprintf(buf, gb_size_of(buf),
				            "%s had no effect",
				            attack_names[game.battle_mode.attack_index]);
				draw_message_box(buf, NULL, NULL);

				if (key_pressed(KEY_Z))
					game.battle_mode.fighting_state = FIGHTING_STATE_NEXT_TURN;
			} break;
			case FIGHTING_STATE_NEXT_TURN: {
				char *end_message = "";
				if (player_mog->health <= 0 &&
				    !(game.battle_mode.state_index & 1)) {
					player_mog->health = 0;

					game.battle_mode.state_index |= 1;
				}
				if (game.battle_mode.state_index & 1) {
					draw_message_box("You lost this battle!", "And the game :(", "Exit the battle.");
					if (key_pressed(KEY_Z)) {
						game.mode = GAME_MODE_END_FAIL;
						return;
					}
				} else {
					draw_message_box("The enemy's turn has ended.", NULL, NULL);
					if (key_pressed(KEY_Z)) {
						end_battle_mode_turn();
						return;
					}
				}
			} break;
			}

		}
	} break;

	case GAME_MODE_END_FAIL: {
		canvas_clear(PC_BLACK);
		draw_message_box("You have failed :(",
		                 "Would you like to try again?",
		                 "Try again [z] Exit [esc]");
		if (key_pressed(KEY_Z)) {
			init_game();
			return;
		}
	} break;

	case GAME_MODE_END_SUCCESS: {
		canvas_clear(PC_BLACK);

		text_draw(4, 4,
		          "Thank you for playing Transmog!\n"
		          "Ginger Bill (c) 2016\n"
		          "\n"
		          "Ludum Dare 35\n"
		          "Theme: Shapeshift\n"
		          "\n"
		          "Please remember for vote!\n",
		          PC_WHITE);

		draw_message_box("Exit [esc]", NULL, NULL);
	} break;
	}
}
