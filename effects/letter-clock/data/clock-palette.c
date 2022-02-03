static const PaletteT clock_pal = {
    .count = 32,
    .colors = {
        0x000, // 00 Background [0x000]
        0xfff, // 01 White [0xfff]
        0x000, // 02 Red (Kop coppera) [0xf00]
        0x000, // 03 Magenta (Passe-partout) [0xf0f]
        0x000, // 04 Teal (Intersection) [0x088]
        0xff0, // 05 Yellow [0xff0]
        0x000, // 06 Green (Hehe) [0x0f0]
        0x808, // 07 Purple [0x808]
        0x000, // 08 4th lens + background = background
        0xfff, // 09 4th lens + white = white
        0xf0f, // 10 4th lens + Red (Kop coppera) = Magenta (Passe-partout)
        0x000, // 11 4th lens + Magenta (Passe-partout) = background
        0xf0f, // 12 4th lens + Teal (Intersection) = Magenta (background)
        0xb21, // 13 4th lens + Yellow = Custom colour
        0x0f0, // 14 4th lens + Green (Hehe) = Hehe
        0xf31, // 15 4th lens + Purple = Custom colour
        0x000, // 16 5th lens + background = background
        0xfff, // 17 5th lens + white = white
        0x000, // 18 5th lens + Red (Kop coppera) = background
        0xf0f, // 19 5th lens + Magenta (Passe-partout) = Magenta (Passe-partout)
        0xf0f, // 20 5th lens + Teal (Intersection) =  Magenta (Passe-partout)
        0xf18, // 21 5th lens + Yellow = Custom colour
        0x0f0, // 22 5th lens + Green (Hehe) = Hehe
        0x16f, // 23 5th lens + Purple = Custom colour
        0x000, // 24 both lenses + background = background
        0xfff, // 25 both lenses + white = white
        0x0ef, // 26 both lenses + Red (Kop coppera) = Custom colour
        0x06f, // 27 both lenses + Magenta (Passe-partout) = Custom colour
        0xf20, // 28 both lenses + Teal (Intersection) = Custom colour
        0xfa1, // 29 both lenses + Yellow = Custom colour
        0x0f7, // 30 both lenses + Green (Hehe) = Custom colour
        0xef1, // 31 both lenses + Purple = Custom colour
               // 4th lens is responsible for Kop coppera
               // 5th lens is responsible for Passe-partout
               // both display Hehe
               // Kop coppera and Passe-partout share the same colour (Magenta)
    }};