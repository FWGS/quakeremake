$modelname "tarbaby.mdl" 
$cd ".\" 
$cdtexture ".\" 
$scale 1.0 
$origin 0 0 3 -90 
$bodygroup body 
{ 
studio "reference" 
} 

$hbox 0 "Bone0" -29.76 -35.79 -37.74  32.99 28.45 43.99
 
$sequence "idle" "idle" fps 10 loop ACT_IDLE 1
$sequence "walk" "walk" LX fps 10 loop ACT_WALK 1
$sequence "run" "run" LX fps 10 loop ACT_RUN 1
$sequence "fly" "fly" fps 10 loop ACT_FLY 1
$sequence "jump" "jump" fps 10 ACT_LEAP 1
$sequence "exp" "exp" fps 10 ACT_DIESIMPLE 1
 
