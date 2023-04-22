-====== hmap2 ======-
Authors: LordHavoc and Vic
Emails: lordhavoc@ghdigital.com and vic@quakesrc.org
Websites: http://icculus.org/twilight/darkplaces and http://hkitchen.quakesrc.org

What is it:
hmap2 is a quake map compiler suite, a combination of qbsp+vis+light+bsp2prt+bspinfo all in one, written by LordHavoc and Vic, it has many new features.

How to use:
Basically just this:
hmap2 mymap
hmap2 -vis mymap
hmap2 -light -extra4x4 mymap
Would compile mymap.map to mymap.bsp and mymap.lit, the other files (.prt, .pts) can be discarded for map releases (.pts is debugging info for finding leaks, go in quake and type pointfile in the console to use it, .prt is for vis compilation).

Re-vising a map:
hmap2 -bsp2prt e1m1
hmap2 -vis e1m1
Would recompile the vis data in e1m1, note that bsp2prt automatically makes water transparent, so this is a way to watervis your maps without using vispatch.
(Note that quake prefers paks over regular files, so if you want the new e1m1.bsp to be loaded this would have to be done in a mod directory for quake to actually load it, or assorted other solutions)

Relighting a map:
hmap2 -light -extra8x8 e1m1
Would make a normal looking version of e1m1 with higher quality shadows.
hmap2 -light -extra8x8 -intensity 0.5 -radiusscale 2 e1m1
Would make a version of e1m1 with much softer lighting, and higher quality shadows.

Note: hmap2 lighting options do not affect darkplaces rtlights. 

See the usage information later in this readme for information on each utility.
(Also available by running the utilities without specifying a map)

