gb_global char *map0_name    = "Your Home";
gb_global i32   map0_width   = 9;
gb_global i32   map0_height  = 6;
gb_global b32   map0_outside = false;
gb_global char *map0[] = {
	"WxxxPxxMC",
	"B      mc",
	"b        ",
	"         ",
	"    .    ",
	"    1    ",
};


gb_global char *map1_name    = "Geometricity";
gb_global i32   map1_width   = 32;
gb_global i32   map1_height  = 33;
gb_global b32   map1_outside = true;
gb_global char *map1[] = {
	" TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT",
	" TTTTTTTTTTTThxxxTTdxxxTTTTTTTTT",
	" TTTTTTTTTTTTxxxxTTxxxxTTTTTTTTT",
	" TTTTTTTTTTTTxxxxTTxxxxTTTTTTTTT",
	" TTTTTTTTTtttxxxxttxxxxtttTTTTTT",
	" TTtttttttt   0     2    tTTTTTT",
	" TTt      t   .     .    tTTTTTT",
	" TTt      Q              tTTTTTT",
	" TTt   tttttttttttttttttttTTTTTT",
	" TTt   tTTTTTTTTTTTTTTTTTTTTTTTT",
	" TTt   tTTTTTTTTTTTTTTTTTTTTTTTT",
	" TTtgggtTTTTTTTTTTTTTTTTTTTTTTTT",
	" TTtgggtTTTTTTTTTTTTTTTTTTTTTTTT",
	" TTtgggtTTTTTTTTTTTTTTTTTTTTTTTT",
	" TTtgggtTTTTTTTTTTTTTTTTTTT     ",
	" TTtgggtttttttttttttttTTTTT     ",
	" TTtgggggggggggggggggtTTTTT     ",
	" TTtgggggggggggggggggtTTTTT     ",
	" TTtgggggggggggggggggtTTTTT     ",
	" TTtttttttttttttttgggtTTTTT     ",
	" TTTTTTTTTTTTTTTTt ggtTTTTT     ",
	" TTTTTTTTTTTTTTTTt ggtTTTTT     ",
	" TTTTTTTTTTTTTTTTt ggtTTTTT     ",
	" TTTTTTTTTTTTTTTTtgggtTTTTT     ",
	" TTTTTTTTTTTTTTTTtgggtTTTTT     ",
	" TTTTTTTTTTTTTTTTtgg tTTTTT     ",
	" TTTTTTTTTTTTTTTTtgg tTTTTT     ",
	"ttttttttttttttttttgggtTTTTT     ",
	"tggg              gggtTTTTT     ",
	"t                  ggtTTTTT     ",
	"t gggggggggggggggggggtTTTTT     ",
	"t.tttttttttttttttttttt TTTT     ",
	"t3tttttttttttttttttttt  TTT     ",
};


gb_global char *map2_name    = "The Lab";
gb_global i32   map2_width   = 9;
gb_global i32   map2_height  = 14;
gb_global b32   map2_outside = false;
gb_global char *map2[] = {
	"xPxWWMCMC",
	"     mcmc",
	"LL       ",
	"ll  D    ",
	"         ",
	"         ",
	"         ",
	"       LL",
	"       ll",
	"         ",
	"LL       ",
	"ll       ",
	"    .    ",
	"    1    ",
};


gb_global char *map3_name    = "The Wild";
gb_global i32   map3_width   = 16;
gb_global i32   map3_height  = 15;
gb_global b32   map3_outside = true;
gb_global char *map3[] = {
	"Ttt6tTtttttttt1t",
	"Ttt.tTt       .t",
	"Tt  tTt  ttttttt",
	"Tt  tTt  tTTTTTT",
	"Tt  ttt  tTTTTTT",
	"TtgggggggtTTTTTT",
	"TtggtttggtTTTTTT",
	"TtggtTtggtttttTT",
	"TtggtTtggggggtTT",
	"TtggtTtggttggtTT",
	"TtggtTtggttggtTT",
	"Tt  tTtggttggtTT",
	"Tt  tTt  ttggtTT",
	"Tt. tTt  tt .tTT",
	"Tt4ttTtttttt5tTT",
};



gb_global char *map4_name    = "Dead-end Wild";
gb_global i32   map4_width   = 5;
gb_global i32   map4_height  = 8;
gb_global b32   map4_outside = true;
gb_global char *map4[] = {
	"Tt3tT",
	"Tt.tT",
	"Tt tT",
	"Tt tT",
	"Tt tT",
	"Tt tT",
	"Tt tT",
	"TTTTT",
};

// NOTE(bill): It's an shit easter egg
gb_global char *map5_name    = "...";
gb_global i32   map5_width   = 13;
gb_global i32   map5_height  = 8;
gb_global b32   map5_outside = true;
gb_global char *map5[] = {
	"tt  t3t   t  ",
	"t t t.t   t  ",
	"tt  t t   t  ",
	"t t t t   t  ",
	"tt  t ttt ttt",
	"             ",
	"             ",
	"             ",
};



gb_global char *map6_name    = "What the?!";
gb_global i32   map6_width   = 19;
gb_global i32   map6_height  = 11;
gb_global b32   map6_outside = true;
gb_global char *map6[] = {
	"ttttttttttttttttttt",
	"tgg             ggt",
	"tgtttttttttttttttgt",
	"t        g    ggt t",
	"tttttttttttttttgt t",
	"tgg        g    t t",
	"tgttttttttttttttt t",
	"t      g      ggt t",
	"tttttttttttttttgt t",
	"t.   g          t.t",
	"t7ttttttttttttttt3t",
};


gb_global char *map7_name    = "The Boss Room";
gb_global i32   map7_width   = 19;
gb_global i32   map7_height  = 11;
gb_global b32   map7_outside = true;
gb_global char *map7[] = {
	"ttttttttt6ttttttttt",
	"tTT      .      TTt",
	"tT               Tt",
	"t                 t",
	"t                 t",
	"t        Y        t",
	"t                 t",
	"t                 t",
	"tT               Tt",
	"tTT             TTt",
	"ttttttttttttttttttt",
};