Features:
.lit file support - colored lighting for supporting engines, without breaking quake compatibility
Mostly tyrlite compatible - the "delay" key for light types is supported
6 light types to suit many purposes - including sun lights casting directional light from sky polygons, large soft 1/(x*x) lights, and others
Light tweaking options on commandline - -radiusscale, -intensity, -defaulttype, -overridetypes
-minlight support - raises dark areas to this level without affecting brighter areas
-ambientlight support - raises all areas of the level by this amount
Re-vis maps - using bsp2prt followed by vis (Vic)
Make new .lit files for existing maps without modifying the .bsp - -relight and various options are useful for this
Compiling an unsealed 'leaky' map still produces a .prt file so it can be vised
Faster lighting - uses vis data to optimize lighting, be sure to run -vis before -light to get this speedup
Faster vis - uses rvis optimization which has no impact on quality and is around 30% faster
More lightmap antialiasing modes - in addition to -extra there are -extra4x4 and -extra8x8 (Vic added -extra8x8)
Able to make higher performance largescale maps with -darkplaces option (most other engines can't handle huge lightmaps like this produces, be warned)
Huge map support - up to +-32768 coordinates, make huge levels like you always wanted (warning: quake's networking limits playable area to +-4096 coordinates, except in darkplaces which fully supports huge maps)
Increased limits to the very max of the .bsp format
Hipnotic compatible rotating bmodel support - any entity classname beginning with rotate_ will check for a target to get the origin from, or you can set origin key on bmodel to make it rotate around that location
Support for WorldCraft 'Hammer' .map enhanced texture alignment
Support for HalfLife "light" key - "red green blue radius" format, example 255 255 255 300
Transparent water support (on by default)
Better error reporting - often says where a problem occurred in the level, and some errors are now only warnings (such as point off plane, which tries to self correct)
Multiple wad support - example: "wad" "wizard.wad;base.wad"
Can disable liquid sounds using -noambient, -noambientwater, -noambientsky, -noambientslime, -noambientlava
Can enable the unused slime sound channel using -ambientslime to make it use slime channel instead of water channel (if engine does not support this the slime is silent) (Vic)
Defaults to -level 4 vis instead of -level 2 like original vis did
Compresses vis data more by merging vis data for identical parts of a map, saving a few k (Vic)
Supports func_group entities which are merged into the world during compile, they are an editing helper (Vic)
Can use harsh light shading instead of traditional quake soft shading using -harshshade

Usage info on the hmap2 utilities:

usage: hmap2 [options] sourcefile [destfile]
Compiles .map to .bsp, does not compile vis or lighting data
other utilities available:
-bsp2prt    bsp2prt utility, run -bsp2prt as the first parameter for more
-bspinfo    bspinfo utility, run -bspinfo as the first parameter for more
-light      lighting utility, run -light as the first parameter for more
-vis        vis utility, run -vis as the first parameter for more
What the options do:
-nowater    disable watervis; r_wateralpha in glquake will not work right
-notjunc    disable tjunction fixing; glquake will have holes between polygons
-nofill     disable sealing of map and vis, used for ammoboxes
-onlyents   patchs entities in existing .bsp, for relighting
-verbose    show more messages
-darkplaces allow really big polygons
-noforcevis don't make a .prt if the map leaks


usage: hmap2 -bsp2prt sourcefile
Makes a .prt file from a .bsp, to allow it to be vised
What the options do:
-nowater    disable watervis; r_wateralpha in glquake will not work right


usage: hmap2 -bspinfo sourcefile
Prints information about a .bsp file


usage: hmap2 -vis [options] bspfile
Compiles visibility data in a .bsp, needs a .prt file
What the options do:
-level 0-4      quality, default 4
-fast           fast but bad quality vis
-v              verbose
-norvis         disable rvis optimization, 0.001% better quality and 30% slower
-ambientslime   do not convert slime channel to water (requires engine support)
-noambient      disable ambient sounds (water bubbling, wind, etc)
-noambientwater disable ambient water sounds (water)
-noambientslime disable ambient slime sounds (water, or -ambientslime)
-noambientlava  disable ambient lava sounds (unused by quake)
-noambientsky   disable ambient sky sounds (wind)
-noreuse        disable merging of identical vis data (less compression)
-farplane       limit visible distance (warning: not a good idea without fog)


usage: hmap2 -light [options] bspfile
Compiles lighting data in a .bsp and also makes .lit colored lighting data
Quick usage notes for entities: (place these in key/value pairs)
wait - falloff rate: 1.0 default, 0.5 = double radius, 2 = half radius
_color - red green blue, specifies color of light, example 1 0.6 0.3 is orange
_lightradius - forces light to be this radius (useful with 1/ types)
delay - light type: (x = distance of surface from light, r = radius)
0: 1-(x/r)    fast, quake lighting, the default, tyrlite compatible
1: 1/(x)      slow, tyrlite compatible
2: 1/(x*x)    slow, realistic, tyrlite compatible
3: 1          fast, no fade, useful for sky lights, tyrlite compatible
4: sun        slow, directional sunlight, uses target direction like spotlights
5: 1-x/r*x/r  fast, looks like darkplaces/tenebrae lights
What the options do:
-extra        antialiased lighting, takes much longer, higher quality
-extra4x4     antialiased lighting, even slower and better than -extra
-extra8x8     antialiased lighting, even slower and better than -extra4x4
-nolightvis   disables use of visibility data to optimize lighting
-relight      make a .lit file for an existing .bsp without modifying the .bsp
-harshshade   harsh shading rather than the normal soft shading
Options from here on are incompatible with darkplaces realtime lighting mode
(it does not know if these options are used, and will require manual rtlights
 editing to look good)
-intensity    scales brightness of all lights
-radiusscale  scales size of all lights (including 1/ types)
-defaulttype <number> sets default light type by number, see delay above
-overridetypes forces all lights to use the -defaulttype setting
-minlight     raises darkest areas of the map to this light level (0-255)
-ambientlight raises all of the map by this light level (0-255)

Vic's changes:
general:
hmap2 is hqbsp + hlight + hvis + bsp2prt + bspinfo combined
fixed memory leaks
general code cleanup
rewrote many parts of the code to run faster and be easier to read
bsp2prt allows maps to be re-vised
bspinfo prints info about compiled maps
qbsp:
no hull files
support for maps where "worldspawn" is not the first entity
support for "func_group" entities
reduced memory usage
hqbsp now properly calculates node bounding boxes making them much smaller for non-axial 
planes (this means maps run faster in engines other than darkplaces; darkplaces already did this on loading)
detects and skips degenerate edges
corrected CheckWindingArea for portals area checking
precise .prt files output (may fix some vis errors on complex maps)
misc bugfixes
light:
added -extra8x8 lightmap antialiasing option
vis:
better vis compression (-noreuse to disable)
fixed ambient sounds calculations (they now fade with distance)
-ambientslime enables the unused slime sound channel (not supported by most (all?) quake engines, normally slime uses the water channel)
added -farplane option to limit visible distance

Programmer stuff:
The sourcecode zip includes a Makefile for GNU make, a Makefile.mingw for mingw (make -f Makefile.mingw), and Microsoft Visual C++ 6.0 project files (open the hmap.dsw workspace).

Thanks to:
id Software for the addictive masterpiece known as Quake, and releasing source

